/*
 * effector.hpp
 *
 *  Created on: Mar 19, 2026
 *      Author: RuK3k
 */

#pragma once
#include "arm_math.h"

class Effector {
public:
  virtual ~Effector() = default;

  virtual void process(float32_t *pLeft, float32_t *pRight, uint32_t numFrames) = 0;
};
