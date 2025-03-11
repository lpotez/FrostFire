#include "StdAfx.h"
#include "resource.h"
#include "EngineWindows.h"

#if _DEBUG
#include <windows.h>
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#endif

namespace FrostFireEngine
{
  HINSTANCE EngineWindows::hAppInstance;

  void EngineWindows::SetWindowsAppInstance(HINSTANCE hInstance)
  {
    hAppInstance = hInstance;
  }

  bool EngineWindows::InitAppInstance()
  {
    TCHAR szTitle[MAX_LOADSTRING];

    LoadString(hAppInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hAppInstance, IDC_PETITMOTEUR3D, szWindowClass, MAX_LOADSTRING);

    if (!MyRegisterClass(hAppInstance)) {
      return false;
    }

    DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

    hMainWnd = CreateWindow(szWindowClass,
      szTitle,
      dwStyle,
      CW_USEDEFAULT,
      0,
      CW_USEDEFAULT,
      0,
      NULL,
      NULL,
      hAppInstance,
      NULL);

    if (!hMainWnd) {
      return false;
    }

    hAccelTable = LoadAccelerators(hAppInstance, MAKEINTRESOURCE(IDC_PETITMOTEUR3D));
    return true;
  }
  ATOM EngineWindows::MyRegisterClass(HINSTANCE hInstance) const
  {
    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = &WndProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PETITMOTEUR3D));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCE(IDC_PETITMOTEUR3D);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
  }

  int EngineWindows::Show()
  {
    ShowWindow(hMainWnd, SW_SHOWNORMAL);
    UpdateWindow(hMainWnd);
    return 0;
  }

  bool EngineWindows::RunSpecific()
  {
    MSG msg;
    while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) {
        return false;
      }

      if (!::TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
        TranslateMessage(&msg);
        ::DispatchMessage(&msg);
      }
    }
    return true;
  }

  #if _DEBUG
  void EngineWindows::InitialisationsImGui()
  {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    const ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hMainWnd);
    ImGui_ImplDX11_Init(pDevice->GetD3DDevice(), pDevice->GetImmediateContext());
  }
  #endif

  int EngineWindows::InitialisationsSpecific()
  {
    if (!InitAppInstance()) {
      return -1;
    }

    Show();
    InputManager::GetInstance().Init(hAppInstance, hMainWnd);
    return 0;
  }

  LRESULT CALLBACK EngineWindows::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
  {
#if _DEBUG
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam)) {
      return true;
    }
#endif

    switch (message) {
    case WM_KEYDOWN:
    {
      if (wParam == VK_F11) {
        // On veut basculer entre plein écran et fenêtré
        auto pDevice = EngineWindows::GetInstance().GetDevice();
        if (pDevice)
        {
          // pDevice est un pointeur vers DispositifD3D11
          DispositifD3D11 *pDisp = dynamic_cast<DispositifD3D11 *>(pDevice);
          if (pDisp)
          {
            bool currentFS = pDisp->IsFullScreen();
            pDisp->SetFullScreen(!currentFS, hWnd);
          }
        }
      }

      if (wParam == VK_ESCAPE) {
        if (!InputManager::GetInstance().IsInUIMode()) {
          InputManager::GetInstance().UnlockCursor();
        }
      }
    }
    break;

    case WM_LBUTTONDOWN:
      if (!InputManager::GetInstance().IsInUIMode()) {
        InputManager::GetInstance().LockCursor();
      }
      break;

    case WM_ACTIVATEAPP:
      if (wParam) {
        InputManager::GetInstance().HandleWindowActivation(true);
      }
      else {
        InputManager::GetInstance().HandleWindowActivation(false);
      }
      break;

    case WM_COMMAND:
    {
      int wmId = LOWORD(wParam);
      switch (wmId) {
      case IDM_ABOUT:
        DialogBox(hAppInstance, (LPCTSTR)IDD_ABOUTBOX, hWnd, About);
        break;
      case IDM_EXIT:
        DestroyWindow(hWnd);
        break;
      default:
        return DefWindowProc(hWnd, message, wParam, lParam);
      }
    }
    break;

    case WM_PAINT:
    {
      PAINTSTRUCT ps;
      HDC         hdc = BeginPaint(hWnd, &ps);
      EndPaint(hWnd, &ps);
    }
    break;

    case WM_DESTROY:
      PostQuitMessage(0);
      break;

    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
  }

  INT_PTR CALLBACK EngineWindows::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
  {
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
      case WM_INITDIALOG:
        return TRUE;

      case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
          EndDialog(hDlg, LOWORD(wParam));
          return TRUE;
        }
        break;
    }
    return FALSE;
  }

  DispositifD3D11* EngineWindows::CreationDispositifSpecific(const CDS_MODE cdsMode)
  {
    return new DispositifD3D11(cdsMode, hMainWnd);
  }

  void EngineWindows::BeginRenderSceneSpecific()
  {
    if (!pDevice) return;

    ID3D11DeviceContext*    pImmediateContext = pDevice->GetImmediateContext();
    ID3D11RenderTargetView* pRenderTargetView = pDevice->GetRenderTargetView();

    float Couleur[4] = {0.5290f, 0.808f, 0.922f, 1.0f};
    pImmediateContext->ClearRenderTargetView(pRenderTargetView, Couleur);

    ID3D11DepthStencilView* pDepthStencilView = pDevice->GetDepthStencilView();
    pImmediateContext->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
  }

  void EngineWindows::EndRenderSceneSpecific()
  {
  }
} // namespace FrostFireEngine
