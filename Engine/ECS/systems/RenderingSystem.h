#pragma once

#include <memory>
#include <vector>
#include <wrl/client.h>
#include <d3d11.h>
#include <DirectXMath.h>

#include "Engine/CameraContext.h"
#include "Engine/Core/D3DResources.h"
#include "Engine/ECS/core/System.h"
#include "Engine/Shaders/ShaderManager.h"
#include "Engine/Shaders/features/RenderPass.h"
#include "rendering/GBuffer.h"

namespace FrostFireEngine
{
  class DispositifD3D11;
  class Texture;
  class Mesh;
  class BaseRendererComponent;
  class ShaderTechnique;
  class ShaderVariant;

  class RenderingSystem : public System {
  public:
    RenderingSystem(DispositifD3D11* device);

    void Initialize() override;
    void Cleanup() override;
    void Update(float deltaTime) override;

    void SetGlobalTechniqueName(const std::string& techniqueName);
    void SetGlobalTechnique(std::unique_ptr<ShaderTechnique> technique);
    void SetSkyboxTexture(Texture* pSkyboxTexture);

  private:
    bool CreateLightingTarget();
    bool CreateShadowMapArray(UINT count);
    bool CreateTransparencyStates();

    void BeginRenderPass(RenderPass pass);
    void EndRenderPass(RenderPass pass);
    bool IsRenderPassActive(RenderPass pass) const;

    void RenderShadowPass(const CameraContext& camera);
    void RenderGBufferPass(const CameraContext& camera) const;
    void ApplyLightingPass(const CameraContext& camera);
    void RenderTransparencyPass(const CameraContext& camera) const;
    void ApplyPostProcessEffects(const CameraContext& camera) const;
    void RenderDebugPass(const CameraContext& camera);
    void RenderSkyboxPass(const CameraContext& camera);
    void CompositeFinalImage(const CameraContext& camera) const;

    void SetViewportDepthRange(float minDepth, float maxDepth) const;
    void RenderUI(const std::vector<BaseRendererComponent*>& uiRenderers) const;

  private:
    std::unique_ptr<D3DResources> m_d3dResources;
    DispositifD3D11*              m_device;
    ShaderManager*                m_shaderManager;

    GBuffer m_gbuffer;

    // Rendertarget pour le lighting pass
    ComPtr<ID3D11Texture2D>          m_lightingTex;
    ComPtr<ID3D11RenderTargetView>   m_lightingRTV;
    ComPtr<ID3D11ShaderResourceView> m_lightingSRV;

    // Shadow map array pour plusieurs lumières directionnelles
    ComPtr<ID3D11Texture2D>          m_shadowMapArray;
    ComPtr<ID3D11DepthStencilView>   m_shadowDSVArray;
    ComPtr<ID3D11ShaderResourceView> m_shadowSRVArray;
    UINT                             m_shadowMapArraySize = 0;

    // Buffers
    ComPtr<ID3D11Buffer> m_screenSizeBuffer;
    ComPtr<ID3D11Buffer> m_lightMatrixBufferArray;
    ComPtr<ID3D11Buffer> m_debugLineVB;
    ComPtr<ID3D11Buffer> m_debugMatrixBuffer;

    ID3D11RenderTargetView* m_backBufferRTV = nullptr;

    std::unique_ptr<ShaderTechnique> m_globalTechnique;

    Texture* m_skyboxTexture = nullptr;

    ComPtr<ID3D11SamplerState> m_linearSampler;
    ComPtr<ID3D11SamplerState> m_shadowSampler;

    std::shared_ptr<Mesh> m_skyboxMesh;
    ComPtr<ID3D11Buffer>  m_skyboxMatrixBuffer;

    ComPtr<ID3D11DepthStencilState> m_skyboxDepthStencilState;
    ComPtr<ID3D11RasterizerState>   m_skyboxRasterizerState;
    ShaderVariant*                  m_skyboxVariant = nullptr;

    ComPtr<ID3D11RenderTargetView>   m_postProcessRTV;
    ComPtr<ID3D11ShaderResourceView> m_postProcessSRV;

    // États pour la transparence
    ComPtr<ID3D11BlendState>        m_transparencyBlendState;
    ComPtr<ID3D11DepthStencilState> m_transparencyDepthState;

    std::vector<BaseRendererComponent*> m_transparentRenderers;
    std::vector<BaseRendererComponent*> m_opaqueRenderers;

    size_t m_debugVBSizeInBytes = 0;

    struct DebugLineVertex {
      XMFLOAT3 pos;
      XMFLOAT4 color;
    };

    uint32_t   m_activeRenderPassMask = 0;
    RenderPass m_currentRenderPass = RenderPass::None;
  };
}
