#include "InputManager.h"
#include "stdafx.h"

namespace FrostFireEngine
{
  bool InputManager::hasBeenInitialized = false;

  bool InputManager::Init(HINSTANCE hInstance, HWND hWnd)
  {
    this->hWnd = hWnd;

    // Un seul objet DirectInput est permis
    if (!hasBeenInitialized) {
      // Objet DirectInput
      HRESULT hr = DirectInput8Create(hInstance,
                                      DIRECTINPUT_VERSION,
                                      IID_IDirectInput8,
                                      (void**)&pDirectInput,
                                      nullptr);

      if (FAILED(hr)) {
        return false;
      }

      // Objet Clavier
      hr = pDirectInput->CreateDevice(GUID_SysKeyboard,
                                      &pKeyboard,
                                      nullptr);

      if (FAILED(hr)) {
        return false;
      }

      hr = pKeyboard->SetDataFormat(&c_dfDIKeyboard);
      if (FAILED(hr)) {
        return false;
      }

      hr = pKeyboard->SetCooperativeLevel(hWnd,
                                          DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
      if (FAILED(hr)) {
        return false;
      }

      // Try to acquire the keyboard
      hr = pKeyboard->Acquire();
      if (FAILED(hr)) {
        return false;
      }

      // Initialize tamponClavier
      ZeroMemory(keyboardBuffer, sizeof(keyboardBuffer));

      // Objet Souris
      hr = pDirectInput->CreateDevice(GUID_SysMouse,
                                      &pMouse,
                                      nullptr);

      if (FAILED(hr)) {
        return false;
      }

      hr = pMouse->SetDataFormat(&c_dfDIMouse);
      if (FAILED(hr)) {
        return false;
      }

      hr = pMouse->SetCooperativeLevel(hWnd,
                                       DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
      if (FAILED(hr)) {
        return false;
      }

      // Try to acquire the mouse
      hr = pMouse->Acquire();
      if (FAILED(hr)) {
        return false;
      }

      // Initialize mouseState
      ZeroMemory(&mouseState, sizeof(mouseState));

      hasBeenInitialized = true;
      return true;
    }

    return true;
  }

  void InputManager::Init()
  {
    pDirectInput = nullptr;
    pKeyboard = nullptr;
    pMouse = nullptr;
    pJoystick = nullptr;
    hWnd = nullptr;
    isMouseLocked = false;
    ZeroMemory(keyboardBuffer, sizeof(keyboardBuffer));
    ZeroMemory(&mouseState, sizeof(mouseState));
    mouseX = 0;
    mouseY = 0;
  }

  void InputManager::Release()
  {
    UnlockCursor();

    if (pKeyboard) {
      pKeyboard->Unacquire();
      pKeyboard->Release();
      pKeyboard = nullptr;
    }
    if (pMouse) {
      pMouse->Unacquire();
      pMouse->Release();
      pMouse = nullptr;
    }
    if (pJoystick) {
      pJoystick->Unacquire();
      pJoystick->Release();
      pJoystick = nullptr;
    }
    if (pDirectInput) {
      pDirectInput->Release();
      pDirectInput = nullptr;
    }

    hasBeenInitialized = false;
  }

  void InputManager::UpdateKeyboard()
  {
    if (pKeyboard) {
      HRESULT hr = pKeyboard->GetDeviceState(sizeof(keyboardBuffer), &keyboardBuffer);

      if (FAILED(hr)) {
        // Input lost, try to reacquire
        if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
          hr = pKeyboard->Acquire();
          if (SUCCEEDED(hr)) {
            hr = pKeyboard->GetDeviceState(sizeof(keyboardBuffer), &keyboardBuffer);
          }
        }

        // If we still failed, zero the buffer
        if (FAILED(hr)) {
          ZeroMemory(keyboardBuffer, sizeof(keyboardBuffer));
        }
      }
    }
  }

  bool InputManager::IsKeyPressed(UINT touche) const
  {
    if (touche >= sizeof(keyboardBuffer)) return false;

    return (keyboardBuffer[touche] & 0x80) != 0;
  }


  void InputManager::LockCursor()
  {
    if (!isMouseLocked && hWnd && pMouse) {
      // Changer en mode exclusif
      pMouse->Unacquire();
      HRESULT hr = pMouse->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
      if (SUCCEEDED(hr)) {
        hr = pMouse->Acquire();
        if (SUCCEEDED(hr)) {
          isMouseLocked = true;

          // Obtenir les dimensions de la fenêtre client
          GetClientRect(hWnd, &windowRect);

          // Convertir les coordonnées client en coordonnées écran
          POINT topLeft = {windowRect.left, windowRect.top};
          POINT bottomRight = {windowRect.right, windowRect.bottom};
          ClientToScreen(hWnd, &topLeft);
          ClientToScreen(hWnd, &bottomRight);

          windowRect.left = topLeft.x;
          windowRect.top = topLeft.y;
          windowRect.right = bottomRight.x;
          windowRect.bottom = bottomRight.y;

          ClipCursor();
          ShowCursor(FALSE);
        }
      }
    }
  }

  void InputManager::UnlockCursor()
  {
    if (isMouseLocked && pMouse) {
      // Revenir en mode non-exclusif
      pMouse->Unacquire();
      HRESULT hr = pMouse->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
      if (SUCCEEDED(hr)) {
        pMouse->Acquire();
      }

      isMouseLocked = false;
      ShowCursor(TRUE);
      ClipCursor();

      mouseState.lX = 0;
      mouseState.lY = 0;

      mouseX = 0;
      mouseY = 0;
    }
  }

  void InputManager::ClipCursor() const
  {
    if (isMouseLocked) {
      ::ClipCursor(&windowRect);
    }
    else {
      ::ClipCursor(nullptr);
    }
  }

  void InputManager::HandleWindowActivation(bool activated)
  {
    if (!activated) {
      // Sauvegarder l'état du verrouillage avant de le perdre
      wasLockedBeforeLostFocus = isMouseLocked;
      UnlockCursor();
    }
    else {
      // Réacquérir les périphériques et restaurer l'état du verrouillage si nécessaire
      if (ReacquireDevices() && wasLockedBeforeLostFocus) {
        LockCursor();
      }
    }
  }
  bool InputManager::IsMouseButtonPressed(const int button) const
  {
    if (button < 0 || button >= 4) return false;
    return (mouseState.rgbButtons[button] & 0x80) != 0;
  }
  void InputManager::SetUIMode(bool enabled)
  {
    isInUIMode = enabled;
    if (enabled) {
      UnlockCursor();
      ShowCursor(TRUE);
    }
    else {
      LockCursor();
      ShowCursor(FALSE);
    }
  }

  bool InputManager::ReacquireDevices()
  {
    bool success = true;

    if (pKeyboard) {
      HRESULT hr = pKeyboard->Acquire();
      if (FAILED(hr)) {
        success = false;
      }
    }

    if (pMouse) {
      HRESULT hr = pMouse->Acquire();
      if (FAILED(hr)) {
        success = false;
      }
    }

    // Réinitialiser les états
    ZeroMemory(keyboardBuffer, sizeof(keyboardBuffer));
    ZeroMemory(&mouseState, sizeof(mouseState));

    return success;
  }
  void InputManager::UpdateMouse()
  {
    if (pMouse) {
      HRESULT hr = pMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState);

      if (FAILED(hr)) {
        if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
          hr = pMouse->Acquire();
          if (SUCCEEDED(hr)) {
            hr = pMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState);
          }
        }

        if (FAILED(hr)) {
          ZeroMemory(&mouseState, sizeof(mouseState));
        }
      }

      // Mise à jour de la position absolue
      if (!isMouseLocked && isInUIMode) {
        POINT cursorPos;
        if (GetCursorPos(&cursorPos)) {
          // En mode plein écran, on utilise les coordonnées d'écran directement
          RECT clientRect;
          GetClientRect(hWnd, &clientRect);
          POINT topLeft = { clientRect.left, clientRect.top };
          ClientToScreen(hWnd, &topLeft);

          mouseX = cursorPos.x - topLeft.x;
          mouseY = cursorPos.y - topLeft.y;
        }
      }
    }
  }
} // namespace FrostFireEngine
