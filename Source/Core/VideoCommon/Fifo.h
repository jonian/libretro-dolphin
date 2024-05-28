// Copyright 2008 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <cstddef>
#include "Common/CommonTypes.h"

class PointerWrap;

namespace Fifo
{
void Init();
void Shutdown();
void Prepare();  // Must be called from the CPU thread.
void DoState(PointerWrap& f);
void PauseAndLock(bool doLock, bool unpauseOnUnlock);
void UpdateWantDeterminism(bool want);
bool UseDeterministicGPUThread();

// Used for diagnostics.
enum class SyncGPUReason
{
  Other,
  Wraparound,
  EFBPoke,
  PerfQuery,
  BBox,
  Swap,
  AuxSpace,
};
// In deterministic GPU thread mode this waits for the GPU to be done with pending work.
void SyncGPU(SyncGPUReason reason, bool may_move_read_ptr = true);

// In single core mode, this runs the GPU for a single slice.
// In dual core mode, this synchronizes with the GPU thread.
void SyncGPUForRegisterAccess();

void PushFifoAuxBuffer(const void* ptr, size_t size);
void* PopFifoAuxBuffer(size_t size);

void FlushGpu();
void RunGpu();
void GpuMaySleep();
void RunGpuLoop();
void StopGpuLoop();
void ExitGpuLoop();
void EmulatorState(bool running);
bool AtBreakpoint();
void ResetVideoBuffer();

}  // namespace Fifo
