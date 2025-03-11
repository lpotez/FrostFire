#pragma once

#include "Engine.h"
#include "Clock.h"
#include "dispositifD3D11.h"

namespace FrostFireEngine
{
#define MAX_LOADSTRING 100

  class EngineWindows final : public Engine<EngineWindows, DispositifD3D11> {
  public:
    static void SetWindowsAppInstance(HINSTANCE hInstance);

  private:
    bool InitAppInstance();
    ATOM MyRegisterClass(HINSTANCE hInstance) const;
    int  Show();

    int  InitialisationsSpecific() override;
    bool RunSpecific() override;

    #if _DEBUG
    void InitialisationsImGui() override;
    #endif

    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    static INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

    HACCEL           hAccelTable = nullptr;
    static HINSTANCE hAppInstance;
    HWND             hMainWnd = nullptr;
    TCHAR            szWindowClass[MAX_LOADSTRING] = TEXT("");

  protected:
    DispositifD3D11* CreationDispositifSpecific(CDS_MODE cdsMode) override;
    void             BeginRenderSceneSpecific() override;
    void             EndRenderSceneSpecific() override;
  };
} // namespace FrostFireEngine
