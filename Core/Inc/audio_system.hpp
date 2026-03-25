/*
 * audio_system.hpp
 *
 *  Audio configuration, mixer, and buffer management
 *      Author: Ruk3k
 */

#pragma once

#include "arm_math.h"
#include "audio_ring_buffer.hpp"
#include <algorithm>
#include <array>

// ============================================================================
// 0. Audio Math & Conversion Constants
// ============================================================================
namespace Audio {
namespace Math {
constexpr float32_t Pi{3.14159265358979323846f};
}

namespace Convert {
constexpr float32_t Int16ToFloat{1.0f / 32768.0f};
constexpr float32_t Int32ToFloat{1.0f / 2147483648.0f};
constexpr float32_t FloatToInt32{2147483647.0f};
} // namespace Convert
} // namespace Audio

// ============================================================================
// 1. Audio System Configurations
// ============================================================================
namespace AudioConfig {
constexpr uint32_t SampleRate{48000}; // 48kHz
constexpr uint32_t Channels{2};       // Stereo

constexpr uint32_t FramesPerBlock{(SampleRate) / 1000};        // 48
constexpr uint32_t SamplesPerBlock{FramesPerBlock * Channels}; // 96
constexpr uint32_t DMABufferSize{SamplesPerBlock * 2}; // 192 (Double Buffering)

constexpr uint32_t USBRingBufferSize{2048};
} // namespace AudioConfig

// ============================================================================
// 2. Hardware DMA & USB Buffers (External Declarations)
// ============================================================================
// SAI DMA Buffers
extern
	__attribute__((section(".sram4"), aligned(4))) std::array<int32_t, AudioConfig::DMABufferSize>
		monitorTxBuffer;

extern
	__attribute__((section(".sram4"), aligned(4))) std::array<int32_t, AudioConfig::DMABufferSize>
		ADCRxBuffer;

extern
	__attribute__((aligned(32))) std::array<int32_t, AudioConfig::DMABufferSize>
		frontUSBRxBuffer;

extern
	__attribute__((aligned(32))) std::array<int32_t, AudioConfig::DMABufferSize>
		rearUSBRxBuffer;

// USB Ring Buffers
extern AudioRingBuffer<int16_t, AudioConfig::USBRingBufferSize> mainUSBRxBuffer;
extern AudioRingBuffer<int16_t, AudioConfig::USBRingBufferSize> mainUSBTxBuffer;

// ============================================================================
// 3. DSP & Mixer Core
// ============================================================================
namespace Mixer {
extern std::array<float32_t, AudioConfig::FramesPerBlock> chLeft;
extern std::array<float32_t, AudioConfig::FramesPerBlock> chRight;

struct Parameter {
	float32_t gainADCLeft{0.0f};
	float32_t gainADCRight{0.0f};
	float32_t gainMainUSB{1.0f};
	float32_t gainFrontUSB{1.0f};
	float32_t gainRearUSB{1.0f};
	float32_t gainMaster{0.8f};
};

extern Parameter param;

// Inline utility functions
inline float32_t clampGain(float32_t value) {
	return std::min(1.0f, std::max(0.0f, value));
}
} // namespace Mixer

// ============================================================================
// 4. Debugging & Monitoring
// ============================================================================
extern uint32_t d_usbRxAvailableFrames;
