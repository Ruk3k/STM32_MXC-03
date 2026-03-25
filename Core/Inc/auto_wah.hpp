/**
 * @file auto_wah.hpp
 * @brief Auto-wah effect implementation.
 */

#pragma once
#include "audio_system.hpp"
#include "effector.hpp"
#include <array>

/**
 * @brief Envelope-following auto-wah effect.
 */
class AutoWah : public Effector {
public:
  /**
   * @brief Construct AutoWah instance.
   * @param sampleRate Audio sample rate in Hz.
   */
  AutoWah(float32_t sampleRate) : sampleRate_(sampleRate) {
    updateFilter(minHz_);
    arm_biquad_cascade_df1_init_f32(&filterL_, 1, coeffs_.data(),
                                    stateL_.data());
    arm_biquad_cascade_df1_init_f32(&filterR_, 1, coeffs_.data(),
                                    stateR_.data());
  }

  /** @brief Set envelope sensitivity. */
  void setSensitivity(float32_t sens) { sensitivity_ = sens; }
  /** @brief Set filter resonance (Q-like parameter). */
  void setResonance(float32_t q) { resonance_ = q; }
  /** @brief Set wet/dry mix ratio. */
  void setMix(float32_t mix) { mix_ = mix; }
  /**
   * @brief Set wah sweep frequency range.
   * @param minHz Minimum cutoff frequency.
   * @param maxHz Maximum cutoff frequency.
   */
  void setRange(float32_t minHz, float32_t maxHz) {
    minHz_ = minHz;
    maxHz_ = maxHz;
  }

  /** @copydoc Effector::process */
  void process(float32_t *pLeft, float32_t *pRight,
               uint32_t numFrames) override;

private:
  float32_t sampleRate_{AudioConfig::SampleRate};
  float32_t sensitivity_{6.0f};
  float32_t resonance_{3.0f};
  float32_t mix_{1.0f};
  float32_t minHz_{150.0f};
  float32_t maxHz_{6000.0f};
  float32_t makeUpGain_{1.5f};

  float32_t envelope_{0.0f};

  arm_biquad_casd_df1_inst_f32 filterL_, filterR_;

  std::array<float32_t, 4> stateL_{}, stateR_{};
  std::array<float32_t, 5> coeffs_{};

  /**
   * @brief Recalculate biquad coefficients for current cutoff.
   * @param cutoffHz Target cutoff frequency in Hz.
   */
  void updateFilter(float32_t cutoffHz);
};
