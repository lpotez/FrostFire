#pragma once

#include <vector>
#include <string>
#include <cstdint>

struct FeatureMetadata {
  std::vector<std::string> requiredFeatures; // Features requises pour que cette feature fonctionne
  std::vector<std::string> incompatibleFeatures;  // Features qui ne peuvent pas être utilisées avec celle-ci
  uint32_t renderPassMask; // Masque indiquant les passes de rendu où cette feature est active

  FeatureMetadata() : renderPassMask(0xFFFFFFFF) // Par défaut, active dans toutes les passes
  {
  }
};
