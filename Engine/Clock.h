#pragma once
#include <cstdint>
#include <windows.h>

namespace FrostFireEngine
{
  class Clock
  {
  public:
    Clock();
    ~Clock() = default;

    // Empêcher la copie pour éviter les problèmes de synchronisation
    Clock(const Clock&) = delete;
    Clock& operator=(const Clock&) = delete;

    int64_t GetTimeCount();
    double GetSecPerCount() const { return m_SecPerCount; }
    double GetTimeBetweenCounts(int64_t start, int64_t stop) const;

  private:
    // Facteur de conversion pour passer des ticks à des secondes
    double m_SecPerCount;
    // Garde en mémoire la fréquence du compteur
    LARGE_INTEGER m_Frequency;
  };
} // namespace FrostFireEngine
