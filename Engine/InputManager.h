#pragma once
#include "Singleton.h"
#include "stdafx.h"

namespace FrostFireEngine
{
  class InputManager : public CSingleton<InputManager>
  {
    friend class CSingleton<InputManager>;

  public:
    bool Init(HINSTANCE hInstance, HWND hwnd);
    void Init();
    void Release();
    bool IsKeyPressed(UINT touche) const;
    void UpdateKeyboard();
    void UpdateMouse();
    void LockCursor();
    void UnlockCursor();
    void ClipCursor() const;

    // Position absolue
    LONG GetMouseX() const { return mouseX; }
    LONG GetMouseY() const { return mouseY; }

    // Delta (mouvement relatif)
    LONG GetMouseDX() const { return mouseState.lX; }
    LONG GetMouseDY() const { return mouseState.lY; }

    // Delta de la molette
    LONG GetMouseDZ() const { return mouseState.lZ; }

    bool HasMouseMoved() const { return mouseState.lX || mouseState.lY; }
    bool IsCursorLocked() const { return isMouseLocked; }

    void HandleWindowActivation(bool activated);
    bool IsMouseLocked() const { return isMouseLocked; }

    bool IsMouseButtonPressed(int button) const;

    bool IsInUIMode() const { return isInUIMode; }
    void SetUIMode(bool enabled);

  private:
    IDirectInput8* pDirectInput;
    IDirectInputDevice8* pKeyboard;
    IDirectInputDevice8* pMouse;
    IDirectInputDevice8* pJoystick;
    static bool hasBeenInitialized;
    char keyboardBuffer[256];
    DIMOUSESTATE mouseState;
    bool isMouseLocked{false};
    RECT windowRect{};
    HWND hWnd{nullptr};

    // Position absolue de la souris
    LONG mouseX{0};
    LONG mouseY{0};

    bool wasLockedBeforeLostFocus = false;
    bool ReacquireDevices();

    bool isInUIMode = false;

    InputManager() : pDirectInput(nullptr), pKeyboard(nullptr), pMouse(nullptr),
                     pJoystick(nullptr), keyboardBuffer{}, mouseState{}
    {
    }
  };
}
