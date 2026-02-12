/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <yoga/style/StyleValuePool.h>

namespace facebook::yoga {

FloatOptional StyleValuePool::resolveCalc(
    StyleValueHandle handle,
    float referenceLength,
    float viewportWidth,
    float viewportHeight) const {
  return getCalc(handle).resolve(
      referenceLength, viewportWidth, viewportHeight);
}

} // namespace facebook::yoga
