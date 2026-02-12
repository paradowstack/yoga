/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>
#include <yoga/style/Style.h>

namespace facebook::yoga {

TEST(Style, computed_padding_is_floored) {
  yoga::Style style;
  style.setPadding(Edge::All, StyleLength::points(-1.0f));
  auto paddingStart = style.computeInlineStartPadding(
      FlexDirection::Row,
      Direction::LTR,
      0.0f /*widthSize*/,
      0.0f /*viewportWidth*/,
      0.0f /*viewportHeight*/);
  ASSERT_EQ(paddingStart, 0.0f);
}

TEST(Style, computed_border_is_floored) {
  yoga::Style style;
  style.setBorder(Edge::All, StyleLength::points(-1.0f));
  auto borderStart = style.computeInlineStartBorder(
      FlexDirection::Row,
      Direction::LTR,
      0.0f /*viewportWidth*/,
      0.0f /*viewportHeight*/);
  ASSERT_EQ(borderStart, 0.0f);
}

TEST(Style, computed_gap_is_floored) {
  yoga::Style style;
  style.setGap(Gutter::Column, StyleLength::points(-1.0f));
  auto gapBetweenColumns = style.computeGapForAxis(
      FlexDirection::Row, 0.0, 0.0f /*viewportWidth*/, 0.0f /*viewportHeight*/);
  ASSERT_EQ(gapBetweenColumns, 0.0f);
}

TEST(Style, computed_margin_is_not_floored) {
  yoga::Style style;
  style.setMargin(Edge::All, StyleLength::points(-1.0f));
  auto marginStart = style.computeInlineStartMargin(
      FlexDirection::Row,
      Direction::LTR,
      0.0f /*widthSize*/,
      0.0f /*viewportWidth*/,
      0.0f /*viewportHeight*/);
  ASSERT_EQ(marginStart, -1.0f);
}

// Calc resolution tests
TEST(Style, resolve_dimension_with_calc_points) {
  yoga::Style style;
  auto calc = StyleCalcLength{
      FloatOptional{100.0f},
      FloatOptional{0.0f},
      FloatOptional{0.0f},
      FloatOptional{0.0f}};
  style.setMinDimension(Dimension::Width, StyleSizeLength::calc(calc));

  auto resolved = style.resolvedMinDimension(
      style.direction(), Dimension::Width, 200.0f, 0.0f, 0.0f, 0.0f);
  EXPECT_TRUE(resolved.isDefined());
  EXPECT_EQ(resolved.unwrap(), 100.0f);
}

TEST(Style, resolve_dimension_with_calc_percent) {
  yoga::Style style;
  auto calc = StyleCalcLength{
      FloatOptional{0.0f},
      FloatOptional{50.0f},
      FloatOptional{0.0f},
      FloatOptional{0.0f}};
  style.setMinDimension(Dimension::Width, StyleSizeLength::calc(calc));

  auto resolved = style.resolvedMinDimension(
      style.direction(), Dimension::Width, 200.0f, 0.0f, 0.0f, 0.0f);
  EXPECT_TRUE(resolved.isDefined());
  EXPECT_EQ(resolved.unwrap(), 100.0f); // 50% of 200
}

TEST(Style, resolve_dimension_with_calc_vw) {
  yoga::Style style;
  auto calc = StyleCalcLength{
      FloatOptional{0.0f},
      FloatOptional{0.0f},
      FloatOptional{10.0f},
      FloatOptional{0.0f}};
  style.setMinDimension(Dimension::Width, StyleSizeLength::calc(calc));
  style.setBoxSizing(BoxSizing::BorderBox);
  auto resolved = style.resolvedMinDimension(
      style.direction(), Dimension::Width, 200.0f, 0.0f, 1000.0f, 0.0f);
  EXPECT_TRUE(resolved.isDefined());
  EXPECT_EQ(resolved.unwrap(), 100.0f); // 10% of 1000
}

TEST(Style, resolve_dimension_with_calc_vh) {
  yoga::Style style;
  auto calc = StyleCalcLength{
      FloatOptional{0.0f},
      FloatOptional{0.0f},
      FloatOptional{0.0f},
      FloatOptional{5.0f}};
  style.setMinDimension(Dimension::Height, StyleSizeLength::calc(calc));

  auto resolved = style.resolvedMinDimension(
      style.direction(), Dimension::Height, 200.0f, 0.0f, 0.0f, 800.0f);
  EXPECT_TRUE(resolved.isDefined());
  EXPECT_EQ(resolved.unwrap(), 40.0f); // 5% of 800
}

TEST(Style, resolve_dimension_with_calc_mixed) {
  yoga::Style style;
  StyleCalcLength calc{
      FloatOptional{20.0f},
      FloatOptional{10.0f},
      FloatOptional{5.0f},
      FloatOptional{2.0f}};
  style.setMinDimension(Dimension::Width, StyleSizeLength::calc(calc));

  auto resolved = style.resolvedMinDimension(
      style.direction(), Dimension::Width, 300.0f, 0.0f, 1000.0f, 800.0f);
  EXPECT_TRUE(resolved.isDefined());
  // 20px + 10% of 300 + 5% of 1000 + 2% of 800
  // = 20 + 30 + 50 + 16 = 116
  EXPECT_EQ(resolved.unwrap(), 116.0f);
}

TEST(Style, compute_margin_with_calc) {
  yoga::Style style;
  StyleCalcLength calc{
      FloatOptional{10.0f},
      FloatOptional{20.0f},
      FloatOptional{0.0f},
      FloatOptional{0.0f}};
  style.setMargin(Edge::All, StyleLength::calc(calc));

  auto marginStart = style.computeInlineStartMargin(
      FlexDirection::Row,
      Direction::LTR,
      100.0f /*widthSize*/,
      0.0f /*viewportWidth*/,
      0.0f /*viewportHeight*/);
  EXPECT_EQ(marginStart, 30.0f); // 10px + 20% of 100
}

TEST(Style, compute_margin_with_calc_viewport_units) {
  yoga::Style style;
  StyleCalcLength calc{
      FloatOptional{5.0f},
      FloatOptional{0.0f},
      FloatOptional{10.0f},
      FloatOptional{0.0f}};
  style.setMargin(Edge::Left, StyleLength::calc(calc));

  auto marginStart = style.computeInlineStartMargin(
      FlexDirection::Row,
      Direction::LTR,
      100.0f /*widthSize*/,
      1000.0f /*viewportWidth*/,
      0.0f /*viewportHeight*/);
  EXPECT_EQ(marginStart, 105.0f); // 5px + 10% of 1000
}

TEST(Style, compute_padding_with_calc) {
  yoga::Style style;
  StyleCalcLength calc{
      FloatOptional{15.0f},
      FloatOptional{10.0f},
      FloatOptional{0.0f},
      FloatOptional{0.0f}};
  style.setPadding(Edge::All, StyleLength::calc(calc));

  auto paddingStart = style.computeInlineStartPadding(
      FlexDirection::Row,
      Direction::LTR,
      200.0f /*widthSize*/,
      0.0f /*viewportWidth*/,
      0.0f /*viewportHeight*/);
  EXPECT_EQ(paddingStart, 35.0f); // 15px + 10% of 200
}

TEST(Style, compute_padding_with_calc_is_floored) {
  yoga::Style style;
  StyleCalcLength calc{
      FloatOptional{-10.0f},
      FloatOptional{-5.0f},
      FloatOptional{0.0f},
      FloatOptional{0.0f}};
  style.setPadding(Edge::All, StyleLength::calc(calc));

  auto paddingStart = style.computeInlineStartPadding(
      FlexDirection::Row,
      Direction::LTR,
      100.0f /*widthSize*/,
      0.0f /*viewportWidth*/,
      0.0f /*viewportHeight*/);
  EXPECT_EQ(paddingStart, 0.0f); // max(0, -10 + -5) = 0
}

TEST(Style, compute_border_with_calc) {
  yoga::Style style;
  StyleCalcLength calc{
      FloatOptional{2.0f},
      FloatOptional{1.0f},
      FloatOptional{0.0f},
      FloatOptional{0.0f}};
  style.setBorder(Edge::All, StyleLength::calc(calc));

  auto borderStart = style.computeInlineStartBorder(
      FlexDirection::Row,
      Direction::LTR,
      100.0f /*viewportWidth*/,
      0.0f /*viewportHeight*/);
  // Border uses 0.0f as reference length, so percents resolve to 0
  // 2px + 1% of 0 = 2.0f
  EXPECT_EQ(borderStart, 2.0f);
}

TEST(Style, compute_gap_with_calc) {
  yoga::Style style;
  StyleCalcLength calc{
      FloatOptional{10.0f},
      FloatOptional{5.0f},
      FloatOptional{0.0f},
      FloatOptional{0.0f}};
  style.setGap(Gutter::Column, StyleLength::calc(calc));

  auto gapBetweenColumns = style.computeGapForAxis(
      FlexDirection::Row,
      200.0f,
      0.0f /*viewportWidth*/,
      0.0f /*viewportHeight*/);
  EXPECT_EQ(gapBetweenColumns, 20.0f); // 10px + 5% of 200
}

TEST(Style, compute_gap_with_calc_viewport_units) {
  yoga::Style style;
  StyleCalcLength calc{
      FloatOptional{5.0f},
      FloatOptional{0.0f},
      FloatOptional{2.0f},
      FloatOptional{1.0f}};
  style.setGap(Gutter::Row, StyleLength::calc(calc));

  auto gapBetweenRows = style.computeGapForAxis(
      FlexDirection::Column,
      200.0f,
      1000.0f /*viewportWidth*/,
      800.0f /*viewportHeight*/);
  // 5px + 2% of 1000 + 1% of 800 = 5 + 20 + 8 = 33
  EXPECT_EQ(gapBetweenRows, 33.0f);
}

TEST(Style, position_with_calc) {
  yoga::Style style;
  StyleCalcLength calc{
      FloatOptional{10.0f},
      FloatOptional{15.0f},
      FloatOptional{0.0f},
      FloatOptional{0.0f}};
  style.setPosition(Edge::Left, StyleLength::calc(calc));

  auto position = style.computeFlexStartPosition(
      FlexDirection::Row,
      Direction::LTR,
      200.0f /*ownerSize*/,
      0.0f /*viewportWidth*/,
      0.0f /*viewportHeight*/);
  EXPECT_EQ(position, 40.0f); // 10px + 15% of 200
}

TEST(Style, position_with_calc_viewport_units) {
  yoga::Style style;
  StyleCalcLength calc{
      FloatOptional{5.0f},
      FloatOptional{0.0f},
      FloatOptional{10.0f},
      FloatOptional{5.0f}};
  style.setPosition(Edge::Top, StyleLength::calc(calc));

  auto position = style.computeFlexStartPosition(
      FlexDirection::Column,
      Direction::LTR,
      200.0f /*ownerSize*/,
      1000.0f /*viewportWidth*/,
      800.0f /*viewportHeight*/);
  // 5px + 10% of 1000 + 5% of 800 = 5 + 100 + 40 = 145
  EXPECT_EQ(position, 145.0f);
}

TEST(Style, min_dimension_with_calc) {
  yoga::Style style;
  StyleCalcLength calc{
      FloatOptional{50.0f},
      FloatOptional{10.0f},
      FloatOptional{0.0f},
      FloatOptional{0.0f}};
  style.setMinDimension(Dimension::Width, StyleSizeLength::calc(calc));

  auto resolved = style.resolvedMinDimension(
      style.direction(),
      Dimension::Width,
      200.0f /*referenceLength*/,
      200.0f /*ownerWidth*/,
      0.0f /*viewportWidth*/,
      0.0f /*viewportHeight*/);
  EXPECT_TRUE(resolved.isDefined());
  EXPECT_EQ(resolved.unwrap(), 70.0f); // 50px + 10% of 200
}

TEST(Style, max_dimension_with_calc) {
  yoga::Style style;
  StyleCalcLength calc{
      FloatOptional{100.0f},
      FloatOptional{25.0f},
      FloatOptional{0.0f},
      FloatOptional{0.0f}};
  style.setMaxDimension(Dimension::Width, StyleSizeLength::calc(calc));

  auto resolved = style.resolvedMaxDimension(
      Direction::LTR,
      Dimension::Width,
      200.0f /*referenceLength*/,
      200.0f /*ownerWidth*/,
      0.0f /*viewportWidth*/,
      0.0f /*viewportHeight*/);
  EXPECT_TRUE(resolved.isDefined());
  EXPECT_EQ(resolved.unwrap(), 150.0f); // 100px + 25% of 200
}

TEST(Style, calc_with_negative_result) {
  yoga::Style style;
  StyleCalcLength calc{
      FloatOptional{-20.0f},
      FloatOptional{-30.0f},
      FloatOptional{0.0f},
      FloatOptional{0.0f}};
  style.setMargin(Edge::All, StyleLength::calc(calc));

  auto marginStart = style.computeInlineStartMargin(
      FlexDirection::Row,
      Direction::LTR,
      100.0f /*widthSize*/,
      0.0f /*viewportWidth*/,
      0.0f /*viewportHeight*/);
  EXPECT_EQ(marginStart, -50.0f); // -20px + -30% of 100
}

} // namespace facebook::yoga
