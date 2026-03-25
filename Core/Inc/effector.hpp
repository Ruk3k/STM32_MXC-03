/**
 * @file effector.hpp
 * @brief Base interface for audio effects.
 */

#pragma once
#include "arm_math.h"

/**
 * @brief Abstract base class for block-based stereo effects.
 */
class Effector {
public:
  /** @brief Virtual destructor. */
  virtual ~Effector() = default;

  /**
   * @brief Process one block of stereo audio in-place.
   * @param pLeft Pointer to left-channel frame buffer.
   * @param pRight Pointer to right-channel frame buffer.
   * @param numFrames Number of frames in the block.
   */
  virtual void process(float32_t *pLeft, float32_t *pRight,
                       uint32_t numFrames) = 0;
};
