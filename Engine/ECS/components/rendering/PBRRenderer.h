#pragma once
#include "Engine/ECS/components/rendering/BaseRendererComponent.h"
#include "Engine/Shaders/techniques/ShaderTechnique.h"
#include <DirectXMath.h>

namespace FrostFireEngine
{
  class Texture;

  class PBRRenderer : public BaseRendererComponent {
  public:
    PBRRenderer(ID3D11Device* device);
    ~PBRRenderer() override;

    void OnAttach() override
    {
      BaseRendererComponent::OnAttach();
      if (m_world && m_world->IsOctreeBuilt()) {
        m_world->InsertOctreeEntity(GetOwner());
      }
    }

    void OnDetach() override
    {
      BaseRendererComponent::OnDetach();
      if (m_world && m_world->IsOctreeBuilt()) {
        m_world->RemoveOctreeEntity(GetOwner());
      }
    }

    void Draw(ID3D11DeviceContext* deviceContext,
              const XMMATRIX&      viewMatrix,
              const XMMATRIX&      projectionMatrix,
              RenderPass           currentPass) override;

    const VertexLayoutDesc& GetVertexLayout() const override;
    ShaderTechnique*        GetTechnique() const override;

    // Paramètres PBR
    void SetBaseColor(const XMFLOAT4& color) const;
    void SetMetallic(float metallic) const;
    void SetRoughness(float roughness) const;
    void SetAmbientOcclusion(float ao) const;

    XMFLOAT4 GetBaseColor() const;
    float    GetMetallic() const;
    float    GetRoughness() const;
    float    GetAO() const;

    // Textures PBR
    void SetAlbedoTexture(Texture* texture)
    {
      m_albedoTexture = texture;
    }
    void SetNormalMap(Texture* texture)
    {
      m_normalMap = texture;
    }
    void SetMetallicRoughnessMap(Texture* texture)
    {
      m_metallicRoughnessMap = texture;
    }
    void SetAOMap(Texture* texture)
    {
      m_aoMap = texture;
    }

    void SetIrradianceMap(Texture *texture) { m_irradianceMap = texture; }
    void SetPrefilteredEnvMap(Texture *texture) { m_prefilteredEnvMap = texture; }
    void SetBRDFLUT(Texture *texture) { m_brdfLUT = texture; }


    Texture* GetAlbedoTexture() const
    {
      return m_albedoTexture;
    }
    Texture* GetNormalMap() const
    {
      return m_normalMap;
    }
    Texture* GetMetallicRoughnessMap() const
    {
      return m_metallicRoughnessMap;
    }
    Texture* GetAOMap() const
    {
      return m_aoMap;
    }

    // Move
    PBRRenderer(PBRRenderer&& other) noexcept;
    PBRRenderer& operator=(PBRRenderer&& other) noexcept;

  private:
    bool InitializeConstantBuffers(ID3D11Device* device) override;

    VertexLayoutDesc m_layout;

    Texture* m_albedoTexture = nullptr;
    Texture* m_normalMap = nullptr;
    Texture* m_metallicRoughnessMap = nullptr;
    Texture* m_aoMap = nullptr;
    Texture *m_irradianceMap = nullptr;
    Texture *m_prefilteredEnvMap = nullptr;
    Texture *m_brdfLUT = nullptr;


    ID3D11SamplerState* m_defaultSamplerState = nullptr;
  };
}
