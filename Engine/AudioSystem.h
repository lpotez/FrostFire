#pragma once

#include <xaudio2.h>
#include <mmreg.h>
#include <wrl/client.h>
#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <vector>
#include <memory>
#include <atomic>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include "Engine/ECS/core/World.h"
#include "ECS/core/System.h"
#include "AudioMixer.h"

namespace FrostFireEngine
{
  class VoiceCallback : public IXAudio2VoiceCallback {
  public:
    HANDLE            hBufferEndEvent;
    std::atomic<bool> bufferEnded;

    VoiceCallback() : hBufferEndEvent(CreateEvent(nullptr, FALSE, FALSE, nullptr)), bufferEnded(false)
    {
    }

    ~VoiceCallback()
    {
      if (hBufferEndEvent) {
        CloseHandle(hBufferEndEvent);
      }
    }

    void STDMETHODCALLTYPE OnBufferEnd(void* pBufferContext) override
    {
      bufferEnded.store(true);
      SetEvent(hBufferEndEvent);
    }

    void STDMETHODCALLTYPE OnVoiceProcessingPassStart(UINT32) override {}
    void STDMETHODCALLTYPE OnVoiceProcessingPassEnd() override {}
    void STDMETHODCALLTYPE OnStreamEnd() override {}
    void STDMETHODCALLTYPE OnBufferStart(void*) override {}
    void STDMETHODCALLTYPE OnLoopEnd(void*) override {}
    void STDMETHODCALLTYPE OnVoiceError(void*, HRESULT) override {}
  };

  class AudioSystem : public System {
  public:
    AudioSystem() : xAudio2(nullptr), masteringVoice(nullptr)
    {
    }

    ~AudioSystem() override = default;

    void Initialize() override
    {
      std::lock_guard<std::mutex> lock(mutex);
      HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
      if (FAILED(hr)) {
        throw std::runtime_error("Erreur lors de l'initialisation de COM");
      }

      hr = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
      if (FAILED(hr)) {
        CoUninitialize();
        throw std::runtime_error("Erreur lors de la création de XAudio2");
      }

      hr = xAudio2->CreateMasteringVoice(&masteringVoice, XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE);
      if (FAILED(hr)) {
        xAudio2->Release();
        xAudio2 = nullptr;
        CoUninitialize();
        throw std::runtime_error("Erreur lors de la création de la voix principale");
      }

      float globalVolume = AudioMixer::GetInstance().GetVolume();
      masteringVoice->SetVolume(globalVolume);
    }

    void Cleanup() override
    {
      std::lock_guard<std::mutex> lock(mutex);
      for (auto& voice : activeVoices) {
        if (voice.sourceVoice) {
          voice.sourceVoice->ExitLoop();
          voice.sourceVoice->Stop(0, XAUDIO2_COMMIT_NOW);
          voice.sourceVoice->DestroyVoice();
          voice.sourceVoice = nullptr;
        }
      }
      activeVoices.clear();
      if (masteringVoice) {
        masteringVoice->DestroyVoice();
        masteringVoice = nullptr;
      }
      if (xAudio2) {
        xAudio2->Release();
        xAudio2 = nullptr;
      }
      CoUninitialize();
    }

    void Update(float deltaTime) override
    {
      UNREFERENCED_PARAMETER(deltaTime);
      std::lock_guard<std::mutex> lock(mutex);
      std::erase_if(activeVoices,
                    [&](const ActiveVoice& v)
                    {
                      if (v.callback->bufferEnded.load()) {
                        if (v.sourceVoice) {
                          v.sourceVoice->Stop(0);
                          v.sourceVoice->FlushSourceBuffers();
                          v.sourceVoice->DestroyVoice();
                        }
                        return true;
                      }
                      return false;
                    });

      if (masteringVoice) {
        float globalVolume = AudioMixer::GetInstance().GetVolume();
        masteringVoice->SetVolume(globalVolume);
      }
    }

    void PlaySound(const std::string& filePath, float volume = 1.0f, bool loop = false)
    {
      if (!xAudio2 || !masteringVoice) {
        throw std::runtime_error("XAudio2 n'est pas initialisé.");
      }

      std::lock_guard<std::mutex> lock(mutex);
      WAVEFORMATEX waveFormat;
      auto audioData = std::make_shared<std::vector<BYTE>>();
      LoadWavFile(filePath, waveFormat, *audioData);

      std::shared_ptr<VoiceCallback> callback = std::make_shared<VoiceCallback>();

      IXAudio2SourceVoice* sourceVoice = nullptr;
      HRESULT hr = xAudio2->CreateSourceVoice(&sourceVoice, &waveFormat, 0, XAUDIO2_DEFAULT_FREQ_RATIO, callback.get());
      if (FAILED(hr)) {
        throw std::runtime_error("Erreur lors de la création de la source voice");
      }

      XAUDIO2_BUFFER buffer = {};
      buffer.AudioBytes = static_cast<UINT32>(audioData->size());
      buffer.pAudioData = audioData->data();
      buffer.Flags = XAUDIO2_END_OF_STREAM;
      buffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;

      hr = sourceVoice->SubmitSourceBuffer(&buffer);
      if (FAILED(hr)) {
        sourceVoice->DestroyVoice();
        throw std::runtime_error("Erreur lors de l'envoi des données audio");
      }

      hr = sourceVoice->Start(0);
      if (FAILED(hr)) {
        sourceVoice->DestroyVoice();
        throw std::runtime_error("Erreur lors du démarrage de la lecture audio");
      }

      activeVoices.push_back({sourceVoice, audioData, callback, false});
    }

    void SetVolume(float volume)
    {
      std::lock_guard<std::mutex> lock(mutex);
      AudioMixer::GetInstance().SetVolume(volume);
      if (masteringVoice) {
        masteringVoice->SetVolume(volume);
      }
    }

    void PauseAllSounds()
    {
      std::lock_guard<std::mutex> lock(mutex);
      for (auto& voice : activeVoices) {
        if (!voice.isPaused && voice.sourceVoice) {
          HRESULT hr = voice.sourceVoice->Stop(0);
          if (SUCCEEDED(hr)) {
            voice.isPaused = true;
          }
        }
      }
    }

    void ResumeAllSounds()
    {
      std::lock_guard<std::mutex> lock(mutex);
      for (auto& voice : activeVoices) {
        if (voice.isPaused && voice.sourceVoice) {
          HRESULT hr = voice.sourceVoice->Start(0);
          if (SUCCEEDED(hr)) {
            voice.isPaused = false;
          }
        }
      }
    }

    void StopAllSounds()
    {
      std::lock_guard<std::mutex> lock(mutex);
      for (auto& voice : activeVoices) {
        if (voice.sourceVoice) {
          voice.sourceVoice->ExitLoop();
          voice.sourceVoice->Stop(0, XAUDIO2_COMMIT_NOW);
          voice.sourceVoice->DestroyVoice();
          voice.sourceVoice = nullptr;
        }
      }
      activeVoices.clear();
    }

    static AudioSystem& Get()
    {
      auto* system = World::GetInstance().GetSystem<AudioSystem>();
      if (!system) {
        throw std::runtime_error("AudioSystem not initialized");
      }
      return *system;
    }

  private:
    struct ActiveVoice {
      IXAudio2SourceVoice*               sourceVoice;
      std::shared_ptr<std::vector<BYTE>> audioData;
      std::shared_ptr<VoiceCallback>     callback;
      bool                               isPaused;
    };

    IXAudio2*                xAudio2;
    IXAudio2MasteringVoice*  masteringVoice;
    std::mutex               mutex;
    std::vector<ActiveVoice> activeVoices;

    struct WavHeader {
      char     riff[4];
      uint32_t fileSize;
      char     wave[4];
    };

    struct FmtChunk {
      char     fmt[4];
      uint32_t chunkSize;
      uint16_t audioFormat;
      uint16_t numChannels;
      uint32_t sampleRate;
      uint32_t byteRate;
      uint16_t blockAlign;
      uint16_t bitsPerSample;
    };

    struct DataChunk {
      char     data[4];
      uint32_t dataSize;
    };

    static void LoadWavFile(const std::string& filePath, WAVEFORMATEX& waveFormat, std::vector<BYTE>& audioData)
    {
      std::ifstream file(filePath, std::ios::binary);
      if (!file) {
        throw std::runtime_error("Impossible d'ouvrir le fichier WAV : " + filePath);
      }

      WavHeader wavHeader;
      file.read(reinterpret_cast<char*>(&wavHeader), sizeof(WavHeader));
      if (std::strncmp(wavHeader.riff, "RIFF", 4) != 0 || std::strncmp(wavHeader.wave, "WAVE", 4) != 0) {
        throw std::runtime_error("Fichier WAV invalide : " + filePath);
      }

      FmtChunk fmtChunk;
      file.read(reinterpret_cast<char*>(&fmtChunk), sizeof(FmtChunk));
      if (std::strncmp(fmtChunk.fmt, "fmt ", 4) != 0 || fmtChunk.audioFormat != 1) {
        throw std::runtime_error("Fichier WAV non PCM ou invalide : " + filePath);
      }

      waveFormat.wFormatTag = WAVE_FORMAT_PCM;
      waveFormat.nChannels = fmtChunk.numChannels;
      waveFormat.nSamplesPerSec = fmtChunk.sampleRate;
      waveFormat.nAvgBytesPerSec = fmtChunk.byteRate;
      waveFormat.nBlockAlign = fmtChunk.blockAlign;
      waveFormat.wBitsPerSample = fmtChunk.bitsPerSample;
      waveFormat.cbSize = 0;

      DataChunk dataChunk;
      while (file.read(reinterpret_cast<char*>(&dataChunk), sizeof(DataChunk))) {
        if (std::strncmp(dataChunk.data, "data", 4) == 0) {
          break;
        }
        file.seekg(dataChunk.dataSize, std::ios::cur);
      }

      if (std::strncmp(dataChunk.data, "data", 4) != 0) {
        throw std::runtime_error("Chunk de données manquant dans le fichier WAV : " + filePath);
      }

      audioData.resize(dataChunk.dataSize);
      file.read(reinterpret_cast<char*>(audioData.data()), dataChunk.dataSize);
    }
  };
}
