#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "features/FeatureMetadata.h"

namespace FrostFireEngine
{
  class ShaderVariantManager {
  public:
    ShaderVariantManager();
    ~ShaderVariantManager();

    bool RegisterFeature(const std::string& featureId, const FeatureMetadata& metadata);
    bool ValidateFeatureCombination(const std::vector<std::string>& featureIds) const;
    bool GetFeatureMetadata(const std::string& featureId, FeatureMetadata& outMetadata) const;
    bool AreCompatible(const std::string& feature1, const std::string& feature2) const;

  private:
    bool ValidateDependencies(const std::string&              featureId,
                              const std::vector<std::string>& activeFeatures) const;
    bool ValidateIncompatibilities(const std::string&              featureId,
                                   const std::vector<std::string>& activeFeatures) const;

  private:
    std::unordered_map<std::string, FeatureMetadata> m_featureMetadata;
  };
}
