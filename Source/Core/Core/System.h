// Copyright 2020 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <memory>

class SoundStream;

namespace AudioInterface
{
class AudioInterfaceState;
};
namespace DVDInterface
{
class DVDInterfaceState;
}
namespace DVDThread
{
class DVDThreadState;
}

namespace Core
{
// Central class that encapsulates the running system.
class System
{
public:
  ~System();

  System(const System&) = delete;
  System& operator=(const System&) = delete;

  System(System&&) = delete;
  System& operator=(System&&) = delete;

  // Intermediate instance accessor until global state is eliminated.
  static System& GetInstance()
  {
    static System instance;
    return instance;
  }

  void Initialize();

  bool IsDualCoreMode() const { return m_separate_cpu_and_gpu_threads; }
  bool IsMMUMode() const { return m_mmu_enabled; }

  SoundStream* GetSoundStream() const;
  void SetSoundStream(std::unique_ptr<SoundStream> sound_stream);
  bool IsSoundStreamRunning() const;
  void SetSoundStreamRunning(bool running);
  bool IsAudioDumpStarted() const;
  void SetAudioDumpStarted(bool started);

  AudioInterface::AudioInterfaceState& GetAudioInterfaceState() const;
  DVDInterface::DVDInterfaceState& GetDVDInterfaceState() const;
  DVDThread::DVDThreadState& GetDVDThreadState() const;

private:
  System();

  struct Impl;
  std::unique_ptr<Impl> m_impl;

  bool m_separate_cpu_and_gpu_threads = false;
  bool m_mmu_enabled = false;
};
}  // namespace Core
