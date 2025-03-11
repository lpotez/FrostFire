#pragma once

#include <Windows.h>
#include <cstdint>
#include <d3d11.h>

namespace FrostFireEngine
{
  enum CDS_MODE {
    CDS_FENETRE,
    CDS_PLEIN_ECRAN
  };

  class CDispositif {
  public:
    virtual      ~CDispositif() = default;
    virtual void PresentSpecific() = 0;

    virtual void Present()
    {
      PresentSpecific();
    };

  protected:
    uint32_t screenWidth;
    uint32_t screenHeight;

    ID3D11RasterizerState* mSolidCullBackRS;
  };
}
