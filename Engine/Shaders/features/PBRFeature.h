#pragma once
#include "Engine/Shaders/features/BaseShaderFeature.h"
#include <DirectXMath.h>

namespace FrostFireEngine
{
  struct PBRMaterialParams {
    DirectX::XMFLOAT4 baseColor; // RGBA
    float             metallic; // [0,1]
    float             roughness; // [0,1]
    float             ao; // [0,1]
    float             padding; // Alignement
  };

  class PBRFeature : public BaseShaderFeature {
  public:
    PBRFeature(ID3D11Device* device);
    ~PBRFeature() override;

    std::vector<std::string> GetDefines() const override;
    void                     UpdateParameters(ID3D11DeviceContext* context) override;
    std::string              GetId() const override;
    FeatureMetadata          GetMetadata() const override;
    uint32_t                 GetFeatureGroup() const override;

    void SetBaseColor(const DirectX::XMFLOAT4& color);
    void SetMetallic(float metallic);
    void SetRoughness(float roughness);
    void SetAmbientOcclusion(float ao);

    DirectX::XMFLOAT4 GetBaseColor() const
    {
      return m_params.baseColor;
    }
    float GetMetallic() const
    {
      return m_params.metallic;
    }
    float GetRoughness() const
    {
      return m_params.roughness;
    }
    float GetAO() const
    {
      return m_params.ao;
    }

  private:
    bool InitializeBuffers(ID3D11Device* device);

    PBRMaterialParams m_params;
    ID3D11Buffer*     m_materialBuffer;
  };
}
