/**
 * @file effects_chain.hpp
 * @brief Effect chain configuration and public control API.
 */

#pragma once

#include "arm_math.h"
#include "auto_wah.hpp"
#include "bass_preamp.hpp"
#include "effector.hpp"
#include <array>
#include <cstddef>

namespace FX {
// ========================================================================
// Effects Chain Configuration
// ========================================================================
/** @brief Maximum number of effect slots in chain. */
constexpr size_t MaxChainCount{5};

// ========================================================================
// Effect Instances and State
// ========================================================================
/** @brief Ordered pointers to active effects. */
extern std::array<Effector *, MaxChainCount> effectChain;
/** @brief Bypass state for each effect slot. */
extern std::array<bool, MaxChainCount> isBypassed;

/** @brief Bass preamp effect instance. */
extern BassPreAmp sadowskyPreAmp;
/** @brief Auto-wah effect instance. */
extern AutoWah autoWah;

// ========================================================================
// Chain Management Interface
// ========================================================================
/**
 * @brief Initialize the effects chain
 */
void initializeChain();

/**
 * @brief Process audio samples through effects chain
 * @param pLeft pointer to left channel samples
 * @param pRight pointer to right channel samples
 * @param numFrames number of frames to process
 */
void processChain(float32_t *pLeft, float32_t *pRight, uint32_t numFrames);

/**
 * @brief Toggle bypass state of an effect
 * @param index effect index (0 to MaxChainCount-1)
 */
void toggleBypass(size_t index);

/**
 * @brief Get bypass state of an effect
 * @param index effect index
 * @return true if bypassed, false otherwise
 */
bool isBypassedEffect(size_t index);
} // namespace FX
