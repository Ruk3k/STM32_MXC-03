/**
 * @file bass_preamp.hpp
 * @brief Bass preamp effect with low/high shelf EQ and volume control.
 */

#pragma once
#include "arm_math.h"
#include "effector.hpp"
#include <array>

/**
 * @brief Preamp effect for bass tone shaping.
 */
class BassPreAmp : public Effector {
public:
  /**
   * @brief Construct BassPreAmp instance.
   * @param sampleRate Audio sample rate in Hz.
   */
  BassPreAmp(float sampleRate) : sampleRate_(sampleRate) {
    arm_biquad_cascade_df1_init_f32(&bassL_, 1, coeffsBass_.data(),
                                    stateBassL_.data());
    arm_biquad_cascade_df1_init_f32(&bassR_, 1, coeffsBass_.data(),
                                    stateBassR_.data());
    arm_biquad_cascade_df1_init_f32(&trebleL_, 1, coeffsTreble_.data(),
                                    stateTrebleL_.data());
    arm_biquad_cascade_df1_init_f32(&trebleR_, 1, coeffsTreble_.data(),
                                    stateTrebleR_.data());
    updateCoeffs();
  }

  /** @brief Set normalized bass knob value. */
  void setBass(float value) {
    rawBass_ = value;
    needsUpdate_ = true;
  }
  /** @brief Set normalized treble knob value. */
  void setTreble(float value) {
    rawTreble_ = value;
    needsUpdate_ = true;
  }
  /** @brief Set output volume scalar. */
  void setVolume(float value) { rawVolume_ = value; }

  /** @copydoc Effector::process */
  void process(float32_t *pLeft, float32_t *pRight,
               uint32_t numFrames) override;

private:
  struct EffectorSpecs {
    static constexpr float32_t kBassFrequency{60.0f};
    static constexpr float32_t kTrebleFrequency{4000.0f};
    static constexpr float32_t kBassQ{0.9f};
    static constexpr float32_t kTrebleQ{0.707f};
    static constexpr float32_t kMaxBoostDB{18.0f};
  };

  float32_t sampleRate_;
  float32_t rawBass_{0.6f};
  float32_t rawTreble_{0.5f};
  float32_t rawVolume_{1.0f};
  bool needsUpdate_{true};

  arm_biquad_casd_df1_inst_f32 bassL_, bassR_;
  arm_biquad_casd_df1_inst_f32 trebleL_, trebleR_;

  std::array<float32_t, 5> coeffsBass_{};
  std::array<float32_t, 5> coeffsTreble_{};

  std::array<float32_t, 4> stateBassL_{}, stateBassR_{};
  std::array<float32_t, 4> stateTrebleL_{}, stateTrebleR_{};

  /** @brief Recompute filter coefficients from current knob values. */
  void updateCoeffs();
  /**
   * @brief Calculate low-shelf biquad coefficients.
   */
  static void calcLowShelf(float *coeffs, float fs, float f0, float Q,
                           float gainDB);
  /**
   * @brief Calculate high-shelf biquad coefficients.
   */
  static void calcHighShelf(float *coeffs, float fs, float f0, float Q,
                            float gainDB);
};
