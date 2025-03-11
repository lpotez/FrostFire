  #include "GBuffer.h"
  #include "Engine/Utils/ErrorLogger.h"

  bool GBuffer::Initialize(ID3D11Device* device, UINT width, UINT height)
  {
    m_width = width;
    m_height = height;

    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.SampleDesc.Count = 1;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    texDesc.Usage = D3D11_USAGE_DEFAULT;

    // Position
    if (FAILED(device->CreateTexture2D(&texDesc, nullptr, &m_positionTex))) {
      ErrorLogger::Log("Failed to create GBuffer position texture");
      return false;
    }
    if (FAILED(device->CreateRenderTargetView(m_positionTex.Get(), nullptr, &m_positionRTV))) {
      ErrorLogger::Log("Failed to create GBuffer position RTV");
      return false;
    }
    if (FAILED(device->CreateShaderResourceView(m_positionTex.Get(), nullptr, &m_positionSRV))) {
      ErrorLogger::Log("Failed to create GBuffer position SRV");
      return false;
    }

    // Normal
    if (FAILED(device->CreateTexture2D(&texDesc, nullptr, &m_normalTex))) {
      ErrorLogger::Log("Failed to create GBuffer normal texture");
      return false;
    }
    if (FAILED(device->CreateRenderTargetView(m_normalTex.Get(), nullptr, &m_normalRTV))) {
      ErrorLogger::Log("Failed to create GBuffer normal RTV");
      return false;
    }
    if (FAILED(device->CreateShaderResourceView(m_normalTex.Get(), nullptr, &m_normalSRV))) {
      ErrorLogger::Log("Failed to create GBuffer normal SRV");
      return false;
    }

    // Albedo
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    if (FAILED(device->CreateTexture2D(&texDesc, nullptr, &m_albedoTex))) {
      ErrorLogger::Log("Failed to create GBuffer albedo texture");
      return false;
    }
    if (FAILED(device->CreateRenderTargetView(m_albedoTex.Get(), nullptr, &m_albedoRTV))) {
      ErrorLogger::Log("Failed to create GBuffer albedo RTV");
      return false;
    }
    if (FAILED(device->CreateShaderResourceView(m_albedoTex.Get(), nullptr, &m_albedoSRV))) {
      ErrorLogger::Log("Failed to create GBuffer albedo SRV");
      return false;
    }

    // Depth : utilisation d'un format "typeless" pour permettre la création d'une SRV
    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_R24G8_TYPELESS; // typeless
    depthDesc.SampleDesc.Count = 1;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;

    if (FAILED(device->CreateTexture2D(&depthDesc, nullptr, &m_depthTex))) {
      ErrorLogger::Log("Failed to create GBuffer depth texture");
      return false;
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;
    if (FAILED(device->CreateDepthStencilView(m_depthTex.Get(), &dsvDesc, &m_depthDSV))) {
      ErrorLogger::Log("Failed to create GBuffer depth DSV");
      return false;
    }

    // Création de la SRV pour le depth buffer
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;

    if (FAILED(device->CreateShaderResourceView(m_depthTex.Get(), &srvDesc, &m_depthSRV))) {
      ErrorLogger::Log("Failed to create SRV for GBuffer depth");
      return false;
    }

    return true;
  }

  void GBuffer::Clear(ID3D11DeviceContext* context, const float clearColor[4]) const
  {
    context->ClearRenderTargetView(m_positionRTV.Get(), clearColor);
    context->ClearRenderTargetView(m_normalRTV.Get(), clearColor);
    context->ClearRenderTargetView(m_albedoRTV.Get(), clearColor);
    context->ClearDepthStencilView(m_depthDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f,
                                   0);
  }

  void GBuffer::SetRenderTargets(ID3D11DeviceContext* context) const
  {
    ID3D11RenderTargetView* rtvs[] = {
      m_positionRTV.Get(),
      m_normalRTV.Get(),
      m_albedoRTV.Get()
    };
    context->OMSetRenderTargets(std::size(rtvs), rtvs, m_depthDSV.Get());
  }

  void GBuffer::SetAsResources(ID3D11DeviceContext* context) const
  {
    // Méthode utile lors de la passe lighting pour binder les SRV
    ID3D11ShaderResourceView* srvs[] = {
      m_positionSRV.Get(),
      m_normalSRV.Get(),
      m_albedoSRV.Get()
    };
    context->PSSetShaderResources(0, std::size(srvs), srvs);
  }

  void GBuffer::Release()
  {
    m_positionTex.Reset();
    m_normalTex.Reset();
    m_albedoTex.Reset();
    m_depthTex.Reset();

    m_positionRTV.Reset();
    m_normalRTV.Reset();
    m_albedoRTV.Reset();
    m_positionSRV.Reset();
    m_normalSRV.Reset();
    m_albedoSRV.Reset();
    m_depthDSV.Reset();
  }
