#pragma once
#include "Singleton.h"
#include <mutex>

namespace FrostFireEngine
{
  class AudioMixer : public CSingleton<AudioMixer> {
    friend class CSingleton<AudioMixer>;

  public:
    void SetVolume(float volume)
    {
      std::lock_guard<std::mutex> lock(mutex);
      if (!isMuted) {
        globalVolume = volume;
      }
      else {
        lastVolume = volume;
      }
    }

    float GetVolume() const
    {
      std::lock_guard<std::mutex> lock(mutex);
      if (isMuted) {
        return 0.0f;
      }
      return globalVolume;
    }

    void Mute()
    {
      std::lock_guard<std::mutex> lock(mutex);
      if (!isMuted) {
        lastVolume = globalVolume;
        isMuted = true;
      }
    }

    void Unmute()
    {
      std::lock_guard<std::mutex> lock(mutex);
      if (isMuted) {
        globalVolume = lastVolume;
        isMuted = false;
      }
    }

  private:
    AudioMixer() : globalVolume(1.0f), lastVolume(1.0f), isMuted(false)
    {
    }
    ~AudioMixer() = default;

    mutable std::mutex mutex;
    float              globalVolume;
    float              lastVolume;
    bool               isMuted;
  };
}
