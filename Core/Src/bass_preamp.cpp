/*
 * bass_preamp.cpp
 *
 *  Created on: Mar 11, 2026
 *      Author: RuK3k
 */

#include "bass_preamp.hpp"
#include <cmath>

BassPreAmp::BassPreAmp(float sampleRate) : sampleRate_(sampleRate) {
	arm_biquad_cascade_df1_init_f32(&bassL_, 1, coeffsBass_.data(), stateBassL_.data());
	arm_biquad_cascade_df1_init_f32(&bassR_, 1, coeffsBass_.data(), stateBassR_.data());
	arm_biquad_cascade_df1_init_f32(&trebleL_, 1, coeffsTreble_.data(), stateTrebleL_.data());
	arm_biquad_cascade_df1_init_f32(&trebleR_, 1, coeffsTreble_.data(), stateTrebleR_.data());

	updateCoeffs();
}

void BassPreAmp::setBass(float value) {
	rawBass_ = value;
	needsUpdate_ = true;
}

void BassPreAmp::setTreble(float value) {
	rawTreble_ = value;
	needsUpdate_ = true;
}

void BassPreAmp::setVolume(float value) {
	rawVolume_ = value;
}

void BassPreAmp::process(int32_t* pTx, int32_t* pRx, uint32_t numSamples) {
	if (needsUpdate_) {
		updateCoeffs();
		needsUpdate_ = false;
	}

	static float32_t s_tempL[ kMaxBufferSize ], s_tempR[ kMaxBufferSize ];
	const uint32_t blockSize{ numSamples / 2 };

	for (uint32_t i = 0, j = 0; i < numSamples; i += 2, j++) {
			s_tempL[j] = static_cast<float32_t>(pRx[i]     * Math::kInt32ToFloat);
			s_tempR[j] = static_cast<float32_t>(pRx[i + 1] * Math::kInt32ToFloat);
	}

	arm_biquad_cascade_df1_f32(&bassL_, s_tempL, s_tempL, blockSize);
	arm_biquad_cascade_df1_f32(&bassR_, s_tempR, s_tempR, blockSize);
	arm_biquad_cascade_df1_f32(&trebleL_, s_tempL, s_tempL, blockSize);
	arm_biquad_cascade_df1_f32(&trebleR_, s_tempR, s_tempR, blockSize);

	for (uint32_t i = 0, j=0; i < numSamples; i += 2, j++) {
			pTx[i]     = (int32_t)__SSAT(static_cast<int32_t>(s_tempL[j] * rawVolume_ * Math::kFloatToInt32), 31);
			pTx[i + 1] = (int32_t)__SSAT(static_cast<int32_t>(s_tempR[j] * rawVolume_ * Math::kFloatToInt32), 31);
  }
}

void BassPreAmp::updateCoeffs() {
	calcLowShelf (coeffsBass_.data(), sampleRate_,
								EffectorSpecs::kBassFrequency,
								EffectorSpecs::kBassQ,
								rawBass_ * EffectorSpecs::kMaxBoostDB);
	calcHighShelf(coeffsTreble_.data(), sampleRate_,
								EffectorSpecs::kTrebleFrequency,
								EffectorSpecs::kTrebleQ,
								rawTreble_ * EffectorSpecs::kMaxBoostDB);
}

void BassPreAmp::calcLowShelf(float* coeffs, float fs, float f0, float Q, float gainDB) {
	// Adapted from Robert Bristow-Johnson's Audio EQ Cookbook
	const float32_t A     { std::pow(10.0f, gainDB / 40.0f) };
	const float32_t w0    { 2.0f * Math::kPi * f0 / fs };
	const float32_t sinW0 { std::sin(w0) };
	const float32_t cosW0 { std::cos(w0) };
	const float32_t alpha { sinW0 / (2.0f * Q) };
	const float32_t beta  { 2.0f * std::sqrt(A) * alpha };

	const float32_t b0 {         A * ( (A + 1.0f) - (A - 1.0f) * cosW0 + beta ) };
	const float32_t b1 {  2.0f * A * ( (A - 1.0f) - (A + 1.0f) * cosW0        ) };
	const float32_t b2 {         A * ( (A + 1.0f) - (A - 1.0f) * cosW0 - beta ) };
	const float32_t a0 {               (A + 1.0f) + (A - 1.0f) * cosW0 + beta   };
	const float32_t a1 { -2.0f *     ( (A - 1.0f) + (A + 1.0f) * cosW0        ) };
	const float32_t a2 {               (A + 1.0f) + (A - 1.0f) * cosW0 - beta   };

	// BiQuad filter difference equation:
	// a0*y[n] + a1*y[n-1] + a2*y[n-2] = b0*x[n] + b1*x[n-1] + b2*x[n-2]
	const float32_t invA0{ 1.0f / a0 };
	coeffs[0] =  b0 * invA0;
	coeffs[1] =  b1 * invA0;
	coeffs[2] =  b2 * invA0;
	coeffs[3] = -a1 * invA0;
	coeffs[4] = -a2 * invA0;
}

void BassPreAmp::calcHighShelf(float* coeffs, float fs, float f0, float Q, float gainDB) {
	// Adapted from Robert Bristow-Johnson's Audio EQ Cookbook
	const float32_t A     { std::pow(10.0f, gainDB / 40.0f) };
	const float32_t w0    { 2.0f * Math::kPi * f0 / fs };
	const float32_t sinW0 { std::sin(w0) };
	const float32_t cosW0 { std::cos(w0) };
	const float32_t alpha { sinW0 / (2.0f * Q) };
	const float32_t beta  { 2.0f * std::sqrt(A) * alpha };

	const float32_t b0 {         A * ( (A + 1.0f) + (A - 1.0f) * cosW0 + beta ) };
	const float32_t b1 { -2.0f * A * ( (A - 1.0f) + (A + 1.0f) * cosW0        ) };
	const float32_t b2 {         A * ( (A + 1.0f) + (A - 1.0f) * cosW0 - beta ) };
	const float32_t a0 {               (A + 1.0f) - (A - 1.0f) * cosW0 + beta   };
	const float32_t a1 {  2.0f *     ( (A - 1.0f) - (A + 1.0f) * cosW0        ) };
	const float32_t a2 {               (A + 1.0f) - (A - 1.0f) * cosW0 - beta   };

	// BiQuad filter difference equation:
	// a0*y[n] + a1*y[n-1] + a2*y[n-2] = b0*x[n] + b1*x[n-1] + b2*x[n-2]
	const float32_t invA0{ 1.0f / a0 };
	coeffs[0] =  b0 * invA0;
	coeffs[1] =  b1 * invA0;
	coeffs[2] =  b2 * invA0;
	coeffs[3] = -a1 * invA0;
	coeffs[4] = -a2 * invA0;
}
