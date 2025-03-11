#include "VariantCache.h"
#include <algorithm>
#include <sstream>

namespace FrostFireEngine
{
  VariantCache::VariantCache(const size_t maxSize) :
    m_maxCacheSize(maxSize),
    m_currentTime(0)
  {
  }

  VariantCache::~VariantCache()
  {
    Clear();
  }

  ShaderVariant* VariantCache::GetOrCreateVariant(
    ID3D11Device*                             device,
    const std::wstring&                       shaderPath,
    const std::vector<std::string>&           featureIds,
    const std::map<std::string, std::string>& defines,
    const VertexLayoutDesc&                   layout)
  {
    const std::string key = GenerateVariantKey(shaderPath, featureIds);

    if (const auto it = m_variantCache.find(key); it != m_variantCache.end()) {
      it->second.lastAccessTime = m_currentTime++;
      it->second.useCount++;
      return it->second.variant.get();
    }

    if (m_variantCache.size() >= m_maxCacheSize) {
      TrimCache();
    }

    auto newVariant = std::make_unique<ShaderVariant>();
    if (!newVariant->Initialize(device, shaderPath, featureIds, defines, layout)) {
      return nullptr;
    }

    CacheEntry entry;
    entry.variant = std::move(newVariant);
    entry.lastAccessTime = m_currentTime++;
    entry.useCount = 1;

    const auto insertedVariant = entry.variant.get();
    m_variantCache[key] = std::move(entry);

    return insertedVariant;
  }

  void VariantCache::Clear()
  {
    m_variantCache.clear();
    m_currentTime = 0;
  }

  void VariantCache::SetMaxSize(const size_t size)
  {
    m_maxCacheSize = size;
    if (m_variantCache.size() > m_maxCacheSize) {
      TrimCache();
    }
  }

  void VariantCache::TrimCache()
  {
    if (m_variantCache.empty()) {
      return;
    }

    std::vector<std::pair<std::string, float>> scores;
    scores.reserve(m_variantCache.size());

    for (const auto& entry : m_variantCache) {
      float score = static_cast<float>(entry.second.useCount) /
      (m_currentTime - entry.second.lastAccessTime + 1);
      scores.emplace_back(entry.first, score);
    }

    std::ranges::sort(scores, [](const auto& a, const auto& b)
    {
      return a.second < b.second;
    });

    const size_t numToRemove = m_variantCache.size() - m_maxCacheSize;
    for (size_t i = 0; i < numToRemove; ++i) {
      m_variantCache.erase(scores[i].first);
    }
  }

  std::string VariantCache::GenerateVariantKey(
    const std::wstring&             shaderPath,
    const std::vector<std::string>& featureIds)
  {
    std::stringstream keyStream;
    keyStream << std::string(shaderPath.begin(), shaderPath.end()) << "|";

    std::vector<std::string> sortedIds = featureIds;
    std::ranges::sort(sortedIds);

    for (const auto& id : sortedIds) {
      keyStream << id << ";";
    }

    return keyStream.str();
  }
}
