/*
 * bass_distortion.hpp
 *
 *  Created on: Mar 20, 2026
 *      Author: Ruk3k
 */

#pragma once
#include "arm_math.h"
#include "effector.hpp"

class bassDistortion : public Effector {
public:
	bassDistortion() : gain_(2.0f),	threshold_(0.5f),	mix_(0.5f) {}

	void setGain(float32_t gain) { gain_ = gain; }
	void setThreshold(float32_t threshold) { threshold_ = threshold; }
	void setMix(float32_t mix) { mix_ = mix; }

	void process(float32_t* pLeft, float32_t* pRight, uint32_t numSamples) override;

private:
	float32_t gain_;
	float32_t threshold_;
	float32_t mix_;

	void applyHardClipping(float32_t& signal, float32_t threshold) {
		if (signal >  threshold) signal =  threshold;
		if (signal < -threshold) signal = -threshold;
	}
};
