#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <map>
#include <d3d11.h>
#include <wrl/client.h>

#include "RenderShader.h"
#include "Engine/Singleton.h"
#include "Engine/Shaders/ShaderVariant.h"
#include "features/RenderPass.h"

namespace FrostFireEngine
{
  class ShaderManager final : public CSingleton<ShaderManager> {
    friend class CSingleton;

    std::unordered_map<std::string, std::unique_ptr<RenderShader>> shaders;
    ID3D11Device*                                                  m_device;

  public:
    static const std::wstring fullPath;

    RenderShader* GetOrCreateShader(const std::string& name);
    bool          Initialize(ID3D11Device* device);
    void          Shutdown();

    // On va stocker les variants selon technique, pass, et un key string
    struct VariantKey {
      std::string technique;
      RenderPass  pass;
      std::string variantString;
      bool        operator<(const VariantKey& other) const
      {
        if (technique < other.technique) return true;
        if (technique > other.technique) return false;
        if (static_cast<int>(pass) < static_cast<int>(other.pass)) return true;
        if (static_cast<int>(pass) > static_cast<int>(other.pass)) return false;
        return variantString < other.variantString;
      }
    };

    ShaderVariant* GetOrCreatePassVariant(const std::string&                        techniqueName,
                                          RenderPass                                pass,
                                          const std::wstring&                       shaderPath,
                                          const std::string&                        vsEntry,
                                          const std::string&                        psEntry,
                                          const std::map<std::string, std::string>& defines,
                                          const VertexLayoutDesc&                   layout);

    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;

    ShaderManager(ShaderManager&& other) noexcept = delete;
    ShaderManager& operator=(ShaderManager&& other) noexcept = delete;

  protected:
    ShaderManager() : m_device(nullptr)
    {
    }
    ~ShaderManager() override
    {
      Shutdown();
    }

  private:
    std::map<VariantKey, std::unique_ptr<ShaderVariant>> m_variants;
  };
}
