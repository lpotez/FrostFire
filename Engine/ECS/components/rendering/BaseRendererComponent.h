#pragma once
#include "Engine/ECS/core/Component.h"
#include "Engine/Shaders/features/BaseShaderFeature.h"
#include "Engine/Shaders/features/RenderPass.h"
#include "Engine/Shaders/techniques/ShaderTechnique.h"
#include <DirectXMath.h>
#include <memory>
#include <vector>
#include <string>

#include "Engine/ECS/components/transform/TransformComponent.h"
#include "Engine/ECS/core/World.h"

namespace FrostFireEngine
{
  using namespace DirectX;

  struct TransformMatrixBuffer {
    XMMATRIX modelViewProjection;
    XMMATRIX world;
    XMMATRIX worldInverseTranspose;
  };

  class BaseRendererComponent : public Component {
  public:
    BaseRendererComponent();
    ~BaseRendererComponent() override;

    virtual void Draw(ID3D11DeviceContext* deviceContext,
                      const XMMATRIX&      viewMatrix,
                      const XMMATRIX&      projectionMatrix,
                      RenderPass           currentPass) = 0;

    virtual const VertexLayoutDesc& GetVertexLayout() const = 0;
    virtual ShaderTechnique*        GetTechnique() const = 0;

    void AddFeature(std::unique_ptr<BaseShaderFeature> feature);
    void SetTechnique(std::unique_ptr<ShaderTechnique> technique);

    void SetVisible(const bool visible)
    {
      m_visible = visible;
    }

    bool IsVisible() const
    {
      return m_visible;
    }

    void SetOpaque(const bool opaque)
    {
      m_opaque = opaque;
    }

    bool IsOpaque() const
    {
      return m_opaque;
    }

    std::vector<std::string> GetActiveFeatures() const;

    virtual float GetDistanceFromCamera(const XMVECTOR& cameraPosition) const
    {
      if (const auto owner = World::GetInstance().GetEntity(GetOwner())) {
        if (const auto* transform = owner->GetComponent<TransformComponent>()) {
          const XMVECTOR objPos = transform->GetWorldPosition();
          const XMVECTOR diff = XMVectorSubtract(objPos, cameraPosition);
          const XMVECTOR lengthV = XMVector3Length(diff);
          float          dist;
          XMStoreFloat(&dist, lengthV);
          return dist;
        }
      }
      return 0.0f;
    }


    // Empêcher la copie
    BaseRendererComponent(const BaseRendererComponent&) = delete;
    BaseRendererComponent& operator=(const BaseRendererComponent&) = delete;

    // Autoriser le déplacement
    BaseRendererComponent(BaseRendererComponent&& other) noexcept;
    BaseRendererComponent& operator=(BaseRendererComponent&& other) noexcept;

  protected:
    virtual bool InitializeConstantBuffers(ID3D11Device* device);
    bool         UpdateConstantBuffers(ID3D11DeviceContext* deviceContext,
                               const XMMATRIX&              worldMatrix,
                               const XMMATRIX&              viewMatrix,
                               const XMMATRIX&              projMatrix) const;

    std::vector<std::unique_ptr<BaseShaderFeature>> m_features;
    ShaderTechnique*                                m_technique;
    std::unique_ptr<ShaderTechnique>                m_ownedTechnique;
    ID3D11Buffer*                                   m_matrixBuffer;
    bool                                            m_visible;
    bool                                            m_opaque;

    World* m_world = nullptr;
  };
}
