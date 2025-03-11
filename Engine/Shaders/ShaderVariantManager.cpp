#include "ShaderVariantManager.h"
#include <algorithm>

namespace FrostFireEngine
{
  ShaderVariantManager::ShaderVariantManager() = default;

  ShaderVariantManager::~ShaderVariantManager() = default;

  bool ShaderVariantManager::RegisterFeature(const std::string&     featureId,
                                             const FeatureMetadata& metadata)
  {
    if (m_featureMetadata.contains(featureId)) {
      return false;
    }
    m_featureMetadata[featureId] = metadata;
    return true;
  }

  bool ShaderVariantManager::ValidateFeatureCombination(
    const std::vector<std::string>& featureIds) const
  {
    return std::ranges::all_of(featureIds, [this, &featureIds](const std::string& featureId)
    {
      if (!m_featureMetadata.contains(featureId)) {
        return false;
      }

      if (!ValidateDependencies(featureId, featureIds)) {
        return false;
      }

      if (!ValidateIncompatibilities(featureId, featureIds)) {
        return false;
      }

      return true;
    });
  }

  bool ShaderVariantManager::GetFeatureMetadata(const std::string& featureId,
                                                FeatureMetadata&   outMetadata) const
  {
    auto it = m_featureMetadata.find(featureId);
    if (it == m_featureMetadata.end()) {
      return false;
    }
    outMetadata = it->second;
    return true;
  }

  bool ShaderVariantManager::AreCompatible(const std::string& feature1,
                                           const std::string& feature2) const
  {
    auto it1 = m_featureMetadata.find(feature1);
    auto it2 = m_featureMetadata.find(feature2);

    if (it1 == m_featureMetadata.end() || it2 == m_featureMetadata.end()) {
      return false;
    }

    const auto& metadata1 = it1->second;
    const auto& metadata2 = it2->second;

    auto isIncompatible = [](const FeatureMetadata& meta, const std::string& feature)
    {
      return std::find(meta.incompatibleFeatures.begin(),
                       meta.incompatibleFeatures.end(),
                       feature) != meta.incompatibleFeatures.end();
    };

    return !isIncompatible(metadata1, feature2) && !isIncompatible(metadata2, feature1);
  }

  bool ShaderVariantManager::ValidateDependencies(const std::string&              featureId,
                                                  const std::vector<std::string>& activeFeatures)
  const
  {
    auto it = m_featureMetadata.find(featureId);
    if (it == m_featureMetadata.end()) {
      return false;
    }

    const auto& metadata = it->second;
    for (const auto& requiredFeature : metadata.requiredFeatures) {
      if (std::find(activeFeatures.begin(),
                    activeFeatures.end(),
                    requiredFeature) == activeFeatures.end()) {
        return false;
      }
    }
    return true;
  }

  bool ShaderVariantManager::ValidateIncompatibilities(const std::string& featureId,
                                                       const std::vector<std::string>&
                                                       activeFeatures)
  const
  {
    auto it = m_featureMetadata.find(featureId);
    if (it == m_featureMetadata.end()) {
      return false;
    }

    const auto& metadata = it->second;
    for (const auto& incompatibleFeature : metadata.incompatibleFeatures) {
      if (std::find(activeFeatures.begin(),
                    activeFeatures.end(),
                    incompatibleFeature) != activeFeatures.end()) {
        return false;
      }
    }
    return true;
  }
}
