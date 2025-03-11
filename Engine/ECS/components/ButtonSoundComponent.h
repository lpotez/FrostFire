#pragma once
#include <functional>

#include "AudioSystem.h"
#include "AudioMixer.h"
#include "InputManager.h"
#include "TextureManager.h"
#include "Engine/ECS/core/Component.h"
#include "rendering/UIRendererComponent.h"
#include "transform/RectTransformComponent.h"


namespace FrostFireEngine
{
  class ButtonSoundComponent : public Component {
  public:
    using ButtonCallback = std::function<void()>;

    enum class State {
      Normal,
      Hovered,
      Pressed,
      Disabled
    };

    struct ButtonLogo {
      std::wstring soundOn = L"Assets/UI/sound.png";
      std::wstring soundOff = L"Assets/UI/soundDisabled.png";
    };

    struct ButtonColors {
      XMFLOAT4 normal = {1.0f, 1.0f, 1.0f, 1.0f};
      XMFLOAT4 hovered = {0.9f, 0.9f, 0.9f, 1.0f};
      XMFLOAT4 pressed = {0.7f, 0.7f, 0.7f, 1.0f};
      XMFLOAT4 disabled = {0.5f, 0.5f, 0.5f, 1.0f};
    };

    explicit ButtonSoundComponent(DispositifD3D11* pDispositifD3D11)
      : currentState_(State::Normal)
        , wasPressed_(false)
        , isSoundOn(true)
        , pDispositifD3D11(pDispositifD3D11)
        , uiRendererComponent(nullptr)
        , rectTransform_(nullptr)
        , transform(nullptr)
        , audioMixer(AudioMixer::GetInstance())
    {
    }

    void Update()
    {
      if (!initialized_) {
        Initialize();
        initialized_ = true;
      }

      // On vérifie ici le statut du component
      if (!IsEnabled() || !uiRendererComponent || !rectTransform_) {
        return;
      }

      const auto* inputManager = &InputManager::GetInstance();

      const float mouseX = static_cast<float>(inputManager->GetMouseX());
      const float mouseY = static_cast<float>(inputManager->GetMouseY());

      const bool isInside = IsPointInside(mouseX, mouseY);
      const bool isMousePressed = inputManager->IsMouseButtonPressed(0);

      UpdateButtonState(isInside, isMousePressed);
      UpdateVisuals();
    }

    void OnClick(const ButtonCallback& callback)
    {
      onClick_ = callback;
    }

  private:
    bool IsPointInside(float x, float y) const
    {
      if (!rectTransform_ || !uiRendererComponent || !transform) return false;

      float contentWidth = uiRendererComponent->GetContentWidth();
      float contentHeight = uiRendererComponent->GetContentHeight();

      XMMATRIX uiMatrix = rectTransform_->GetUITransformMatrix(transform,
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

      float left = std::min({ftl.x, ftr.x, fbr.x, fbl.x});
      float right = std::max({ftl.x, ftr.x, fbr.x, fbl.x});
      float top = std::min({ftl.y, ftr.y, fbr.y, fbl.y});
      float bottom = std::max({ftl.y, ftr.y, fbr.y, fbl.y});

      return (x >= left && x <= right && y >= top && y <= bottom);
    }

    void Initialize()
    {
      auto entity = World::GetInstance().GetEntity(owner);
      uiRendererComponent = entity->GetComponent<UIRendererComponent>();
      rectTransform_ = entity->GetComponent<RectTransformComponent>();
      transform = entity->GetComponent<TransformComponent>();
    }

    void UpdateButtonState(bool isInside, bool isMousePressed)
    {
      auto newState = State::Normal;
      bool newSoundState = isSoundOn;

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
              isSoundOn = !isSoundOn;
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
        UpdateTexture();
      }

      if (isSoundOn != newSoundState) {
        UpdateSoundState();
      }
    }

    void UpdateVisuals()
    {
      if (!uiRendererComponent) return;

      switch (currentState_) {
        case State::Normal:
          uiRendererComponent->SetColor(colors_.normal);
          break;
        case State::Hovered:
          uiRendererComponent->SetColor(colors_.hovered);
          break;
        case State::Pressed:
          uiRendererComponent->SetColor(colors_.pressed);
          break;
        case State::Disabled:
          uiRendererComponent->SetColor(colors_.disabled);
          break;
      }
    }

    void UpdateTexture()
    {
      if (!uiRendererComponent) return;

      TextureManager& textureManager = TextureManager::GetInstance();
      if (isSoundOn) {
        uiRendererComponent->SetTexture(
          textureManager.GetNewTexture(buttonLogo.soundOn, pDispositifD3D11));
      }
      else {
        uiRendererComponent->SetTexture(
          textureManager.GetNewTexture(buttonLogo.soundOff, pDispositifD3D11));
      }
    }

    void UpdateSoundState()
    {
      if (isSoundOn) {
        audioMixer.Unmute(); 
      }
      else {
        audioMixer.Mute();
      }
    }

    State      currentState_;
    ButtonLogo buttonLogo;

    bool           wasPressed_;
    bool           isSoundOn;
    bool           initialized_ = false;
    ButtonColors   colors_;
    ButtonCallback onClick_;

    DispositifD3D11*        pDispositifD3D11;
    UIRendererComponent*    uiRendererComponent;
    RectTransformComponent* rectTransform_;
    TransformComponent*     transform;
    AudioMixer&            audioMixer;
  };
}
