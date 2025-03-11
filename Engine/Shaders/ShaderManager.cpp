#include "ShaderManager.h"
#include "Engine/Utils/ErrorLogger.h"
#include <algorithm>
#include <filesystem>
#include <sstream>

namespace FrostFireEngine
{
  const std::wstring ShaderManager::fullPath = L"Assets/Shaders";

  RenderShader* ShaderManager::GetOrCreateShader(const std::string& name)
  {
    if (const auto it = shaders.find(name); it != shaders.end()) {
      return it->second.get();
    }

    std::filesystem::path shaderPath = fullPath;
    shaderPath /= std::wstring(name.begin(), name.end()) + L".fx";

    auto shader = std::make_unique<RenderShader>(m_device);
    if (!shader->Initialize(shaderPath.wstring())) {
      ErrorLogger::Log("Failed to initialize shader: " + name);
      return nullptr;
    }

    RenderShader* shaderPtr = shader.get();
    shaders.emplace(name, std::move(shader));
    return shaderPtr;
  }

  bool ShaderManager::Initialize(ID3D11Device* device)
  {
    if (!device) {
      ErrorLogger::Log("Invalid device pointer in ShaderManager::Initialize");
      return false;
    }

    m_device = device;
    return true;
  }

  void ShaderManager::Shutdown()
  {
    m_device = nullptr;
    shaders.clear();
    m_variants.clear();
  }

  ShaderVariant* ShaderManager::GetOrCreatePassVariant(const std::string&  techniqueName,
                                                       RenderPass          pass,
                                                       const std::wstring& shaderPath,
                                                       const std::string&  vsEntry,
                                                       const std::string&  psEntry,
                                                       const std::map<std::string, std::string>&
                                                       defines,
                                                       const VertexLayoutDesc& layout)
  {
    // Crée un string unique pour les defines
    std::stringstream ss;
    for (auto& d : defines) {
      ss << d.first << "=" << d.second << ";";
    }
    VariantKey key{techniqueName, pass, ss.str()};

    auto it = m_variants.find(key);
    if (it != m_variants.end()) {
      return it->second.get();
    }

    auto                     variant = std::make_unique<ShaderVariant>();
    std::vector<std::string> featureIds;

    for (auto& d : defines) {
      featureIds.push_back(d.first);
    }

    if (!variant->Initialize(m_device, shaderPath, featureIds, defines, layout, vsEntry, psEntry)) {
      ErrorLogger::Log("Failed to initialize shader variant for pass.");
      return nullptr;
    }

    auto ptr = variant.get();
    m_variants[key] = std::move(variant);
    return ptr;
  }
}
