#pragma once

#include <string>
#include <map>
#include <memory>
#include "ShaderVariant.h"

namespace FrostFireEngine
{
  struct CacheEntry {
    std::unique_ptr<ShaderVariant> variant;
    size_t                         lastAccessTime;
    size_t                         useCount;
  };

  class VariantCache {
  public:
    VariantCache(size_t maxSize = 100);
    ~VariantCache();

    ShaderVariant* GetOrCreateVariant(
      ID3D11Device*                             device,
      const std::wstring&                       shaderPath,
      const std::vector<std::string>&           featureIds,
      const std::map<std::string, std::string>& defines,
      const VertexLayoutDesc&                   layout);

    void Clear();
    void SetMaxSize(size_t size);

  private:
    void               TrimCache();
    static std::string GenerateVariantKey(
      const std::wstring&             shaderPath,
      const std::vector<std::string>& featureIds);

  private:
    std::map<std::string, CacheEntry> m_variantCache;
    size_t                            m_maxCacheSize;
    size_t                            m_currentTime;
  };
}
