/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>
#include <yoga/style/StyleCalcLength.h>

namespace facebook::yoga {

TEST(StyleCalcLength, default_initialization) {
  StyleCalcLength calc;
  EXPECT_TRUE(calc.px().isUndefined());
  EXPECT_TRUE(calc.percent().isUndefined());
  EXPECT_TRUE(calc.vw().isUndefined());
  EXPECT_TRUE(calc.vh().isUndefined());
  EXPECT_TRUE(calc.isUndefined());
}

TEST(StyleCalcLength, direct_initialization) {
  StyleCalcLength calc{
      FloatOptional{10.0f},
      FloatOptional{20.0f},
      FloatOptional{5.0f},
      FloatOptional{3.0f}};
  EXPECT_EQ(calc.px(), 10.0f);
  EXPECT_EQ(calc.percent(), 20.0f);
  EXPECT_EQ(calc.vw(), 5.0f);
  EXPECT_EQ(calc.vh(), 3.0f);
}

TEST(StyleCalcLength, isPointsOnly) {
  EXPECT_TRUE(StyleCalcLength::points(100.0f).isPointsOnly());
  EXPECT_FALSE(StyleCalcLength::percent(50.0f).isPointsOnly());
  EXPECT_FALSE(StyleCalcLength::vw(10.0f).isPointsOnly());
  EXPECT_FALSE((StyleCalcLength{
                    FloatOptional{10.0f},
                    FloatOptional{5.0f},
                    FloatOptional{0.0f},
                    FloatOptional{0.0f}})
                   .isPointsOnly());
}

TEST(StyleCalcLength, isPercentOnly) {
  EXPECT_FALSE(StyleCalcLength::points(100.0f).isPercentOnly());
  EXPECT_TRUE(StyleCalcLength::percent(50.0f).isPercentOnly());
  EXPECT_FALSE(StyleCalcLength::vw(10.0f).isPercentOnly());
  EXPECT_FALSE((StyleCalcLength{
                    FloatOptional{0.0f},
                    FloatOptional{50.0f},
                    FloatOptional{5.0f},
                    FloatOptional{0.0f}})
                   .isPercentOnly());
}

TEST(StyleCalcLength, resolve) {
  // calc(10px + 20% + 5vw + 2.5vh)
  StyleCalcLength calc{
      FloatOptional{10.0f},
      FloatOptional{20.0f},
      FloatOptional{5.0f},
      FloatOptional{2.5f}};
  auto result = calc.resolve(100.0f, 1000.0f, 800.0f);
  EXPECT_TRUE(result.isDefined());
  // 10 + 20 + 50 + 20 = 100
  EXPECT_EQ(result.unwrap(), 100.0f);
}

TEST(StyleCalcLength, equality) {
  StyleCalcLength calc1{
      FloatOptional{10.0f},
      FloatOptional{20.0f},
      FloatOptional{5.0f},
      FloatOptional{3.0f}};
  StyleCalcLength calc2{
      FloatOptional{10.0f},
      FloatOptional{20.0f},
      FloatOptional{5.0f},
      FloatOptional{3.0f}};
  StyleCalcLength calc3{
      FloatOptional{10.0f},
      FloatOptional{20.0f},
      FloatOptional{5.0f},
      FloatOptional{4.0f}};

  EXPECT_TRUE(calc1 == calc2);
  EXPECT_FALSE(calc1 == calc3);
}

TEST(StyleCalcLength, inexactEquals) {
  StyleCalcLength calc1{
      FloatOptional{10.0f},
      FloatOptional{20.0f},
      FloatOptional{5.0f},
      FloatOptional{3.0f}};
  StyleCalcLength calc2{
      FloatOptional{10.0f},
      FloatOptional{20.0f},
      FloatOptional{5.0f},
      FloatOptional{3.0f}};
  StyleCalcLength calc3{
      FloatOptional{10.0f},
      FloatOptional{20.0f},
      FloatOptional{5.0f},
      FloatOptional{4.0f}};

  EXPECT_TRUE(inexactEquals(calc1, calc2));
  EXPECT_FALSE(inexactEquals(calc1, calc3));
}

TEST(StyleCalcLength, ygvalue_conversion) {
  // Undefined → YGValueUndefined
  auto undefinedValue = static_cast<YGValue>(StyleCalcLength{});
  EXPECT_EQ(undefinedValue.unit, YGUnitUndefined);

  // Points only → YGUnitPoint
  auto pointsValue = static_cast<YGValue>(StyleCalcLength::points(50.0f));
  EXPECT_EQ(pointsValue.unit, YGUnitPoint);
  EXPECT_EQ(pointsValue.value.scalar, 50.0f);

  // Percent only → YGUnitPercent
  auto percentValue = static_cast<YGValue>(StyleCalcLength::percent(25.0f));
  EXPECT_EQ(percentValue.unit, YGUnitPercent);
  EXPECT_EQ(percentValue.value.scalar, 25.0f);

  // Mixed → YGUnitCalc
  StyleCalcLength mixed{
      FloatOptional{10.0f},
      FloatOptional{20.0f},
      FloatOptional{5.0f},
      FloatOptional{3.0f}};
  auto calcValue = static_cast<YGValue>(mixed);
  EXPECT_EQ(calcValue.unit, YGUnitCalc);
  EXPECT_EQ(calcValue.value.calc.px, 10.0f);
  EXPECT_EQ(calcValue.value.calc.percent, 20.0f);
  EXPECT_EQ(calcValue.value.calc.vw, 5.0f);
  EXPECT_EQ(calcValue.value.calc.vh, 3.0f);
}

} // namespace facebook::yoga
