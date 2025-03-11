#include "RenderingSystem.h"
#include "LightSystem.h"
#include <d3d11.h>
#include <string>
#include <algorithm>

#include "CameraSystem.h"
#include "debug/DebugSystem.h"
#include "Engine/DispositifD3D11.h"
#include "Engine/MeshManager.h"
#include "Engine/Texture.h"
#include "Engine/TextureManager.h"
#include "Engine/ECS/components/transform/TransformComponent.h"
#include "Engine/ECS/components/rendering/PBRRenderer.h"
#include "Engine/Shaders/techniques/GlobaleTechnique.h"
#include "Engine/Shaders/techniques/ShaderTechniqueFactory.h"
#include "Engine/Textures/FallbackTextures.h"
#include "Engine/Utils/ErrorLogger.h"
#include "Engine/ECS/core/World.h"
#include "Engine/ECS/components/mesh/MeshComponent.h"
#include "Engine/ECS/components/rendering/SpriteRenderer.h"
#include "Engine/ECS/components/transform/TransformComponent.h"
#include "Engine/ECS/components/rendering/TextRendererComponent.h"
#include "Engine/ECS/components/rendering/UIRendererComponent.h"
#include "Engine/ImGui/imgui_impl_dx11.h"
#include "Engine/ImGui/imgui_impl_win32.h"
#include "Engine/Shaders/ShaderVariant.h"

using namespace FrostFireEngine;
using namespace DirectX;

static const UINT MAX_DIRECTIONAL_LIGHTS = 4;

RenderingSystem::RenderingSystem(DispositifD3D11 *device)
  : m_device(device), m_shaderManager(&ShaderManager::GetInstance())
{
  D3DResources d3dResources;
  d3dResources.setContext(device->GetImmediateContext());
  d3dResources.setDevice(device->GetD3DDevice());
  m_d3dResources = std::make_unique<D3DResources>(d3dResources);

  m_backBufferRTV = device->GetRenderTargetView();
}

void RenderingSystem::Initialize()
{
  m_shaderManager->Initialize(m_device->GetD3DDevice());

  UINT width = static_cast<UINT>(m_device->GetViewportWidth());
  UINT height = static_cast<UINT>(m_device->GetViewportHeight());
  if (!m_gbuffer.Initialize(m_device->GetD3DDevice(), width, height)) {
    ErrorLogger::Log("Failed to initialize GBuffer");
    return;
  }

  if (!CreateLightingTarget()) {
    ErrorLogger::Log("Failed to create lighting target");
    return;
  }

  if (!CreateShadowMapArray(MAX_DIRECTIONAL_LIGHTS)) {
    ErrorLogger::Log("Failed to create shadow map array");
    return;
  }

  // Screen size buffer
  {
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(float) * 4;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    if (HRESULT hr = m_device->GetD3DDevice()->CreateBuffer(&bd, nullptr, &m_screenSizeBuffer);
      FAILED(hr)) {
      ErrorLogger::Log("Failed to create screen size buffer.");
    }
  }

  m_globalTechnique = std::make_unique<GlobaleTechnique>();

  InitializeWhiteFallbackTexture(m_device);
  InitializeWhiteCubeMapFallbackTexture(m_device);
  InitializeWhiteFallbackTexture(m_device);
  InitializeWhiteCubeMapFallbackTexture(m_device);
  InitializeNeutralNormalFallbackTexture(m_device);

  // Linear Sampler
  {
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    HRESULT hr = m_device->GetD3DDevice()->CreateSamplerState(&samplerDesc, &m_linearSampler);
    if (FAILED(hr)) {
      ErrorLogger::Log("Failed to create linear sampler state.");
    }
  }

  // Shadow sampler (Comparison)
  {
    D3D11_SAMPLER_DESC cmpDesc = {};
    cmpDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    cmpDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    cmpDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    cmpDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    cmpDesc.BorderColor[0] = 1.0f;
    cmpDesc.BorderColor[1] = 1.0f;
    cmpDesc.BorderColor[2] = 1.0f;
    cmpDesc.BorderColor[3] = 1.0f;
    cmpDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    cmpDesc.MinLOD = 0;
    cmpDesc.MaxLOD = D3D11_FLOAT32_MAX;

    HRESULT hr = m_device->GetD3DDevice()->CreateSamplerState(&cmpDesc, &m_shadowSampler);
    if (FAILED(hr)) {
      ErrorLogger::Log("Failed to create shadow comparison sampler state.");
    }
  }

  // Skybox
  {
    m_skyboxMesh = MeshManager::GetInstance().GetCubeMapMesh(m_device->GetD3DDevice(), 100.0f);

    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.ByteWidth = sizeof(XMFLOAT4X4);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT hr = m_device->GetD3DDevice()->CreateBuffer(&cbDesc, nullptr, &m_skyboxMatrixBuffer);
    if (FAILED(hr)) {
      ErrorLogger::Log("Failed to create skybox matrix buffer.");
    }

    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = TRUE;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    dsDesc.StencilEnable = FALSE;

    hr = m_device->GetD3DDevice()->CreateDepthStencilState(&dsDesc, &m_skyboxDepthStencilState);
    if (FAILED(hr)) {
      ErrorLogger::Log("Failed to create skybox depth stencil state.");
    }

    D3D11_RASTERIZER_DESC rastDesc = {};
    rastDesc.FillMode = D3D11_FILL_SOLID;
    rastDesc.CullMode = D3D11_CULL_BACK;
    rastDesc.DepthClipEnable = FALSE;

    hr = m_device->GetD3DDevice()->CreateRasterizerState(&rastDesc, &m_skyboxRasterizerState);
    if (FAILED(hr)) {
      ErrorLogger::Log("Failed to create skybox rasterizer state.");
    }

    std::vector<std::string> features;
    VertexLayoutDesc         skyboxLayout;
    for (const auto &i : Vertex::layout) {
      skyboxLayout.elements.push_back(i);
    }

    m_skyboxVariant = m_globalTechnique->GetVariantForPass(RenderPass::Skybox, features,
      skyboxLayout);
    if (!m_skyboxVariant) {
      ErrorLogger::Log("No skybox variant found in global technique.");
    }
  }

  // Transparency states
  if (!CreateTransparencyStates()) {
    ErrorLogger::Log("Failed to create transparency states.");
  }

  // Light matrix buffer array
  {
    D3D11_BUFFER_DESC cbd = {};
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.ByteWidth = sizeof(XMFLOAT4X4) * MAX_DIRECTIONAL_LIGHTS;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT hr = m_device->GetD3DDevice()->CreateBuffer(&cbd, nullptr, &m_lightMatrixBufferArray);
    if (FAILED(hr)) {
      ErrorLogger::Log("Failed to create light matrix buffer array.");
    }
  }

  // Post-process target
  {
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    texDesc.SampleDesc.Count = 1;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    texDesc.Usage = D3D11_USAGE_DEFAULT;

    ComPtr<ID3D11Texture2D> postProcessTex;
    if (FAILED(m_device->GetD3DDevice()->CreateTexture2D(&texDesc, nullptr, &postProcessTex))) {
      ErrorLogger::Log("Failed to create post-process texture.");
    }

    if (FAILED(
      m_device->GetD3DDevice()->CreateRenderTargetView(postProcessTex.Get(), nullptr, &
        m_postProcessRTV))) {
      ErrorLogger::Log("Failed to create post-process RTV.");
    }

    if (FAILED(
      m_device->GetD3DDevice()->CreateShaderResourceView(postProcessTex.Get(), nullptr, &
        m_postProcessSRV))) {
      ErrorLogger::Log("Failed to create post-process SRV.");
    }
  }

#ifdef _DEBUG
  {
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(DirectX::XMFLOAT4X4);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT hr = m_device->GetD3DDevice()->CreateBuffer(&bd, nullptr, &m_debugMatrixBuffer);
    if (FAILED(hr)) {
      ErrorLogger::Log("Failed to create debug matrix buffer.");
    }
  }
#endif
}

bool RenderingSystem::CreateShadowMapArray(UINT count)
{
  D3D11_TEXTURE2D_DESC texDesc = {};
  texDesc.Width = 4096;
  texDesc.Height = 4096;
  texDesc.MipLevels = 1;
  texDesc.ArraySize = count;
  texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
  texDesc.SampleDesc.Count = 1;
  texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
  texDesc.Usage = D3D11_USAGE_DEFAULT;

  auto device = m_device->GetD3DDevice();
  if (FAILED(device->CreateTexture2D(&texDesc, nullptr, &m_shadowMapArray))) {
    return false;
  }

  D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
  dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
  dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
  dsvDesc.Texture2DArray.ArraySize = count;
  dsvDesc.Texture2DArray.FirstArraySlice = 0;
  dsvDesc.Texture2DArray.MipSlice = 0;

  if (FAILED(device->CreateDepthStencilView(m_shadowMapArray.Get(), &dsvDesc, &m_shadowDSVArray))) {
    return false;
  }

  D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
  srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
  srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
  srvDesc.Texture2DArray.ArraySize = count;
  srvDesc.Texture2DArray.FirstArraySlice = 0;
  srvDesc.Texture2DArray.MipLevels = 1;

  if (FAILED(
    device->CreateShaderResourceView(m_shadowMapArray.Get(), &srvDesc, &m_shadowSRVArray))) {
    return false;
  }

  m_shadowMapArraySize = count;
  return true;
}

bool RenderingSystem::CreateTransparencyStates()
{
  D3D11_BLEND_DESC blendDesc = {};
  blendDesc.RenderTarget[0].BlendEnable = TRUE;
  blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
  blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
  blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

  blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
  blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
  blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
  blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

  HRESULT hr = m_device->GetD3DDevice()->CreateBlendState(&blendDesc, &m_transparencyBlendState);
  if (FAILED(hr)) {
    return false;
  }

  D3D11_DEPTH_STENCIL_DESC depthDesc = {};
  depthDesc.DepthEnable = TRUE;
  depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
  depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
  depthDesc.StencilEnable = FALSE;

  hr = m_device->GetD3DDevice()->CreateDepthStencilState(&depthDesc, &m_transparencyDepthState);
  if (FAILED(hr)) {
    return false;
  }

  return true;
}

bool RenderingSystem::CreateLightingTarget()
{
  const UINT width = static_cast<UINT>(m_device->GetViewportWidth());
  const UINT height = static_cast<UINT>(m_device->GetViewportHeight());

  D3D11_TEXTURE2D_DESC texDesc = {};
  texDesc.Width = width;
  texDesc.Height = height;
  texDesc.MipLevels = 1;
  texDesc.ArraySize = 1;
  texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
  texDesc.SampleDesc.Count = 1;
  texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
  texDesc.Usage = D3D11_USAGE_DEFAULT;

  const auto device = m_device->GetD3DDevice();
  if (FAILED(device->CreateTexture2D(&texDesc, nullptr, &m_lightingTex))) {
    return false;
  }

  m_lightingTex->SetPrivateData(WKPDID_D3DDebugObjectName,
    static_cast<UINT>(strlen("m_lightingTex")),
    "m_lightingTex");

  if (FAILED(device->CreateRenderTargetView(m_lightingTex.Get(), nullptr, &m_lightingRTV))) {
    return false;
  }

  if (FAILED(device->CreateShaderResourceView(m_lightingTex.Get(), nullptr, &m_lightingSRV))) {
    return false;
  }

  return true;
}

void RenderingSystem::Cleanup()
{
  m_gbuffer.Release();
  m_lightingTex.Reset();
  m_lightingRTV.Reset();
  m_lightingSRV.Reset();
  m_globalTechnique.reset();
  m_skyboxMesh.reset();
  m_skyboxMatrixBuffer.Reset();
  m_skyboxDepthStencilState.Reset();
  m_skyboxRasterizerState.Reset();
  m_linearSampler.Reset();

  m_transparencyBlendState.Reset();
  m_transparencyDepthState.Reset();

  m_shadowMapArray.Reset();
  m_shadowDSVArray.Reset();
  m_shadowSRVArray.Reset();
  m_lightMatrixBufferArray.Reset();
}

void RenderingSystem::SetViewportDepthRange(float minDepth, float maxDepth) const
{
  const auto     context = m_device->GetImmediateContext();
  D3D11_VIEWPORT vp;
  UINT           numVP = 1;
  context->RSGetViewports(&numVP, &vp);
  vp.MinDepth = minDepth;
  vp.MaxDepth = maxDepth;
  context->RSSetViewports(1, &vp);
}

void RenderingSystem::RenderUI(const std::vector<BaseRendererComponent *> &uiRenderers) const
{
  const auto context = m_device->GetImmediateContext();
  context->OMSetDepthStencilState(nullptr, 0);
  constexpr float blendFactor[4] = { 1, 1, 1, 1 };
  context->OMSetBlendState(m_transparencyBlendState.Get(), blendFactor, 0xFFFFFFFF);
  for (auto &uiComp : uiRenderers) {
    uiComp->Draw(context, XMMatrixIdentity(), XMMatrixIdentity(), RenderPass::UI);
  }
}

void RenderingSystem::Update(float deltaTime)
{
  CameraContext cameraContext;
  const auto *cameraSystem = World::GetInstance().GetSystem<CameraSystem>();
  if (!cameraSystem)
  {
    ErrorLogger::Log("No CameraSystem found.");
    return;
  }

  const auto cameraComponent = cameraSystem->GetActiveCameraComponent();
  const auto cameraTransform = cameraSystem->GetActiveCameraTransform();
  if (!cameraComponent || !cameraTransform)
  {
    ErrorLogger::Log("Active camera or its components are missing.");
    return;
  }

  cameraContext.viewMatrix = cameraComponent->GetViewMatrix();
  cameraContext.projMatrix = cameraComponent->GetProjMatrix();
  cameraContext.viewProjMatrix = cameraComponent->GetViewProjMatrix();
  cameraContext.orthoMatrix = cameraComponent->GetOrthoMatrix();
  {
    const XMVECTOR camPos = cameraTransform->GetWorldPosition();
    XMFLOAT3       camPosF;
    XMStoreFloat3(&camPosF, camPos);
    cameraContext.cameraPosition = camPosF;
  }
  cameraContext.viewportWidth = static_cast<int>(m_device->GetViewportWidth());
  cameraContext.viewportHeight = static_cast<int>(m_device->GetViewportHeight());

  m_opaqueRenderers.clear();
  m_transparentRenderers.clear();

  Frustum frustum;
  frustum.ConstructFrustum(cameraContext.viewMatrix, cameraContext.projMatrix);

#ifdef _DEBUG
  if (auto *debugSystem = World::GetInstance().GetSystem<DebugSystem>())
  {
    debugSystem->SetCurrentFrustum(frustum);
  }
#endif

  std::vector<BaseRendererComponent *> visibleRenderers;
  World::GetInstance().GetOctree().QueryFrustum(frustum, visibleRenderers);

  for (auto *renderer : visibleRenderers)
  {
    (renderer->IsOpaque() ? m_opaqueRenderers : m_transparentRenderers).push_back(renderer);
  }

  // Récupération des Billboards et ajout aux transparents
  World::GetInstance().ForEachComponent<SpriteRenderer>(
    [this](SpriteRenderer *comp)
    {
      if (comp->IsVisible() && comp->IsBillboard())
        m_transparentRenderers.push_back(comp);
    });

  // Tri des transparents par distance
  {
    XMVECTOR camPos = XMLoadFloat3(&cameraContext.cameraPosition);
    std::ranges::sort(m_transparentRenderers,
      [camPos](const BaseRendererComponent *a, const BaseRendererComponent *b)
      {
        const float distA = a->GetDistanceFromCamera(camPos);
        const float distB = b->GetDistanceFromCamera(camPos);
        return distA > distB;
      });
  }

  // Récupération des SpriteRenderer (non-billboard) et ajout aux transparents
  World::GetInstance().ForEachComponent<SpriteRenderer>(
    [this](SpriteRenderer *comp)
    {
      if (comp->IsVisible() && !comp->IsBillboard())
        m_transparentRenderers.push_back(comp);
    });

  // Récupération des UI renderers
  std::vector<BaseRendererComponent *> uiRenderers;
  World::GetInstance().ForEachComponent<UIRendererComponent>(
    [&uiRenderers](UIRendererComponent *comp)
    {
      if (comp->IsVisible()) uiRenderers.push_back(comp);
    });
  World::GetInstance().ForEachComponent<TextRendererComponent>(
    [&uiRenderers](TextRendererComponent *comp)
    {
      if (comp->IsVisible()) uiRenderers.push_back(comp);
    });

  RenderPass renderPassSequence[] = {
      RenderPass::Shadow,
      RenderPass::GBuffer,
      RenderPass::Lighting,
      RenderPass::Skybox,
      RenderPass::Transparency,
      RenderPass::PostProcess,
      RenderPass::Final,
#ifdef _DEBUG
        RenderPass::Debug,
#endif
        RenderPass::UI
  };

  for (const RenderPass currentPass : renderPassSequence)
  {
    BeginRenderPass(currentPass);
    switch (currentPass)
    {
    case RenderPass::Shadow:
      RenderShadowPass(cameraContext);
      break;
    case RenderPass::GBuffer:
      RenderGBufferPass(cameraContext);
      break;
    case RenderPass::Lighting:
      ApplyLightingPass(cameraContext);
      break;
    case RenderPass::Skybox:
      RenderSkyboxPass(cameraContext);
      break;
    case RenderPass::Transparency:
      RenderTransparencyPass(cameraContext);
      break;
    case RenderPass::PostProcess:
      ApplyPostProcessEffects(cameraContext);
      break;
    case RenderPass::Final:
      CompositeFinalImage(cameraContext);
      break;
#ifdef _DEBUG
    case RenderPass::Debug:
      RenderDebugPass(cameraContext);
      break;
#endif
    case RenderPass::UI:
      RenderUI(uiRenderers);
      break;
    default:
      break;
    }
    EndRenderPass(currentPass);
  }

  m_opaqueRenderers.clear();
  m_transparentRenderers.clear();
}

void RenderingSystem::BeginRenderPass(RenderPass pass)
{
  m_activeRenderPassMask |= static_cast<uint32_t>(pass);
  m_currentRenderPass = pass;
  const auto context = m_device->GetImmediateContext();

  switch (pass) {
  case RenderPass::Shadow:
  {
    ID3D11RenderTargetView *nullRTV[1] = { nullptr };
    context->OMSetRenderTargets(1, nullRTV, m_shadowDSVArray.Get());
    context->ClearDepthStencilView(m_shadowDSVArray.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    D3D11_VIEWPORT vp;
    vp.Width = 4096.0f;
    vp.Height = 4096.0f;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    context->RSSetViewports(1, &vp);
  }
  break;
  case RenderPass::GBuffer:
  {
    constexpr float clearColor[4] = { 0, 0, 0, 1 };
    m_gbuffer.Clear(context, clearColor);
    m_gbuffer.SetRenderTargets(context);
  }
  break;
  case RenderPass::Lighting:
  {
    constexpr float clearColor[4] = { 0, 0, 0, 1 };
    context->OMSetRenderTargets(1, m_lightingRTV.GetAddressOf(), nullptr);
    context->ClearRenderTargetView(m_lightingRTV.Get(), clearColor);

    D3D11_VIEWPORT vp;
    vp.Width = m_device->GetViewportWidth();
    vp.Height = m_device->GetViewportHeight();
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    context->RSSetViewports(1, &vp);
  }
  break;
  case RenderPass::Skybox:
  case RenderPass::Transparency:
  {
    context->OMSetRenderTargets(1, m_lightingRTV.GetAddressOf(), m_gbuffer.GetDepthDSV());
  }
  break;
  case RenderPass::PostProcess:
  {
    ID3D11RenderTargetView *nullRTV[1] = { nullptr };
    context->OMSetRenderTargets(1, nullRTV, nullptr);
  }
  break;
  case RenderPass::Final:
  {
    context->OMSetRenderTargets(1, &m_backBufferRTV, nullptr);
    D3D11_VIEWPORT vp;
    vp.Width = m_device->GetViewportWidth();
    vp.Height = m_device->GetViewportHeight();
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    context->RSSetViewports(1, &vp);
  }
  break;
  case RenderPass::UI:
  {
    context->OMSetRenderTargets(1, &m_backBufferRTV, nullptr);
    D3D11_VIEWPORT vp;
    vp.Width = m_device->GetViewportWidth();
    vp.Height = m_device->GetViewportHeight();
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    context->RSSetViewports(1, &vp);
  }
  break;
  default:
    break;
  }
}

void RenderingSystem::EndRenderPass(RenderPass pass)
{
  m_activeRenderPassMask &= ~static_cast<uint32_t>(pass);
  m_currentRenderPass = RenderPass::None;
  const auto context = m_device->GetImmediateContext();
  context->OMSetDepthStencilState(nullptr, 0);
  context->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);

  if (pass == RenderPass::Shadow) {
    D3D11_VIEWPORT vp;
    vp.Width = m_device->GetViewportWidth();
    vp.Height = m_device->GetViewportHeight();
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    context->RSSetViewports(1, &vp);
  }

#ifdef _DEBUG
  if (auto *debugSystem = World::GetInstance().GetSystem<DebugSystem>()) {

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    debugSystem->RenderImGui();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
  }
#endif
}

bool RenderingSystem::IsRenderPassActive(RenderPass pass) const
{
  return (m_activeRenderPassMask & static_cast<uint32_t>(pass)) != 0;
}

void RenderingSystem::RenderShadowPass(const CameraContext &camera)
{
  if (!m_globalTechnique) {
    ErrorLogger::Log("No global technique for shadow pass.");
    return;
  }

  const auto  context = m_device->GetImmediateContext();
  const auto *lightSystem = World::GetInstance().GetSystem<LightSystem>();
  if (!lightSystem) {
    ErrorLogger::Log("No LightSystem found.");
    return;
  }

  std::vector<XMMATRIX> directionalLightMatrices;
  lightSystem->GetDirectionalLightMatrices(directionalLightMatrices);

  D3D11_MAPPED_SUBRESOURCE mapped;
  if (SUCCEEDED(
    context->Map(m_lightMatrixBufferArray.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
    for (size_t i = 0; i < directionalLightMatrices.size(); i++) {
      XMFLOAT4X4 lvp;
      XMStoreFloat4x4(&lvp, XMMatrixTranspose(directionalLightMatrices[i]));
      memcpy(static_cast<char *>(mapped.pData) + i * sizeof(XMFLOAT4X4), &lvp, sizeof(XMFLOAT4X4));
    }
    context->Unmap(m_lightMatrixBufferArray.Get(), 0);
  }

  context->VSSetConstantBuffers(1, 1, m_lightMatrixBufferArray.GetAddressOf());

  const std::vector<std::string> features;
  VertexLayoutDesc               shadowLayout;
  for (const auto &i : Vertex::layout) {
    shadowLayout.elements.push_back(i);
  }

  const ShaderVariant *shadowVariant = m_globalTechnique->GetVariantForPass(
    RenderPass::Shadow, features, shadowLayout);
  if (!shadowVariant) {
    ErrorLogger::Log("No shadow variant found in global technique.");
    return;
  }

  shadowVariant->Apply(context);

  for (UINT i = 0; i < static_cast<UINT>(directionalLightMatrices.size()); i++) {
    Frustum lightFrustum;
    // On suppose que directionalLightMatrices[i] est une matrice ViewProjection
    lightFrustum.ConstructFrustumFromMatrix(directionalLightMatrices[i]);

    // On récupère les objets visibles par cette lumière
    std::vector<BaseRendererComponent *> lightVisibleRenderers;
    World::GetInstance().GetOctree().QueryFrustum(lightFrustum, lightVisibleRenderers);

    D3D11_VIEWPORT vp;
    vp.Width = 4096.0f;
    vp.Height = 4096.0f;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    context->RSSetViewports(1, &vp);

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvSliceDesc = {};
    dsvSliceDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvSliceDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
    dsvSliceDesc.Texture2DArray.ArraySize = 1;
    dsvSliceDesc.Texture2DArray.FirstArraySlice = i;
    dsvSliceDesc.Texture2DArray.MipSlice = 0;

    ComPtr<ID3D11DepthStencilView> dsvSlice;
    if (FAILED(
      m_device->GetD3DDevice()->CreateDepthStencilView(m_shadowMapArray.Get(), &dsvSliceDesc, &
        dsvSlice))) {
      continue;
    }

    ID3D11RenderTargetView *nullRTV[1] = { nullptr };
    context->OMSetRenderTargets(1, nullRTV, dsvSlice.Get());
    context->ClearDepthStencilView(dsvSlice.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    for (const auto &renderer : lightVisibleRenderers) {
      renderer->Draw(context, XMMatrixIdentity(), XMMatrixIdentity(), RenderPass::Shadow);
    }
  }
}

void RenderingSystem::RenderGBufferPass(const CameraContext &camera) const
{
  const auto context = m_device->GetImmediateContext();
  for (auto &renderer : m_opaqueRenderers) {
    renderer->Draw(context, camera.viewMatrix, camera.projMatrix, RenderPass::GBuffer);
  }
}

void RenderingSystem::ApplyLightingPass(const CameraContext &camera)
{
  if (!m_globalTechnique) {
    ErrorLogger::Log("No global technique set for lighting pass.");
    return;
  }

  const auto context = m_device->GetImmediateContext();
  m_gbuffer.SetAsResources(context);

  struct ScreenSizeData {
    float screenWidth;
    float screenHeight;
    float padding[2];
  };
  ScreenSizeData screenData;
  screenData.screenWidth = static_cast<float>(camera.viewportWidth);
  screenData.screenHeight = static_cast<float>(camera.viewportHeight);

  {
    D3D11_MAPPED_SUBRESOURCE mapped;
    if (SUCCEEDED(context->Map(m_screenSizeBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
      memcpy(mapped.pData, &screenData, sizeof(screenData));
      context->Unmap(m_screenSizeBuffer.Get(), 0);
    }
  }
  context->PSSetConstantBuffers(1, 1, m_screenSizeBuffer.GetAddressOf());

  if (auto *lightSystem = World::GetInstance().GetSystem<LightSystem>()) {
    lightSystem->FillLightBuffer(context, camera.cameraPosition, World::GetInstance());

    context->PSSetShaderResources(3, 1, m_shadowSRVArray.GetAddressOf());
    context->PSSetSamplers(1, 1, m_shadowSampler.GetAddressOf());
    context->VSSetConstantBuffers(4, 1, m_lightMatrixBufferArray.GetAddressOf());
    context->PSSetConstantBuffers(4, 1, m_lightMatrixBufferArray.GetAddressOf());
  }
  else {
    ErrorLogger::Log("No LightSystem found during lighting pass.");
  }

  VertexLayoutDesc fullscreenLayout;
  fullscreenLayout.elements = {};
  const std::vector<std::string> features;

  const ShaderVariant *lightingVariant = m_globalTechnique->GetVariantForPass(
    RenderPass::Lighting, features, fullscreenLayout
  );

  if (!lightingVariant) {
    ErrorLogger::Log("No lighting variant found in global technique.");
    return;
  }

  lightingVariant->Apply(context);
  context->IASetInputLayout(nullptr);
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context->Draw(3, 0);
}

void RenderingSystem::RenderSkyboxPass(const CameraContext &camera)
{
  if (!m_globalTechnique) {
    ErrorLogger::Log("No global technique set for skybox pass.");
    return;
  }

  if (!m_skyboxVariant) {
    ErrorLogger::Log("No skybox variant found in global technique.");
    return;
  }

  const auto context = m_device->GetImmediateContext();

  context->OMSetDepthStencilState(m_skyboxDepthStencilState.Get(), 0);
  context->RSSetState(m_skyboxRasterizerState.Get());

  XMMATRIX viewNoTrans = camera.viewMatrix;
  viewNoTrans.r[3] = XMVectorSet(0, 0, 0, 1);

  const XMMATRIX viewProj = XMMatrixMultiply(viewNoTrans, camera.projMatrix);

  XMFLOAT4X4 vpFloat4x4;
  XMStoreFloat4x4(&vpFloat4x4, XMMatrixTranspose(viewProj));

  {
    D3D11_MAPPED_SUBRESOURCE mapped;
    if (SUCCEEDED(
      context->Map(m_skyboxMatrixBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
      memcpy(mapped.pData, &vpFloat4x4, sizeof(vpFloat4x4));
      context->Unmap(m_skyboxMatrixBuffer.Get(), 0);
    }
  }

  ID3D11ShaderResourceView *skyCubeSRV;
  if (m_skyboxTexture && m_skyboxTexture->GetShaderResourceView()) {
    skyCubeSRV = m_skyboxTexture->GetShaderResourceView().Get();
  }
  else {
    skyCubeSRV = TextureManager::GetInstance().GetTexture(L"__white_cubemap_fallback__")
      ->GetShaderResourceView().Get();
  }

  context->PSSetShaderResources(0, 1, &skyCubeSRV);
  context->PSSetSamplers(0, 1, m_linearSampler.GetAddressOf());
  context->VSSetConstantBuffers(0, 1, m_skyboxMatrixBuffer.GetAddressOf());

  m_skyboxVariant->Apply(context);

  constexpr UINT stride = sizeof(Vertex);
  constexpr UINT offset = 0;
  ID3D11Buffer *vb = m_skyboxMesh->GetVertexBuffer();
  ID3D11Buffer *ib = m_skyboxMesh->GetIndexBuffer();

  context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
  context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  context->DrawIndexed(m_skyboxMesh->GetIndexCount(), 0, 0);

  context->OMSetDepthStencilState(nullptr, 0);
  context->RSSetState(nullptr);
}

void RenderingSystem::RenderTransparencyPass(const CameraContext &camera) const
{
  if (!m_globalTechnique) {
    ErrorLogger::Log("No global technique set for transparency pass.");
    return;
  }

  const auto context = m_device->GetImmediateContext();

  context->OMSetBlendState(m_transparencyBlendState.Get(), nullptr, 0xFFFFFFFF);
  context->OMSetDepthStencilState(m_transparencyDepthState.Get(), 0);

  for (auto &renderer : m_transparentRenderers) {
    renderer->Draw(context, camera.viewMatrix, camera.projMatrix, RenderPass::Transparency);
  }
}

void RenderingSystem::ApplyPostProcessEffects(const CameraContext &camera) const
{
  if (!m_globalTechnique) {
    ErrorLogger::Log("No global technique set for postprocess pass.");
    return;
  }

  const auto context = m_device->GetImmediateContext();

  ID3D11RenderTargetView *nullRTV[1] = { nullptr };
  context->OMSetRenderTargets(1, nullRTV, nullptr);

  ID3D11ShaderResourceView *inputSrv = m_lightingSRV.Get();
  context->PSSetShaderResources(0, 1, &inputSrv);

  ID3D11SamplerState *sampler = m_linearSampler.Get();
  context->PSSetSamplers(0, 1, &sampler);

  ID3D11RenderTargetView *rtv = m_postProcessRTV.Get();
  context->OMSetRenderTargets(1, &rtv, nullptr);
  constexpr float clearColor[4] = { 0, 0, 0, 1 };
  context->ClearRenderTargetView(m_postProcessRTV.Get(), clearColor);

  D3D11_VIEWPORT vp;
  vp.Width = m_device->GetViewportWidth();
  vp.Height = m_device->GetViewportHeight();
  vp.MinDepth = 0.0f;
  vp.MaxDepth = 1.0f;
  vp.TopLeftX = 0.0f;
  vp.TopLeftY = 0.0f;
  context->RSSetViewports(1, &vp);

  VertexLayoutDesc fullscreenLayout;
  fullscreenLayout.elements = {};
  const std::vector<std::string> features;

  const ShaderVariant *ppVariant = m_globalTechnique->GetVariantForPass(
    RenderPass::PostProcess, features, fullscreenLayout
  );

  if (!ppVariant) {
    ErrorLogger::Log("No post-process variant found in global technique.");
    return;
  }

  ppVariant->Apply(context);
  context->IASetInputLayout(nullptr);
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context->Draw(3, 0);
}

void RenderingSystem::RenderDebugPass(const CameraContext &camera)
{
  if (!m_globalTechnique) {
    ErrorLogger::Log("No global technique set for debug pass.");
    return;
  }

  const auto context = m_device->GetImmediateContext();

  auto *debugSystem = World::GetInstance().GetSystem<DebugSystem>();
  if (!debugSystem) {
    return;
  }

  const auto &debugVertices = debugSystem->GetDebugLineVertices();
  if (debugVertices.empty()) return;

  UINT requiredSizeInBytes = static_cast<UINT>(debugVertices.size() * sizeof(DebugLineVertex));

  if (!m_debugLineVB || requiredSizeInBytes > m_debugVBSizeInBytes) {
    m_debugLineVB.Reset();

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = requiredSizeInBytes;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT hr = m_device->GetD3DDevice()->CreateBuffer(&bd, nullptr, &m_debugLineVB);
    if (FAILED(hr)) {
      ErrorLogger::Log("Failed to create debug line vertex buffer.");
      return;
    }

    m_debugVBSizeInBytes = requiredSizeInBytes;
  }

  D3D11_MAPPED_SUBRESOURCE mapped;
  if (SUCCEEDED(context->Map(m_debugLineVB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
    memcpy(mapped.pData, debugVertices.data(), debugVertices.size() * sizeof(DebugLineVertex));
    context->Unmap(m_debugLineVB.Get(), 0);
  }
  else {
    ErrorLogger::Log("Failed to map debug line vertex buffer.");
    return;
  }

  XMMATRIX   viewProj = XMMatrixMultiply(camera.viewMatrix, camera.projMatrix);
  XMFLOAT4X4 vpFloat4x4;
  XMStoreFloat4x4(&vpFloat4x4, XMMatrixTranspose(viewProj));

  if (SUCCEEDED(context->Map(m_debugMatrixBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
    memcpy(mapped.pData, &vpFloat4x4, sizeof(vpFloat4x4));
    context->Unmap(m_debugMatrixBuffer.Get(), 0);
  }
  else {
    ErrorLogger::Log("Failed to map debug matrix buffer.");
    return;
  }

  context->VSSetConstantBuffers(0, 1, m_debugMatrixBuffer.GetAddressOf());

  const std::vector<std::string> features;
  VertexLayoutDesc               debugLayout;
  D3D11_INPUT_ELEMENT_DESC       debugElements[] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
  };
  for (auto &e : debugElements) {
    debugLayout.elements.push_back(e);
  }

  const ShaderVariant *debugVariant = m_globalTechnique->GetVariantForPass(
    RenderPass::Debug, features, debugLayout
  );

  if (!debugVariant) {
    ErrorLogger::Log("No debug variant found in global technique.");
    return;
  }

  debugVariant->Apply(context);

  UINT          stride = sizeof(DebugLineVertex);
  UINT          offset = 0;
  ID3D11Buffer *vb = m_debugLineVB.Get();
  context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

  context->Draw(static_cast<UINT>(debugVertices.size()), 0);

  debugSystem->ClearDebugLineVertices();
}

void RenderingSystem::CompositeFinalImage(const CameraContext &camera) const
{
  if (!m_globalTechnique) {
    ErrorLogger::Log("No global technique set for final pass.");
    return;
  }

  const auto context = m_device->GetImmediateContext();

  ID3D11ShaderResourceView *finalInputSrv = m_postProcessSRV.Get();
  context->PSSetShaderResources(0, 1, &finalInputSrv);

  ID3D11SamplerState *sampler = m_linearSampler.Get();
  context->PSSetSamplers(0, 1, &sampler);

  VertexLayoutDesc fullscreenLayout;
  fullscreenLayout.elements = {};
  const std::vector<std::string> features;

  const ShaderVariant *finalVariant = m_globalTechnique->GetVariantForPass(
    RenderPass::Final, features, fullscreenLayout
  );

  if (!finalVariant) {
    ErrorLogger::Log("No final variant found in global technique.");
    return;
  }

  finalVariant->Apply(context);
  context->IASetInputLayout(nullptr);
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context->Draw(3, 0);
}

void RenderingSystem::SetSkyboxTexture(Texture *pSkyboxTexture)
{
  m_skyboxTexture = pSkyboxTexture;
}

void RenderingSystem::SetGlobalTechniqueName(const std::string &techniqueName)
{
  auto technique = ShaderTechniqueFactory::CreateTechnique(techniqueName);
  if (!technique) {
    ErrorLogger::Log("Failed to create global technique with name: " + techniqueName);
  }
  m_globalTechnique = std::move(technique);

  if (m_globalTechnique) {
    const std::vector<std::string> features;
    VertexLayoutDesc               skyboxLayout;
    for (const auto &i : Vertex::layout) {
      skyboxLayout.elements.push_back(i);
    }
    m_skyboxVariant = m_globalTechnique->GetVariantForPass(RenderPass::Skybox, features,
      skyboxLayout);
    if (!m_skyboxVariant) {
      ErrorLogger::Log("No skybox variant found after technique change.");
    }
  }
}

void RenderingSystem::SetGlobalTechnique(std::unique_ptr<ShaderTechnique> technique)
{
  m_globalTechnique = std::move(technique);

  if (m_globalTechnique) {
    const std::vector<std::string> features;
    VertexLayoutDesc               skyboxLayout;
    for (const auto &i : Vertex::layout) {
      skyboxLayout.elements.push_back(i);
    }
    m_skyboxVariant = m_globalTechnique->GetVariantForPass(RenderPass::Skybox, features,
      skyboxLayout);
    if (!m_skyboxVariant) {
      ErrorLogger::Log("No skybox variant found after technique change.");
    }
  }
}
