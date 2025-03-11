#pragma once
#include <algorithm>

#include "Engine/ECS/core/Component.h"
#include "Engine/DispositifD3D11.h"
#include "Engine/InputManager.h"
#include "Engine/ECS/components/transform/TransformComponent.h"
#include "Engine/ECS/components/rendering/UIRendererComponent.h"
#include "Engine/ECS/components/transform/RectTransformComponent.h"
#include <functional>
#include "ButtonComponent.h"

namespace FrostFireEngine
{

  class SliderComponent : public Component {
  public:
    using ButtonCallback = std::function<void()>;

    enum class State {
      Normal,
      Hovered,
      Pressed,
      Disabled
    };

    using ValueChangedCallback = std::function<void(float)>;

    explicit SliderComponent(DispositifD3D11 *pDispositifD3D11, EntityId cursorId) :
      pDispositifD3D11(pDispositifD3D11),
      cursorId(cursorId)
      , uiRendererSlidebarComponent(nullptr)
      , uiRendererCursorComponent(nullptr)
      , rectTransformSlidebar(nullptr)
      , rectTransformCursor(nullptr)
      , transformSlidebar(nullptr)
      , transformCursor(nullptr)
    {

    }

    void Initialize() {
      // Slidebar
      auto entity = World::GetInstance().GetEntity(owner);
      uiRendererSlidebarComponent = entity->GetComponent<UIRendererComponent>();
      rectTransformSlidebar = entity->GetComponent<RectTransformComponent>();
      transformSlidebar = entity->GetComponent<TransformComponent>();

      sliderStartX = rectTransformSlidebar->GetAnchorOffset().x - rectTransformSlidebar->GetSize().x / 2.0f;
      sliderWidth = rectTransformSlidebar->GetSize().x;

      // Cursor
      auto cursorEntity = World::GetInstance().GetEntity(cursorId);
      uiRendererCursorComponent = cursorEntity->GetComponent<UIRendererComponent>();
      rectTransformCursor = cursorEntity->GetComponent<RectTransformComponent>();
      transformCursor = cursorEntity->GetComponent<TransformComponent>();
    }


    void OnClick(const ButtonComponent::ButtonCallback &callback)
    {
      onClick_ = callback;
    }

    float GetValue() const { return value; }


    void UpdateCursorPosition(float newPosX)
    {
      if (!rectTransformCursor) return;
      if (newPosX < 0) newPosX = 0;
      if (newPosX > sliderWidth) newPosX = sliderWidth;
      rectTransformCursor->SetAnchorOffset({ newPosX, rectTransformCursor->GetAnchorOffset().y });
    }
    void Update()
    {
      if (!initialized_)
      {
        Initialize();
        initialized_ = true;
      }

      if (!IsEnabled() || !uiRendererSlidebarComponent || !rectTransformSlidebar)
      {
        return;
      }

      const auto *inputManager = &InputManager::GetInstance();

      const float mouseX = static_cast<float>(inputManager->GetMouseX());
      const float mouseY = static_cast<float>(inputManager->GetMouseY());

      const bool isInside = IsPointInside(mouseX, mouseY);
      const bool isMousePressed = inputManager->IsMouseButtonPressed(0);

      UpdateButtonState(isInside, isMousePressed);

      if (isMousePressed && isInside)
      {
        float x = mouseX - sliderWidth  + (rectTransformCursor->GetSize().x / 2);

        value = ((x - sliderStartX) * (maxValue - minValue) / sliderWidth + minValue) - 0.47f;
        if (value < minValue) value = minValue;
        if (value > maxValue) value = maxValue;

        if (callback)
        {
          callback(value);
        }
        UpdateCursorPosition(x);
      }
    }

  private:

    bool           wasPressed_;
    ButtonCallback onClick_;
    State          currentState_;

    float minValue = 0.0f;
    float maxValue = 1.0f;
    float value = 0.5f;
    float sliderStartX;
    float sliderWidth;   
    ValueChangedCallback callback;
    bool initialized_ = false;
    DispositifD3D11 *pDispositifD3D11;
    EntityId cursorId;
    UIRendererComponent *uiRendererSlidebarComponent;
    UIRendererComponent *uiRendererCursorComponent;
    RectTransformComponent *rectTransformSlidebar;
    RectTransformComponent *rectTransformCursor;
    TransformComponent *transformSlidebar;
    TransformComponent *transformCursor;

    bool IsPointInside(float x, float y) const
    {
      if (!rectTransformSlidebar || !uiRendererSlidebarComponent || !transformSlidebar) return false;

      float contentWidth = uiRendererSlidebarComponent->GetContentWidth();
      float contentHeight = uiRendererSlidebarComponent->GetContentHeight();

      XMMATRIX uiMatrix = rectTransformSlidebar->GetUITransformMatrix(transformSlidebar,
        contentWidth,
        contentHeight);
      XMVECTOR tl = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
      XMVECTOR tr = XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f);
      XMVECTOR br = XMVectorSet(1.0f, 1.0f, 0.0f, 1.0f);
      XMVECTOR bl = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);

      tl = XMVector4Transform(tl, uiMatrix);
      tr = XMVector4Transform(tr, uiMatrix);
      br = XMVector4Transform(br, uiMatrix);
      bl = XMVector4Transform(bl, uiMatrix);

      XMFLOAT4 ftl, ftr, fbr, fbl;
      XMStoreFloat4(&ftl, tl);
      XMStoreFloat4(&ftr, tr);
      XMStoreFloat4(&fbr, br);
      XMStoreFloat4(&fbl, bl);

      float left = std::min({ ftl.x, ftr.x, fbr.x, fbl.x });
      float right = std::max({ ftl.x, ftr.x, fbr.x, fbl.x });
      float top = std::min({ ftl.y, ftr.y, fbr.y, fbl.y });
      float bottom = std::max({ ftl.y, ftr.y, fbr.y, fbl.y });

      return (x >= left && x <= right && y >= top && y <= bottom);
    }

    void UpdateButtonState(bool isInside, bool isMousePressed)
    {
      auto newState = State::Normal;

      if (!IsEnabled()) {
        newState = State::Disabled;
      }
      else if (isInside) {
        if (isMousePressed) {
          newState = State::Pressed;
          wasPressed_ = true;
        }
        else {
          newState = State::Hovered;
          if (wasPressed_) {
            wasPressed_ = false;
            if (onClick_) {
              onClick_();
            }
          }
        }
      }
      else {
        wasPressed_ = false;
      }

      if (currentState_ != newState) {
        currentState_ = newState;
      }
    }
  };
}

