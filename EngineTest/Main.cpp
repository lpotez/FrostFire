#pragma once
#include "stdafx.h"
#include "EngineWindows.h"
#include "FBXEntityBuilder.h"
#include "MenuScene.h"

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

using namespace FrostFireEngine;

int APIENTRY _tWinMain(_In_ HINSTANCE     hInstance,
                       _In_opt_ HINSTANCE hPrevInstance,
                       _In_ LPTSTR        lpCmdLine,
                       _In_ int           nShowCmd)
{
#ifdef _DEBUG
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  EngineWindows& engine = EngineWindows::GetInstance();

  try {
    EngineWindows::SetWindowsAppInstance(hInstance);
    engine.Initialize();

    SceneManager::GetInstance().SetActiveScene<MenuScene>(engine.GetDevice());

    engine.Run();
    engine.Cleanup();
    return 1;
  }
  catch (int errorCode) {
    wchar_t szErrMsg[MAX_LOADSTRING];
    LoadStringW(hInstance, errorCode, szErrMsg, MAX_LOADSTRING);
    MessageBoxW(nullptr, szErrMsg, L"Erreur", MB_ICONWARNING);

    engine.Cleanup();
    return 99;
  }
  /*catch (...)
  {
      MessageBoxW(nullptr, L"Erreur inconnue", L"Erreur", MB_ICONWARNING);
      engine.Cleanup();
      return 99;
  }*/
}
