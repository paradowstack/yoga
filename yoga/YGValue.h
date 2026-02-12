/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdbool.h>

#include <yoga/YGCalc.h>
#include <yoga/YGEnums.h>
#include <yoga/YGMacros.h>

/**
 * Float value to represent "undefined" in style values.
 */
#ifdef __cplusplus
#include <limits>
constexpr float YGUndefined = std::numeric_limits<float>::quiet_NaN();
#else
#include <math.h>
#define YGUndefined NAN
#endif

YG_EXTERN_C_BEGIN

/**
 * Structure used to represent a dimension in a style.
 *
 * BREAKING CHANGE: `value` is now a tagged union. For Point/Percent units use
 * `value.scalar`; for Calc units use `value.calc`. Previously this was a plain
 * float field.
 */
typedef struct YGValue {
  union Value {
    float scalar;
    YGCalc calc;
  } value;
  YGUnit unit;
} YGValue;

/**
 * Constant for a dimension of "auto".
 */
YG_EXPORT extern const YGValue YGValueAuto;

/**
 * Constant for a dimension which is not defined.
 */
YG_EXPORT extern const YGValue YGValueUndefined;

/**
 * Constant for a dimension that is zero-length.
 */
YG_EXPORT extern const YGValue YGValueZero;

/**
 * Whether a dimension represented as a float is defined.
 */
YG_EXPORT bool YGFloatIsUndefined(float value);

YG_EXTERN_C_END

// Equality operators for comparison of YGValue in C++
#ifdef __cplusplus
inline bool operator==(const YGValue& lhs, const YGValue& rhs) {
  if (lhs.unit != rhs.unit) {
    return false;
  }

  switch (lhs.unit) {
    case YGUnitUndefined:
    case YGUnitAuto:
    case YGUnitFitContent:
    case YGUnitMaxContent:
    case YGUnitStretch:
      return true;
    case YGUnitPoint:
    case YGUnitPercent:
      return lhs.value.scalar == rhs.value.scalar;
    case YGUnitCalc:
      return lhs.value.calc.px == rhs.value.calc.px &&
          lhs.value.calc.percent == rhs.value.calc.percent &&
          lhs.value.calc.vw == rhs.value.calc.vw &&
          lhs.value.calc.vh == rhs.value.calc.vh;
    default:
      return false;
  }
}

inline bool operator!=(const YGValue& lhs, const YGValue& rhs) {
  return !(lhs == rhs);
}

inline YGValue operator-(const YGValue& value) {
  if (value.unit == YGUnitCalc) {
    return {
        .value =
            {.calc =
                 {-value.value.calc.px,
                  -value.value.calc.percent,
                  -value.value.calc.vw,
                  -value.value.calc.vh}},
        .unit = YGUnitCalc};
  }
  return {.value = {.scalar = -value.value.scalar}, .unit = value.unit};
}

#endif
