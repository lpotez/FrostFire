#pragma once
#include "TransformComponent.h"
#include <DirectXMath.h>
#include <unordered_set>
#include "Engine/ECS/core/World.h"
#include "Engine/ECS/core/Entity.h"
#include "Engine/DispositifD3D11.h"
#include "Engine/ECS/core/Component.h"

namespace FrostFireEngine
{
  using namespace DirectX;

  enum class RectAnchorPreset {
    TopLeft,
    TopCenter,
    TopRight,
    MiddleLeft,
    MiddleCenter,
    MiddleRight,
    BottomLeft,
    BottomCenter,
    BottomRight
  };

  enum class RectSizingMode {
    None,
    FitWidth,
    FitHeight,
    FitBoth,
    Stretch
  };

  class RectTransformComponent : public Component {
  public:
    RectTransformComponent(DispositifD3D11* dispositif)
      : dispositif_(dispositif),
        anchor_(RectAnchorPreset::MiddleCenter),
        sizingMode_(RectSizingMode::None),
        anchorOffset_(0.0f, 0.0f),
        scale_(1.0f, 1.0f),
        size_(0.0f, 0.0f),
        pivotPoint_(0.f, 0.f)
    {
    }

    void SetAnchor(RectAnchorPreset anchor)
    {
      anchor_ = anchor;
    }
    void SetSizingMode(RectSizingMode mode)
    {
      sizingMode_ = mode;
    }
    void SetAnchorOffset(const XMFLOAT2& offset)
    {
      anchorOffset_ = offset;
    }
    void SetScale(const XMFLOAT2& scale)
    {
      scale_ = scale;
    }
    void SetSize(const XMFLOAT2& size)
    {
      size_ = size;
    }
    void SetPivotPoint(const XMFLOAT2& pivot)
    {
      pivotPoint_ = pivot;
    }

    RectAnchorPreset GetAnchor() const
    {
      return anchor_;
    }
    RectSizingMode GetSizingMode() const
    {
      return sizingMode_;
    }
    XMFLOAT2 GetAnchorOffset() const
    {
      return anchorOffset_;
    }
    XMFLOAT2 GetScale() const
    {
      return scale_;
    }
    XMFLOAT2 GetSize() const
    {
      return size_;
    }
    XMFLOAT2 GetPivotPoint() const
    {
      return pivotPoint_;
    }

    XMMATRIX GetUITransformMatrix(TransformComponent* transform,
                                  float               contentWidth,
                                  float               contentHeight,
                                  bool                first = true) const
    {
      const RectTransformComponent* parentRect = GetParentRectTransform();
      float                         parentWidth = 0.0f;
      float                         parentHeight = 0.0f;
      if (parentRect) {
        parentRect->GetFinalSizeForChild(parentWidth, parentHeight);
      }
      else {
        parentWidth = dispositif_->GetViewportWidth();
        parentHeight = dispositif_->GetViewportHeight();
      }

      float finalWidth = 0.0f;
      float finalHeight = 0.0f;
      ComputeFinalSize(contentWidth, contentHeight, finalWidth, finalHeight);

      XMFLOAT2 anchorPoint = ComputeAnchorPoint(parentWidth, parentHeight);
      float    posX = anchorPoint.x + anchorOffset_.x;
      float    posY = anchorPoint.y + anchorOffset_.y;
      float    pivotOffsetX = pivotPoint_.x * finalWidth;
      float    pivotOffsetY = pivotPoint_.y * finalHeight;
      /*posX -= pivotOffsetX;
      posY -= pivotOffsetY;*/

      if (parentRect) {
        XMMATRIX pivotTrans = XMMatrixTranslation(-pivotOffsetX, -pivotOffsetY, 0.0f);
        XMMATRIX rot = XMMatrixRotationZ(transform->GetRotationEuler().z);
        XMMATRIX localMatrix = first
                                 ? XMMatrixScaling(scale_.x * finalWidth, scale_.y * finalHeight,
                                                   1.0f) * pivotTrans
                                 * rot *  XMMatrixTranslation(posX + pivotOffsetX, posY + pivotOffsetY, 0.0f)
                                 : XMMatrixScaling(scale_.x, scale_.y, 1.0f) * pivotTrans
                                 * rot * XMMatrixTranslation(posX + pivotOffsetX, posY + pivotOffsetY, 0.0f);

       
        auto parentTransform = GetParentTransform();

        XMMATRIX parentMatrix = parentRect->GetUITransformMatrix(
          parentTransform, parentWidth, parentHeight, false);
        return localMatrix * parentMatrix;
      }

      else {
        XMMATRIX rot = XMMatrixRotationZ(transform->GetRotationEuler().z);
        XMMATRIX uiMatrix = first
                              ? XMMatrixScaling(scale_.x * finalWidth, scale_.y * finalHeight, 1.0f)
                              * rot * XMMatrixTranslation(posX, posY, 0.0f)
                              : XMMatrixScaling(scale_.x, scale_.y, 1.0f)
                              * rot * XMMatrixTranslation(posX, posY, 0.0f);

      
        

        return  uiMatrix;
      }
    }

  private:
    DispositifD3D11* dispositif_;
    RectAnchorPreset anchor_;
    RectSizingMode   sizingMode_;
    XMFLOAT2         anchorOffset_;
    XMFLOAT2         scale_;
    XMFLOAT2         size_;
    XMFLOAT2         pivotPoint_;

    const RectTransformComponent* GetParentRectTransform() const
    {
      auto entity = World::GetInstance().GetEntity(owner);
      if (!entity) return nullptr;
      auto transform = entity->GetComponent<TransformComponent>();
      if (!transform) return nullptr;
      EntityId p = transform->GetParent();
      if (p == INVALID_ENTITY_ID) return nullptr;
      auto parentEntity = World::GetInstance().GetEntity(p);
      if (!parentEntity) return nullptr;
      return parentEntity->GetComponent<RectTransformComponent>();
    }

     TransformComponent *GetParentTransform() const {
      auto entity = World::GetInstance().GetEntity(owner);
      if (!entity) return nullptr;
      auto transform = entity->GetComponent<TransformComponent>();
      if (!transform) return nullptr;
      EntityId p = transform->GetParent();
      if (p == INVALID_ENTITY_ID) return nullptr;
      auto parentEntity = World::GetInstance().GetEntity(p);
      if (!parentEntity) return nullptr;
      return parentEntity->GetComponent<TransformComponent>();
    }

    void ComputeFinalSize(float  contentWidth,
                          float  contentHeight,
                          float& outWidth,
                          float& outHeight) const
    {
      float parentWidth = 0.0f;
      float parentHeight = 0.0f;
      if (auto parentRect = GetParentRectTransform()) {
        parentRect->GetFinalSizeForChild(parentWidth, parentHeight);
      }
      else {
        parentWidth = dispositif_->GetViewportWidth();
        parentHeight = dispositif_->GetViewportHeight();
      }

      switch (sizingMode_) {
        case RectSizingMode::Stretch:
          outWidth = parentWidth;
          outHeight = parentHeight;
          break;
        case RectSizingMode::FitWidth:
          if (contentWidth > 0.0f) {
            outWidth = parentWidth;
            float scaleFactor = parentWidth / contentWidth;
            outHeight = contentHeight * scaleFactor;
          }
          else {
            outWidth = (size_.x > 0.0f) ? size_.x : parentWidth;
            outHeight = (size_.y > 0.0f) ? size_.y : contentHeight;
          }
          break;
        case RectSizingMode::FitHeight:
          if (contentHeight > 0.0f) {
            outHeight = parentHeight;
            float scaleFactor = parentHeight / contentHeight;
            outWidth = contentWidth * scaleFactor;
          }
          else {
            outWidth = (size_.x > 0.0f) ? size_.x : contentWidth;
            outHeight = (size_.y > 0.0f) ? size_.y : parentHeight;
          }
          break;
        case RectSizingMode::FitBoth:
          if (contentWidth > 0.0f && contentHeight > 0.0f) {
            float scaleFactor = std::min(parentWidth / contentWidth, parentHeight / contentHeight);
            outWidth = contentWidth * scaleFactor;
            outHeight = contentHeight * scaleFactor;
          }
          else {
            outWidth = (size_.x > 0.0f) ? size_.x : parentWidth;
            outHeight = (size_.y > 0.0f) ? size_.y : parentHeight;
          }
          break;
        case RectSizingMode::None:
        default:
          outWidth = (size_.x > 0.0f) ? size_.x : contentWidth;
          outHeight = (size_.y > 0.0f) ? size_.y : contentHeight;
          break;
      }
    }


    XMFLOAT2 ComputeAnchorPoint(float parentWidth, float parentHeight) const
    {
      float anchorX = 0.0f;
      float anchorY = 0.0f;
      float delta_x = size_.x * scale_.x;
      float delta_y = size_.y * scale_.y;
      switch (anchor_) {
        case RectAnchorPreset::TopLeft:
          anchorX = 0.0f;
          anchorY = 0.0f;
          break;
        case RectAnchorPreset::TopCenter:
          anchorX = parentWidth * 0.5f - delta_x* 0.5;
          anchorY = 0.0f;
          break;
        case RectAnchorPreset::TopRight:
          anchorX = parentWidth - delta_x;
          anchorY = 0.0f;
          break;
        case RectAnchorPreset::MiddleLeft:
          anchorX = 0.0f;
          anchorY = parentHeight * 0.5f - delta_y * 0.5;
          break;
        case RectAnchorPreset::MiddleCenter:
          anchorX = parentWidth * 0.5f - delta_x * 0.5;
          anchorY = parentHeight * 0.5f - delta_y * 0.5;
          break;
        case RectAnchorPreset::MiddleRight:
          anchorX = parentWidth  - delta_x;
          anchorY = parentHeight * 0.5f - delta_y * 0.5;
          break;
        case RectAnchorPreset::BottomLeft:
          anchorX = 0.0f;
          anchorY = parentHeight - delta_y;
          break;
        case RectAnchorPreset::BottomCenter:
          anchorX = parentWidth * 0.5f - delta_x * 0.5;
          anchorY = parentHeight - delta_y;
          break;
        case RectAnchorPreset::BottomRight:
          anchorX = parentWidth  - delta_x;
          anchorY = parentHeight  - delta_y;
          break;
      }
      return XMFLOAT2(anchorX, anchorY);
    }

    void GetFinalSizeForChild(float& outWidth, float& outHeight) const
    {
      float contentWidth = (size_.x > 0.0f) ? size_.x : 100.0f;
      float contentHeight = (size_.y > 0.0f) ? size_.y : 100.0f;
      ComputeFinalSize(contentWidth, contentHeight, outWidth, outHeight);
    }
  };
}
