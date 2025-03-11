#pragma once

#include <d3d11.h>
#include <string>
#include <vector>
#include "FeatureMetadata.h"

class BaseShaderFeature {
public:
  virtual                          ~BaseShaderFeature() = default;
  virtual std::vector<std::string> GetDefines() const = 0;
  virtual void                     UpdateParameters(ID3D11DeviceContext* context) = 0;
  virtual std::string              GetId() const = 0;
  virtual FeatureMetadata          GetMetadata() const = 0;
  virtual uint32_t                 GetFeatureGroup() const = 0;
};
