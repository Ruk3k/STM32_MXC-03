/*
 * effects_chain.cpp
 *
 *  Effects chain implementation
 */

#include "effects_chain.hpp"
#include "audio_system.hpp"

namespace FX {
// ========================================================================
// Effect Instances (Definitions)
// ========================================================================
std::array<Effector *, MaxChainCount> effectChain{};
std::array<bool, MaxChainCount> isBypassed{};

BassPreAmp sadowskyPreAmp{static_cast<float>(AudioConfig::SampleRate)};
AutoWah autoWah{static_cast<float>(AudioConfig::SampleRate)};

// ========================================================================
// Chain Management Implementation
// ========================================================================
void initializeChain() {
	// Clear all bypasses by default
	isBypassed.fill(true);

	// Initialize effect chain
	effectChain[0] = &sadowskyPreAmp;
	isBypassed[0] = false; // Bass Preamp enabled by default

	effectChain[1] = &autoWah;
	isBypassed[1] = true; // Auto Wah disabled by default

	// Remaining slots empty
	for (size_t i = 2; i < MaxChainCount; ++i) {
		effectChain[i] = nullptr;
		isBypassed[i] = true;
	}
}

void processChain(float32_t *pLeft, float32_t *pRight, uint32_t numFrames) {
	for (size_t i = 0; i < MaxChainCount; ++i) {
		if (effectChain[i] != nullptr && !isBypassed[i]) {
			effectChain[i]->process(pLeft, pRight, numFrames);
		}
	}
}

void toggleBypass(size_t index) {
	if (index < MaxChainCount && effectChain[index] != nullptr) {
		isBypassed[index] = !isBypassed[index];
	}
}

bool isBypassedEffect(size_t index) {
	if (index < MaxChainCount) {
		return isBypassed[index];
	}
	return true;
}
} // namespace FX
