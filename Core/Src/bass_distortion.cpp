/*
 * bass_distortion.cpp
 *
 *  Created on: Mar 20, 2026
 *      Author: Ruk3k
 */

#include "bass_distortion.hpp"

void bassDistortion::process(float32_t* pLeft, float32_t* pRight, uint32_t numSamples) {
	float32_t wetMixRate{ mix_ };
	float32_t dryMixRate{ 1.0f - wetMixRate };

	for (uint32_t i = 0; i < numSamples; ++i) {
		float32_t dryL{ pLeft[i] };
		float32_t wetL{ dryL * gain_ };
		applyHardClipping(wetL, threshold_);

		pLeft[i] = (dryL * dryMixRate) + (wetL * wetMixRate);

		float32_t dryR{ pRight[i] };
		float32_t wetR{ dryR * gain_ };
		applyHardClipping(wetR, threshold_);

		pRight[i] = (dryR * dryMixRate) + (wetR * wetMixRate);
	}
}

