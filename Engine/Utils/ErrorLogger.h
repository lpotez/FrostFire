#pragma once

#include <string>
#include <Windows.h>
#include <comdef.h>
#include <fstream>

class ErrorLogger {
public:
  static void Log(const std::string& message)
  {
    const std::string errorMessage = "Error: " + message;
    MessageBoxA(nullptr, errorMessage.c_str(), "Error", MB_ICONERROR);
    throw std::runtime_error(errorMessage);
  }

  static void Log(const HRESULT hr, const std::string& message)
  {
    const _com_error   error(hr);
    const std::wstring errorMessage = L"Error: " + std::wstring(error.ErrorMessage()) + L"\n" +
    std::wstring(message.begin(), message.end());
    MessageBoxW(nullptr, errorMessage.c_str(), L"Error", MB_ICONERROR);
  }
};
