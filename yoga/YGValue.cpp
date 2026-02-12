/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <yoga/YGValue.h>
#include <yoga/numeric/Comparison.h>

using namespace facebook;
using namespace facebook::yoga;

const YGValue YGValueZero = {.value = {.scalar = 0}, .unit = YGUnitPoint};
const YGValue YGValueUndefined = {
    .value = {.scalar = YGUndefined},
    .unit = YGUnitUndefined};
const YGValue YGValueAuto = {
    .value = {.scalar = YGUndefined},
    .unit = YGUnitAuto};

bool YGFloatIsUndefined(const float value) {
  return yoga::isUndefined(value);
}
