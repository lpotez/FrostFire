#pragma once
#include "Engine/ECS/components/rendering/BaseRendererComponent.h"
#include "Engine/Shaders/features/RenderPass.h"
#include "Engine/Shaders/techniques/ShaderTechnique.h"
#include "Engine/Mesh.h"
#include <DirectXMath.h>

namespace FrostFireEngine
{
  class Texture;

  class SpriteRenderer : public BaseRendererComponent {
  public:
    SpriteRenderer(ID3D11Device* device);
    ~SpriteRenderer() override;
    SpriteRenderer(SpriteRenderer&& other) noexcept;
    SpriteRenderer& operator=(SpriteRenderer&& other) noexcept;

    void Draw(ID3D11DeviceContext* deviceContext,
              const DirectX::XMMATRIX& viewMatrix,
              const DirectX::XMMATRIX& projectionMatrix,
              RenderPass currentPass) override;

    const VertexLayoutDesc& GetVertexLayout() const override;
    ShaderTechnique* GetTechnique() const override;

    void SetTexture(Texture* texture);
    bool InitializeConstantBuffers(ID3D11Device* device) override;

    void SetOpacity(float opacity);
    float GetOpacity() const;

    void SetBillboard(bool enabled);
    bool IsBillboard() const;

  private:
    VertexLayoutDesc m_layout;
    ID3D11SamplerState* m_defaultSamplerState;
    Texture* m_texture;
    std::shared_ptr<Mesh> m_quadMesh;
    float m_opacity;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_materialBuffer;
    bool m_billboard;
  };
}
