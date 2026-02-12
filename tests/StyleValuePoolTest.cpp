/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>
#include <yoga/style/StyleValuePool.h>

namespace facebook::yoga {

TEST(StyleValuePool, undefined_at_init) {
  StyleValuePool pool;
  StyleValueHandle handle;

  EXPECT_TRUE(handle.isUndefined());
  EXPECT_FALSE(handle.isDefined());
  EXPECT_EQ(pool.getLength(handle), StyleLength::undefined());
  EXPECT_EQ(pool.getNumber(handle), FloatOptional{});
}

TEST(StyleValuePool, auto_at_init) {
  StyleValuePool pool;
  auto handle = StyleValueHandle::ofAuto();

  EXPECT_TRUE(handle.isAuto());
  EXPECT_EQ(pool.getLength(handle), StyleLength::ofAuto());
}

TEST(StyleValuePool, store_small_int_points) {
  StyleValuePool pool;
  StyleValueHandle handle;

  pool.store(handle, StyleLength::points(10));

  EXPECT_EQ(pool.getLength(handle), StyleLength::points(10));
}

TEST(StyleValuePool, store_small_negative_int_points) {
  StyleValuePool pool;
  StyleValueHandle handle;

  pool.store(handle, StyleLength::points(-10));

  EXPECT_EQ(pool.getLength(handle), StyleLength::points(-10));
}

TEST(StyleValuePool, store_small_int_percent) {
  StyleValuePool pool;
  StyleValueHandle handle;

  pool.store(handle, StyleLength::percent(10));

  EXPECT_EQ(pool.getLength(handle), StyleLength::percent(10));
}

TEST(StyleValuePool, store_large_int_percent) {
  StyleValuePool pool;
  StyleValueHandle handle;

  pool.store(handle, StyleLength::percent(262144));

  EXPECT_EQ(pool.getLength(handle), StyleLength::percent(262144));
}

TEST(StyleValuePool, store_large_int_after_small_int) {
  StyleValuePool pool;
  StyleValueHandle handle;

  pool.store(handle, StyleLength::percent(10));
  pool.store(handle, StyleLength::percent(262144));

  EXPECT_EQ(pool.getLength(handle), StyleLength::percent(262144));
}

TEST(StyleValuePool, store_small_int_after_large_int) {
  StyleValuePool pool;
  StyleValueHandle handle;

  pool.store(handle, StyleLength::percent(262144));
  pool.store(handle, StyleLength::percent(10));

  EXPECT_EQ(pool.getLength(handle), StyleLength::percent(10));
}

TEST(StyleValuePool, store_small_int_number) {
  StyleValuePool pool;
  StyleValueHandle handle;

  pool.store(handle, FloatOptional{10.0f});

  EXPECT_EQ(pool.getNumber(handle), FloatOptional{10.0f});
}

TEST(StyleValuePool, store_undefined) {
  StyleValuePool pool;
  StyleValueHandle handle;

  pool.store(handle, StyleLength::undefined());

  EXPECT_TRUE(handle.isUndefined());
  EXPECT_FALSE(handle.isDefined());
  EXPECT_EQ(pool.getLength(handle), StyleLength::undefined());
}

TEST(StyleValuePool, store_undefined_after_small_int) {
  StyleValuePool pool;
  StyleValueHandle handle;

  pool.store(handle, StyleLength::points(10));
  pool.store(handle, StyleLength::undefined());

  EXPECT_TRUE(handle.isUndefined());
  EXPECT_FALSE(handle.isDefined());
  EXPECT_EQ(pool.getLength(handle), StyleLength::undefined());
}

TEST(StyleValuePool, store_undefined_after_large_int) {
  StyleValuePool pool;
  StyleValueHandle handle;

  pool.store(handle, StyleLength::points(262144));
  pool.store(handle, StyleLength::undefined());

  EXPECT_TRUE(handle.isUndefined());
  EXPECT_FALSE(handle.isDefined());
  EXPECT_EQ(pool.getLength(handle), StyleLength::undefined());
}

TEST(StyleValuePool, store_keywords) {
  StyleValuePool pool;
  StyleValueHandle handleMaxContent;
  StyleValueHandle handleFitContent;
  StyleValueHandle handleStretch;

  pool.store(handleMaxContent, StyleSizeLength::ofMaxContent());
  pool.store(handleFitContent, StyleSizeLength::ofFitContent());
  pool.store(handleStretch, StyleSizeLength::ofStretch());

  EXPECT_EQ(pool.getSize(handleMaxContent), StyleSizeLength::ofMaxContent());
  EXPECT_EQ(pool.getSize(handleFitContent), StyleSizeLength::ofFitContent());
  EXPECT_EQ(pool.getSize(handleStretch), StyleSizeLength::ofStretch());
}

// Calc tests
TEST(StyleValuePool, store_calc_length) {
  StyleValuePool pool;
  StyleValueHandle handle;
  StyleCalcLength calc{
      FloatOptional{10.0f},
      FloatOptional{20.0f},
      FloatOptional{5.0f},
      FloatOptional{3.0f}};

  pool.store(handle, StyleSizeLength::calc(calc));

  EXPECT_TRUE(handle.isCalc());
  auto retrieved = pool.getLength(handle).calcValue();
  EXPECT_EQ(retrieved.px(), 10.0f);
  EXPECT_EQ(retrieved.percent(), 20.0f);
  EXPECT_EQ(retrieved.vw(), 5.0f);
  EXPECT_EQ(retrieved.vh(), 3.0f);
}

TEST(StyleValuePool, store_calc_from_points) {
  StyleValuePool pool;
  StyleValueHandle handle;
  auto calc = StyleCalcLength{
      FloatOptional{50.0f},
      FloatOptional{0.0f},
      FloatOptional{0.0f},
      FloatOptional{0.0f}};

  pool.store(handle, StyleSizeLength::calc(calc));

  auto retrieved = pool.getLength(handle).calcValue();
  EXPECT_EQ(retrieved.px(), 50.0f);
  EXPECT_EQ(retrieved.percent(), 0.0f);
  EXPECT_EQ(retrieved.vw(), 0.0f);
  EXPECT_EQ(retrieved.vh(), 0.0f);
}

TEST(StyleValuePool, store_calc_from_percent) {
  StyleValuePool pool;
  StyleValueHandle handle;
  auto calc = StyleCalcLength{
      FloatOptional{0.0f},
      FloatOptional{25.0f},
      FloatOptional{0.0f},
      FloatOptional{0.0f}};

  pool.store(handle, StyleSizeLength::calc(calc));

  auto retrieved = pool.getLength(handle).calcValue();
  EXPECT_EQ(retrieved.px(), 0.0f);
  EXPECT_EQ(retrieved.percent(), 25.0f);
  EXPECT_EQ(retrieved.vw(), 0.0f);
  EXPECT_EQ(retrieved.vh(), 0.0f);
}

TEST(StyleValuePool, store_calc_from_vw) {
  StyleValuePool pool;
  StyleValueHandle handle;
  auto calc = StyleCalcLength{
      FloatOptional{0.0f},
      FloatOptional{0.0f},
      FloatOptional{10.0f},
      FloatOptional{0.0f}};

  pool.store(handle, StyleSizeLength::calc(calc));

  auto retrieved = pool.getLength(handle).calcValue();
  EXPECT_EQ(retrieved.px(), 0.0f);
  EXPECT_EQ(retrieved.percent(), 0.0f);
  EXPECT_EQ(retrieved.vw(), 10.0f);
  EXPECT_EQ(retrieved.vh(), 0.0f);
}

TEST(StyleValuePool, store_calc_from_vh) {
  StyleValuePool pool;
  StyleValueHandle handle;
  auto calc = StyleCalcLength{
      FloatOptional{0.0f},
      FloatOptional{0.0f},
      FloatOptional{0.0f},
      FloatOptional{15.0f}};

  pool.store(handle, StyleSizeLength::calc(calc));

  auto retrieved = pool.getLength(handle).calcValue();
  EXPECT_EQ(retrieved.px(), 0.0f);
  EXPECT_EQ(retrieved.percent(), 0.0f);
  EXPECT_EQ(retrieved.vw(), 0.0f);
  EXPECT_EQ(retrieved.vh(), 15.0f);
}

TEST(StyleValuePool, replace_calc_with_calc) {
  StyleValuePool pool;
  StyleValueHandle handle;
  StyleCalcLength calc1{
      FloatOptional{10.0f},
      FloatOptional{20.0f},
      FloatOptional{5.0f},
      FloatOptional{3.0f}};
  StyleCalcLength calc2{
      FloatOptional{50.0f},
      FloatOptional{30.0f},
      FloatOptional{10.0f},
      FloatOptional{8.0f}};

  pool.store(handle, StyleSizeLength::calc(calc1));
  pool.store(handle, StyleSizeLength::calc(calc2));

  auto retrieved = pool.getLength(handle).calcValue();
  EXPECT_EQ(retrieved.px(), 50.0f);
  EXPECT_EQ(retrieved.percent(), 30.0f);
  EXPECT_EQ(retrieved.vw(), 10.0f);
  EXPECT_EQ(retrieved.vh(), 8.0f);
}

TEST(StyleValuePool, replace_points_with_calc) {
  StyleValuePool pool;
  StyleValueHandle handle;

  pool.store(handle, StyleLength::points(100));
  pool.store(
      handle,
      StyleSizeLength::calc(
          StyleCalcLength{
              FloatOptional{10.0f},
              FloatOptional{20.0f},
              FloatOptional{5.0f},
              FloatOptional{3.0f}}));

  EXPECT_TRUE(handle.isCalc());
  auto retrieved = pool.getLength(handle).calcValue();
  EXPECT_EQ(retrieved.px(), 10.0f);
  EXPECT_EQ(retrieved.percent(), 20.0f);
}

TEST(StyleValuePool, replace_calc_with_points) {
  StyleValuePool pool;
  StyleValueHandle handle;

  pool.store(
      handle,
      StyleSizeLength::calc(
          StyleCalcLength{
              FloatOptional{10.0f},
              FloatOptional{20.0f},
              FloatOptional{5.0f},
              FloatOptional{3.0f}}));
  pool.store(handle, StyleLength::points(100));

  EXPECT_FALSE(handle.isCalc());
  EXPECT_EQ(pool.getLength(handle), StyleLength::points(100));
}

TEST(StyleValuePool, get_length_with_calc) {
  StyleValuePool pool;
  StyleValueHandle handle;
  auto calc = StyleCalcLength{
      FloatOptional{50.0f},
      FloatOptional{0.0f},
      FloatOptional{0.0f},
      FloatOptional{0.0f}};

  pool.store(handle, StyleLength::calc(calc));

  auto length = pool.getLength(handle);
  EXPECT_TRUE(length.isCalc());
  auto retrieved = length.calcValue();
  EXPECT_EQ(retrieved.px(), 50.0f);
}

TEST(StyleValuePool, get_size_with_calc) {
  StyleValuePool pool;
  StyleValueHandle handle;
  auto calc = StyleCalcLength{
      FloatOptional{10.0f},
      FloatOptional{20.0f},
      FloatOptional{5.0f},
      FloatOptional{3.0f}};

  pool.store(handle, StyleSizeLength::calc(calc));

  auto size = pool.getSize(handle);
  EXPECT_TRUE(size.isCalc());
  auto retrieved = size.calcValue();
  EXPECT_EQ(retrieved.px(), 10.0f);
  EXPECT_EQ(retrieved.percent(), 20.0f);
}

TEST(StyleValuePool, resolve_length_calc) {
  StyleValuePool pool;
  StyleValueHandle handle;
  StyleCalcLength calc{
      FloatOptional{10.0f},
      FloatOptional{20.0f},
      FloatOptional{0.0f},
      FloatOptional{0.0f}};
  pool.store(handle, StyleSizeLength::calc(calc));

  auto result = pool.resolveLength(handle, 100.0f, 0.0f, 0.0f);
  EXPECT_TRUE(result.isDefined());
  EXPECT_EQ(result.unwrap(), 30.0f); // 10px + 20% of 100
}

TEST(StyleValuePool, resolve_length_calc_with_viewport) {
  StyleValuePool pool;
  StyleValueHandle handle;
  StyleCalcLength calc{
      FloatOptional{10.0f},
      FloatOptional{0.0f},
      FloatOptional{5.0f},
      FloatOptional{10.0f}};
  pool.store(handle, StyleSizeLength::calc(calc));

  auto result = pool.resolveLength(handle, 0.0f, 1000.0f, 800.0f);
  EXPECT_TRUE(result.isDefined());
  // 10px + 0% + 5vw of 1000 + 10vh of 800 = 10 + 0 + 50 + 80 = 140
  EXPECT_EQ(result.unwrap(), 140.0f);
}

TEST(StyleValuePool, resolve_size_length_calc) {
  StyleValuePool pool;
  StyleValueHandle handle;
  StyleCalcLength calc{
      FloatOptional{20.0f},
      FloatOptional{50.0f},
      FloatOptional{0.0f},
      FloatOptional{0.0f}};
  pool.store(handle, StyleSizeLength::calc(calc));

  auto result = pool.resolveSize(handle, 100.0f, 0.0f, 0.0f);
  EXPECT_TRUE(result.isDefined());
  EXPECT_EQ(result.unwrap(), 70.0f); // 20px + 50% of 100
}

TEST(StyleValuePool, resolve_size_length_calc_all_units) {
  StyleValuePool pool;
  StyleValueHandle handle;
  StyleCalcLength calc{
      FloatOptional{10.0f},
      FloatOptional{10.0f},
      FloatOptional{5.0f},
      FloatOptional{2.0f}};
  pool.store(handle, StyleSizeLength::calc(calc));

  auto result = pool.resolveSize(handle, 200.0f, 1000.0f, 800.0f);
  EXPECT_TRUE(result.isDefined());
  // 10px + 10% of 200 + 5% of 1000 + 2% of 800
  // = 10 + 20 + 50 + 16 = 96
  EXPECT_EQ(result.unwrap(), 96.0f);
}

TEST(StyleValuePool, multiple_calc_handles) {
  StyleValuePool pool;
  StyleValueHandle handle1, handle2, handle3;

  pool.store(
      handle1,
      StyleSizeLength::calc(
          StyleCalcLength{
              FloatOptional{10.0f},
              FloatOptional{0.0f},
              FloatOptional{0.0f},
              FloatOptional{0.0f}}));
  pool.store(
      handle2,
      StyleSizeLength::calc(
          StyleCalcLength{
              FloatOptional{0.0f},
              FloatOptional{20.0f},
              FloatOptional{0.0f},
              FloatOptional{0.0f}}));
  pool.store(
      handle3,
      StyleSizeLength::calc(
          StyleCalcLength{
              FloatOptional{0.0f},
              FloatOptional{0.0f},
              FloatOptional{5.0f},
              FloatOptional{0.0f}}));

  EXPECT_EQ(pool.getLength(handle1).calcValue().px(), 10.0f);
  EXPECT_EQ(pool.getLength(handle2).calcValue().percent(), 20.0f);
  EXPECT_EQ(pool.getLength(handle3).calcValue().vw(), 5.0f);
}

TEST(StyleValuePool, calc_with_zero_values) {
  StyleValuePool pool;
  StyleValueHandle handle;
  StyleCalcLength calc{
      FloatOptional{0.0f},
      FloatOptional{0.0f},
      FloatOptional{0.0f},
      FloatOptional{0.0f}};

  pool.store(handle, StyleSizeLength::calc(calc));

  auto retrieved = pool.getLength(handle).calcValue();
  auto result = retrieved.resolve(100.0f, 1000.0f, 800.0f);
  EXPECT_TRUE(result.isDefined());
  EXPECT_EQ(result.unwrap(), 0.0f);
}

TEST(StyleValuePool, calc_with_negative_values) {
  StyleValuePool pool;
  StyleValueHandle handle;
  StyleCalcLength calc{
      FloatOptional{-10.0f},
      FloatOptional{-20.0f},
      FloatOptional{-5.0f},
      FloatOptional{-2.0f}};

  pool.store(handle, StyleSizeLength::calc(calc));

  auto result = pool.resolveLength(handle, 100.0f, 1000.0f, 800.0f);
  EXPECT_TRUE(result.isDefined());
  // -10 + -20 + -50 + -16 = -96
  EXPECT_EQ(result.unwrap(), -96.0f);
}

} // namespace facebook::yoga
