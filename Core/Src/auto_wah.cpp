/*
 * auto_wah.cpp
 *
 *  Created on: Mar 20, 2026
 *      Author: Ruk3k
 */

#include "auto_wah.hpp"
#include "arm_math.h"
#include <array>
#include <cmath>


void AutoWah::process(float32_t *pLeft, float32_t *pRight, uint32_t numFrames) {
  static std::array<float32_t, 48> dryL{}, dryR{};
  for (uint32_t i = 0; i < numFrames; ++i) {
    dryL[i] = pLeft[i];
    dryR[i] = pRight[i];
  }

  float32_t maxInThisBlock{0.0f};
  for (uint32_t i = 0; i < numFrames; ++i) {
    float32_t absSignal{std::abs(pLeft[i])};
    if (absSignal > maxInThisBlock) {
      maxInThisBlock = absSignal;
    }
  }

  float32_t target{maxInThisBlock * sensitivity_};
  if (target > envelope_) {
    envelope_ = 0.7f * envelope_ + 0.3f * target;
  } else {
    envelope_ = 0.99f * envelope_ + 0.01f * target;
  }

  if (envelope_ > 1.0f)
    envelope_ = 1.0f;

  float32_t cutoff = minHz_ * std::pow(2.0f, envelope_ * 4.0f);
  if (cutoff > maxHz_)
    cutoff = maxHz_;

  updateFilter(cutoff);

  arm_biquad_cascade_df1_f32(&filterL_, pLeft, pLeft, numFrames);
  arm_biquad_cascade_df1_f32(&filterR_, pRight, pRight, numFrames);

  float32_t wetMixRate{mix_};
  float32_t dryMixRate{1.0f - wetMixRate};

  for (uint32_t i = 0; i < numFrames; ++i) {
    pLeft[i] = ((dryL[i] * dryMixRate) + (pLeft[i] * wetMixRate)) * makeUpGain_;
    pRight[i] =
        ((dryR[i] * dryMixRate) + (pRight[i] * wetMixRate)) * makeUpGain_;
  }
}

void AutoWah::updateFilter(float32_t cutoffHz) {
  float32_t omega{2.0f * Audio::Math::Pi * cutoffHz / sampleRate_};
  float32_t sinW{std::sin(omega)};
  float32_t cosW{std::cos(omega)};
  float32_t alpha{sinW / (2.0f * resonance_)};

  float32_t a0{1.0f + alpha};
  float32_t invA0{1.0f / a0};

  coeffs_[0] = (1.0f - cosW) / 2.0f * invA0;
  coeffs_[1] = (1.0f - cosW) * invA0;
  coeffs_[2] = coeffs_[0];
  coeffs_[3] = (2.0f * cosW) * invA0;
  coeffs_[4] = (alpha - 1.0) * invA0;
}
