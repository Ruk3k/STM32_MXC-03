/*
 * audio_constants.hpp
 *
 *  Created on: Mar 19, 2026
 *      Author: Ruk3k
 */

#pragma once
#include "arm_math.h"

namespace Constants {
	namespace Math {
		static constexpr float32_t kPi{ 3.14159265358979323846f };

		static constexpr float32_t kInt16ToFloat{ 1.0f / 32768.0f };
		static constexpr float32_t kInt32ToFloat{ 1.0f / 2147483648.0f };
		static constexpr float32_t kFloatToInt32{ 2147483647.0f };
	}

	namespace Config {
		static constexpr float32_t kDefaultSampleRate{ 48000.0f };
	}
}
