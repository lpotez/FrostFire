#include "Clock.h"
#include <stdexcept>

namespace FrostFireEngine
{
  Clock::Clock()
  {
    // Récupération de la fréquence du compteur haute performance
    if (!QueryPerformanceFrequency(&m_Frequency))
    {
      throw std::runtime_error("Le compteur haute performance n'est pas disponible sur ce système");
    }

    // Initialisation du facteur de conversion
    m_SecPerCount = 1.0 / static_cast<double>(m_Frequency.QuadPart);
  }

  int64_t Clock::GetTimeCount()
  {
    LARGE_INTEGER currentCount;
    if (!QueryPerformanceCounter(&currentCount))
    {
      throw std::runtime_error("Erreur lors de la lecture du compteur haute performance");
    }
    return currentCount.QuadPart;
  }

  double Clock::GetTimeBetweenCounts(int64_t start, int64_t stop) const
  {
    // Vérifie que stop est bien après start
    if (stop < start)
    {
      return 0.0;
    }

    int64_t delta = stop - start;
    // Conversion en secondes en utilisant le facteur précalculé
    return static_cast<double>(delta) * m_SecPerCount;
  }
} // namespace FrostFireEngine
