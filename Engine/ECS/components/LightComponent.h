#pragma once
#include "Engine/ECS/core/Component.h"
#include <DirectXMath.h>

namespace FrostFireEngine
{
  using namespace DirectX;

  enum class LightType {
    Directional,
    Point,
    Spot
  };

  class LightComponent : public Component {
  public:
    LightComponent() = default;
    LightComponent(LightType       type,
                   const XMFLOAT3& color,
                   float           intensity,
                   const XMFLOAT3& direction = {0, -1, 0},
                   float           range = 100.0f,
                   float           spotAngle = 45.0f)
      : m_type(type), m_color(color), m_intensity(intensity), m_direction(direction),
        m_range(range), m_spotAngle(spotAngle)
    {
    }

    LightType GetType() const
    {
      return m_type;
    }
    const XMFLOAT3& GetColor() const
    {
      return m_color;
    }
    float GetIntensity() const
    {
      return m_intensity;
    }
    const XMFLOAT3& GetDirection() const
    {
      return m_direction;
    }
    float GetRange() const
    {
      return m_range;
    }
    float GetSpotAngle() const
    {
      return m_spotAngle;
    }

    void SetType(LightType t)
    {
      m_type = t;
    }
    void SetColor(const XMFLOAT3& c)
    {
      m_color = c;
    }
    void SetIntensity(float i)
    {
      m_intensity = i;
    }
    void SetDirection(const XMFLOAT3& d)
    {
      m_direction = d;
    }
    void SetRange(float r)
    {
      m_range = r;
    }
    void SetSpotAngle(float a)
    {
      m_spotAngle = a;
    }

  private:
    LightType m_type = LightType::Directional;
    XMFLOAT3  m_color = {1, 1, 1};
    float     m_intensity = 1.0f;
    XMFLOAT3  m_direction = {0, -1, 0};
    float     m_range = 100.0f;
    float     m_spotAngle = 45.0f; // en degrés
  };
}
