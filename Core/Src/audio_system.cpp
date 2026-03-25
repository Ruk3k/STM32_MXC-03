/*
 * audio_system.cpp
 *
 *  Audio buffer and mixer variable definitions
 */

#include "audio_system.hpp"

// ============================================================================
// Hardware DMA & USB Buffers (Definitions)
// ============================================================================
__attribute__((section(".sram4"), aligned(4))) std::array<int32_t, AudioConfig::DMABufferSize>
	monitorTxBuffer{};

__attribute__((section(".sram4"), aligned(4))) std::array<int32_t, AudioConfig::DMABufferSize>
	ADCRxBuffer{};

__attribute__((aligned(32))) std::array<int32_t, AudioConfig::DMABufferSize>
	frontUSBRxBuffer{};

__attribute__((aligned(32))) std::array<int32_t, AudioConfig::DMABufferSize>
	rearUSBRxBuffer{};

// USB Ring Buffers
AudioRingBuffer<int16_t, AudioConfig::USBRingBufferSize> mainUSBRxBuffer;
AudioRingBuffer<int16_t, AudioConfig::USBRingBufferSize> mainUSBTxBuffer;

// ============================================================================
// DSP & Mixer Core (Definitions)
// ============================================================================
namespace Mixer {
std::array<float32_t, AudioConfig::FramesPerBlock> chLeft{};
std::array<float32_t, AudioConfig::FramesPerBlock> chRight{};
Parameter param;
} // namespace Mixer

// ============================================================================
// Debugging & Monitoring (Definitions)
// ============================================================================
uint32_t d_usbRxAvailableFrames{};
