/*
 * bass_preamp.hpp
 *
 *  Created on: Mar 11, 2026
 *      Author: RuK3k
 */

#pragma once
#include <array>
#include "arm_math.h"

class BassPreAmp {
public:
	BassPreAmp(float sampleRate);

	void setBass(float value);
	void setTreble(float value);
	void setVolume(float value);

	void process(int32_t* pTx, int32_t* pRx, uint32_t numSamples);

private:
	struct EffectorSpecs {
		static constexpr float32_t kBassFrequency   { 60.0f };
		static constexpr float32_t kTrebleFrequency { 4000.0f };
		static constexpr float32_t kBassQ           { 0.9f };
		static constexpr float32_t kTrebleQ         { 0.707f };
		static constexpr float32_t kMaxBoostDB      { 18.0f };
	};

	struct Math {
	static constexpr float32_t kPi{ 3.14159265358979323846f };
	static constexpr float kInt32ToFloat{ 1.0f / 2147483648.0f };
	static constexpr float kFloatToInt32{ 2147483647.0f };
	};

	static constexpr int kMaxBufferSize{ 96 };

	float sampleRate_;
	float rawBass_	{ 0.5f };
	float rawTreble_{ 0.5f };
	float rawVolume_{ 1.0f };
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
