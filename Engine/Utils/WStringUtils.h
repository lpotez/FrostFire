#pragma once
#include <string>

#include "Engine/ECS/components/rendering/BaseRendererComponent.h"

inline std::string ConvertWStringToString(const std::wstring& wstr)
{
  if (wstr.empty()) return {};

  int size_needed =
  WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
  std::string str(size_needed, 0);
  WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str[0], size_needed, NULL, NULL);
  return str;
}
