/*
 * auto_wah.hpp
 *
 *  Created on: Mar 20, 2026
 *      Author: Ruk3k
 */

#pragma once
#include <array>
#include "Effector.hpp"
#include "audio_constants.hpp"

class AutoWah: public Effector {
public:
	AutoWah(float32_t sampleRate)	: sampleRate_ (sampleRate) {
		updateFilter(minHz_);
		arm_biquad_cascade_df1_init_f32(&filterL_, 1, coeffs_.data(), stateL_.data());
		arm_biquad_cascade_df1_init_f32(&filterR_, 1, coeffs_.data(), stateR_.data());
	}

	void setSensitivity(float32_t sens) { sensitivity_ = sens; }
	void setResonance(float32_t q) { resonance_ = q; }
	void setMix(float32_t mix) { mix_ = mix; }
	void setRange(float32_t minHz, float32_t maxHz) {	minHz_ = minHz;	maxHz_ = maxHz;	}

	void process(float32_t* pLeft, float32_t* pRight, uint32_t numSamples) override;

private:
	float32_t sampleRate_  { Constants::Config::kDefaultSampleRate };
	float32_t sensitivity_ { 6.0f };
	float32_t resonance_	 { 3.0f };
	float32_t mix_				 { 1.0f };
	float32_t minHz_			 { 150.0f };
	float32_t maxHz_			 { 6000.0f };
	float32_t makeUpGain_  { 1.5f };

	float32_t envelope_		 { 0.0f };

	arm_biquad_casd_df1_inst_f32 filterL_, filterR_;

	std::array<float32_t, 4> stateL_{}, stateR_{};
	std::array<float32_t, 5> coeffs_{};

	void updateFilter(float32_t cutoffHz);
};

