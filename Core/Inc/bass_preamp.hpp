/*
 * bass_preamp.hpp
 *
 *  Created on: Mar 11, 2026
 *      Author: RuK3k
 */

#pragma once
#include <array>
#include "arm_math.h"
#include "audio_constants.hpp"
#include "effector.hpp"

class BassPreAmp : public Effector {
public:
	BassPreAmp(float sampleRate) : sampleRate_(sampleRate) {
		arm_biquad_cascade_df1_init_f32(&bassL_, 1, coeffsBass_.data(), stateBassL_.data());
		arm_biquad_cascade_df1_init_f32(&bassR_, 1, coeffsBass_.data(), stateBassR_.data());
		arm_biquad_cascade_df1_init_f32(&trebleL_, 1, coeffsTreble_.data(), stateTrebleL_.data());
		arm_biquad_cascade_df1_init_f32(&trebleR_, 1, coeffsTreble_.data(), stateTrebleR_.data());
		updateCoeffs();
	}

	void setBass  (float value) {	rawBass_   = value; needsUpdate_ = true; }
	void setTreble(float value) {	rawTreble_ = value; needsUpdate_ = true; }
	void setVolume(float value) {	rawVolume_ = value; 										 }

	void process(float32_t* pLeft, float32_t* pRight, uint32_t numSamples) override;

private:
	struct EffectorSpecs {
		static constexpr float32_t kBassFrequency   { 60.0f };
		static constexpr float32_t kTrebleFrequency { 4000.0f };
		static constexpr float32_t kBassQ           { 0.9f };
		static constexpr float32_t kTrebleQ         { 0.707f };
		static constexpr float32_t kMaxBoostDB      { 18.0f };
	};

	float32_t sampleRate_;
	float32_t rawBass_	{ 0.6f };
	float32_t rawTreble_{ 0.5f };
	float32_t rawVolume_{ 1.0f };
	bool needsUpdate_{ true };

	arm_biquad_casd_df1_inst_f32 bassL_, bassR_;
	arm_biquad_casd_df1_inst_f32 trebleL_, trebleR_;

	std::array<float32_t, 5> coeffsBass_{};
	std::array<float32_t, 5> coeffsTreble_{};

	std::array<float32_t, 4> stateBassL_{}, stateBassR_{};
	std::array<float32_t, 4> stateTrebleL_{}, stateTrebleR_{};

	void updateCoeffs();
	static void calcLowShelf(float* coeffs, float fs, float f0, float Q, float gainDB);
	static void calcHighShelf(float* coeffs, float fs, float f0, float Q, float gainDB);
};
