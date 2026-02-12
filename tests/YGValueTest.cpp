/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include <yoga/YGValue.h>
#include <yoga/Yoga.h>
#include <yoga/style/StyleCalcLength.h>
#include <yoga/style/StyleLength.h>
#include <yoga/style/StyleSizeLength.h>

using namespace facebook::yoga;

TEST(YGValue, supports_equality) {
  ASSERT_EQ(
      (YGValue{.value = {.scalar = 12.5f}, .unit = YGUnitPercent}),
      (YGValue{.value = {.scalar = 12.5f}, .unit = YGUnitPercent}));
  ASSERT_NE(
      (YGValue{.value = {.scalar = 12.5f}, .unit = YGUnitPercent}),
      (YGValue{.value = {.scalar = 56.7f}, .unit = YGUnitPercent}));
  ASSERT_NE(
      (YGValue{.value = {.scalar = 12.5f}, .unit = YGUnitPercent}),
      (YGValue{.value = {.scalar = 12.5f}, .unit = YGUnitPoint}));
  ASSERT_NE(
      (YGValue{.value = {.scalar = 12.5f}, .unit = YGUnitPercent}),
      (YGValue{.value = {.scalar = 12.5f}, .unit = YGUnitAuto}));
  ASSERT_NE(
      (YGValue{.value = {.scalar = 12.5f}, .unit = YGUnitPercent}),
      (YGValue{.value = {.scalar = 12.5f}, .unit = YGUnitUndefined}));

  ASSERT_EQ(
      (YGValue{.value = {.scalar = 12.5f}, .unit = YGUnitUndefined}),
      (YGValue{.value = {.scalar = 12.5f}, .unit = YGUnitUndefined}));
  ASSERT_EQ(
      (YGValue{.value = {.scalar = 0.0f}, .unit = YGUnitAuto}),
      (YGValue{.value = {.scalar = -1.0f}, .unit = YGUnitAuto}));
}

TEST(YGValue, calc_points_only_converts_to_points) {
  auto calc = StyleCalcLength::points(50.0f);
  auto length = StyleLength::calc(calc);
  auto value = static_cast<YGValue>(length);

  EXPECT_EQ(value.unit, YGUnitPoint);
  EXPECT_EQ(value.value.scalar, 50.0f);
}

TEST(YGValue, calc_percent_only_converts_to_percent) {
  auto calc = StyleCalcLength::percent(25.0f);
  auto length = StyleLength::calc(calc);
  auto value = static_cast<YGValue>(length);

  EXPECT_EQ(value.unit, YGUnitPercent);
  EXPECT_EQ(value.value.scalar, 25.0f);
}

TEST(YGValue, calc_mixed_units_converts_to_calc) {
  StyleCalcLength calc{
      FloatOptional{10.0f},
      FloatOptional{20.0f},
      FloatOptional{0.0f},
      FloatOptional{0.0f}};
  auto length = StyleLength::calc(calc);
  auto value = static_cast<YGValue>(length);

  EXPECT_EQ(value.unit, YGUnitCalc);
  EXPECT_EQ(value.value.calc.px, 10.0f);
  EXPECT_EQ(value.value.calc.percent, 20.0f);
  EXPECT_EQ(value.value.calc.vw, 0.0f);
  EXPECT_EQ(value.value.calc.vh, 0.0f);
}

TEST(YGValue, calc_all_units_converts_to_calc) {
  auto calc = StyleCalcLength{
      FloatOptional{10.0f},
      FloatOptional{20.0f},
      FloatOptional{5.0f},
      FloatOptional{15.0f}};
  auto length = StyleLength::calc(calc);
  auto value = static_cast<YGValue>(length);

  EXPECT_EQ(value.unit, YGUnitCalc);
  EXPECT_EQ(value.value.calc.px, 10.0f);
  EXPECT_EQ(value.value.calc.percent, 20.0f);
  EXPECT_EQ(value.value.calc.vw, 5.0f);
  EXPECT_EQ(value.value.calc.vh, 15.0f);
}

TEST(YGValue, size_length_calc_mixed_units) {
  auto calc = StyleCalcLength{
      FloatOptional{50.0f},
      FloatOptional{25.0f},
      FloatOptional{10.0f},
      FloatOptional{5.0f}};
  auto size = StyleSizeLength::calc(calc);
  auto value = static_cast<YGValue>(size);

  EXPECT_EQ(value.unit, YGUnitCalc);
  EXPECT_EQ(value.value.calc.px, 50.0f);
  EXPECT_EQ(value.value.calc.percent, 25.0f);
  EXPECT_EQ(value.value.calc.vw, 10.0f);
  EXPECT_EQ(value.value.calc.vh, 5.0f);
}

TEST(YGValue, calc_value_negation) {
  auto calc = StyleCalcLength{
      FloatOptional{10.0f},
      FloatOptional{20.0f},
      FloatOptional{5.0f},
      FloatOptional{15.0f}};
  auto length = StyleLength::calc(calc);
  auto value = static_cast<YGValue>(length);
  auto negated = -value;

  EXPECT_EQ(negated.unit, YGUnitCalc);
  EXPECT_EQ(negated.value.calc.px, -10.0f);
  EXPECT_EQ(negated.value.calc.percent, -20.0f);
  EXPECT_EQ(negated.value.calc.vw, -5.0f);
  EXPECT_EQ(negated.value.calc.vh, -15.0f);
}

TEST(YGValue, set_width_calc) {
  YGNodeRef node = YGNodeNew();
  YGCalc calc = {10.0f, 20.0f, 5.0f, 3.0f};
  YGNodeStyleSetWidthCalc(node, calc);

  YGValue value = YGNodeStyleGetWidth(node);
  EXPECT_EQ(value.unit, YGUnitCalc);
  EXPECT_EQ(value.value.calc.px, 10.0f);
  EXPECT_EQ(value.value.calc.percent, 20.0f);
  EXPECT_EQ(value.value.calc.vw, 5.0f);
  EXPECT_EQ(value.value.calc.vh, 3.0f);

  YGNodeFree(node);
}

TEST(YGValue, set_margin_calc) {
  YGNodeRef node = YGNodeNew();
  YGCalc calc = {5.0f, 10.0f, 0.0f, 0.0f};
  YGNodeStyleSetMarginCalc(node, YGEdgeLeft, calc);

  YGValue value = YGNodeStyleGetMargin(node, YGEdgeLeft);
  EXPECT_EQ(value.unit, YGUnitCalc);
  EXPECT_EQ(value.value.calc.px, 5.0f);
  EXPECT_EQ(value.value.calc.percent, 10.0f);

  YGNodeFree(node);
}

TEST(YGValue, set_gap_calc) {
  YGNodeRef node = YGNodeNew();
  YGCalc calc = {8.0f, 0.0f, 2.0f, 0.0f};
  YGNodeStyleSetGapCalc(node, YGGutterColumn, calc);

  YGValue value = YGNodeStyleGetGap(node, YGGutterColumn);
  EXPECT_EQ(value.unit, YGUnitCalc);
  EXPECT_EQ(value.value.calc.px, 8.0f);
  EXPECT_EQ(value.value.calc.vw, 2.0f);

  YGNodeFree(node);
}

TEST(YGValue, set_calc_marks_node_dirty) {
  YGNodeRef root = YGNodeNew();
  YGNodeStyleSetWidth(root, 100);
  YGNodeStyleSetHeight(root, 100);
  YGNodeCalculateLayout(root, YGUndefined, YGUndefined, YGDirectionLTR);

  EXPECT_FALSE(YGNodeIsDirty(root));

  YGCalc calc = {10.0f, 20.0f, 0.0f, 0.0f};
  YGNodeStyleSetWidthCalc(root, calc);

  EXPECT_TRUE(YGNodeIsDirty(root));

  YGNodeFree(root);
}
