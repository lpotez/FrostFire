#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <vector>

class GBuffer {
public:
  bool Initialize(ID3D11Device* device, UINT width, UINT height);
  void Clear(ID3D11DeviceContext* context, const float clearColor[4]) const;
  void SetRenderTargets(ID3D11DeviceContext* context) const;
  void SetAsResources(ID3D11DeviceContext* context) const;
  void Release();

  ID3D11ShaderResourceView* GetPositionSRV() const
  {
    return m_positionSRV.Get();
  }
  ID3D11ShaderResourceView* GetNormalSRV() const
  {
    return m_normalSRV.Get();
  }
  ID3D11ShaderResourceView* GetAlbedoSRV() const
  {
    return m_albedoSRV.Get();
  }

  ID3D11DepthStencilView* GetDepthDSV() const
  {
    return m_depthDSV.Get();
  }

  // Ajouter un getter pour la SRV du depth, si vous le souhaitez
  ID3D11ShaderResourceView* GetDepthSRV() const
  {
    return m_depthSRV.Get();
  }

private:
  Microsoft::WRL::ComPtr<ID3D11Texture2D> m_positionTex;
  Microsoft::WRL::ComPtr<ID3D11Texture2D> m_normalTex;
  Microsoft::WRL::ComPtr<ID3D11Texture2D> m_albedoTex;
  Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthTex;

  Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_positionRTV;
  Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_normalRTV;
  Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_albedoRTV;

  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_positionSRV;
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_normalSRV;
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_albedoSRV;
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_depthSRV;

  Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthDSV;

  UINT m_width = 0;
  UINT m_height = 0;
};
