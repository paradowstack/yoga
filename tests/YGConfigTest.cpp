/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>
#include <yoga/Yoga.h>
#include <yoga/config/Config.h>
#include <yoga/node/Node.h>

#include <functional>
#include <memory>

using namespace facebook;

struct ConfigCloningTest : public ::testing::Test {
  std::unique_ptr<yoga::Config, std::function<void(yoga::Config*)>> config;
  void SetUp() override;
  void TearDown() override;

  static yoga::Node clonedNode;
  static YGNodeRef cloneNode(
      YGNodeConstRef /*unused*/,
      YGNodeConstRef /*unused*/,
      size_t /*unused*/) {
    return &clonedNode;
  }
  static YGNodeRef doNotClone(
      YGNodeConstRef /*unused*/,
      YGNodeConstRef /*unused*/,
      size_t /*unused*/) {
    return nullptr;
  }
};

TEST_F(ConfigCloningTest, uses_values_provided_by_cloning_callback) {
  config->setCloneNodeCallback(cloneNode);

  yoga::Node node{};
  yoga::Node owner{};
  auto clone = config->cloneNode(&node, &owner, 0);

  ASSERT_EQ(clone, &clonedNode);
}

TEST_F(
    ConfigCloningTest,
    falls_back_to_regular_cloning_if_callback_returns_null) {
  config->setCloneNodeCallback(doNotClone);

  yoga::Node node{};
  yoga::Node owner{};
  auto clone = config->cloneNode(&node, &owner, 0);

  ASSERT_NE(clone, nullptr);
  YGNodeFree(clone);
}

void ConfigCloningTest::SetUp() {
  config = {static_cast<yoga::Config*>(YGConfigNew()), YGConfigFree};
}

void ConfigCloningTest::TearDown() {
  config.reset();
}

yoga::Node ConfigCloningTest::clonedNode = {};

TEST(YGConfigTest, default_viewport_dimensions) {
  const YGConfigRef config = YGConfigNew();
  EXPECT_EQ(YGConfigGetViewportWidth(config), 0.0f);
  EXPECT_EQ(YGConfigGetViewportHeight(config), 0.0f);
  YGConfigFree(config);
}

TEST(YGConfigTest, set_viewport_dimensions) {
  const YGConfigRef config = YGConfigNew();
  YGConfigSetViewportWidth(config, 1920.0f);
  YGConfigSetViewportHeight(config, 1080.0f);

  EXPECT_EQ(YGConfigGetViewportWidth(config), 1920.0f);
  EXPECT_EQ(YGConfigGetViewportHeight(config), 1080.0f);
  YGConfigFree(config);
}

TEST(YGConfigTest, update_viewport_dimensions) {
  const YGConfigRef config = YGConfigNew();
  YGConfigSetViewportWidth(config, 1920.0f);
  YGConfigSetViewportHeight(config, 1080.0f);

  YGConfigSetViewportWidth(config, 1000.0f);
  YGConfigSetViewportHeight(config, 800.0f);

  EXPECT_EQ(YGConfigGetViewportWidth(config), 1000.0f);
  EXPECT_EQ(YGConfigGetViewportHeight(config), 800.0f);
  YGConfigFree(config);
}

TEST(YGConfigTest, calc_width_height_layout) {
  const YGConfigRef config = YGConfigNew();

  const YGNodeRef root = YGNodeNewWithConfig(config);
  YGNodeStyleSetPositionType(root, YGPositionTypeAbsolute);
  // width: calc(50px + 50%)  with parent width 200 => 50 + 100 = 150
  YGNodeStyleSetWidthCalc(root, {50.0f, 50.0f, 0.0f, 0.0f});
  YGNodeStyleSetHeight(root, 100);

  YGNodeRef child = YGNodeNewWithConfig(config);
  // height: calc(10px + 25%) with parent height 100 => 10 + 25 = 35
  YGNodeStyleSetWidth(child, 50);
  YGNodeStyleSetHeightCalc(child, {10.0f, 25.0f, 0.0f, 0.0f});
  YGNodeInsertChild(root, child, 0);

  YGNodeCalculateLayout(root, 200, YGUndefined, YGDirectionLTR);

  ASSERT_FLOAT_EQ(150, YGNodeLayoutGetWidth(root));
  ASSERT_FLOAT_EQ(100, YGNodeLayoutGetHeight(root));
  ASSERT_FLOAT_EQ(50, YGNodeLayoutGetWidth(child));
  ASSERT_FLOAT_EQ(35, YGNodeLayoutGetHeight(child));

  YGNodeFreeRecursive(root);
  YGConfigFree(config);
}

TEST(YGConfigTest, calc_padding_layout) {
  const YGConfigRef config = YGConfigNew();

  const YGNodeRef root = YGNodeNewWithConfig(config);
  YGNodeStyleSetPositionType(root, YGPositionTypeAbsolute);
  YGNodeStyleSetWidth(root, 200);
  YGNodeStyleSetHeight(root, 200);

  YGNodeRef child = YGNodeNewWithConfig(config);
  YGNodeStyleSetWidth(child, 200);
  YGNodeStyleSetHeight(child, 200);
  // padding-left: calc(10px + 5%) => 10 + (5% of 200) = 10 + 10 = 20
  YGNodeStyleSetPaddingCalc(child, YGEdgeLeft, {10.0f, 5.0f, 0.0f, 0.0f});
  YGNodeInsertChild(root, child, 0);

  YGNodeRef grandchild = YGNodeNewWithConfig(config);
  YGNodeStyleSetWidth(grandchild, 50);
  YGNodeStyleSetHeight(grandchild, 50);
  YGNodeInsertChild(child, grandchild, 0);

  YGNodeCalculateLayout(root, YGUndefined, YGUndefined, YGDirectionLTR);

  ASSERT_FLOAT_EQ(20, YGNodeLayoutGetLeft(grandchild));

  YGNodeFreeRecursive(root);
  YGConfigFree(config);
}

TEST(YGConfigTest, calc_gap_layout) {
  const YGConfigRef config = YGConfigNew();

  const YGNodeRef root = YGNodeNewWithConfig(config);
  YGNodeStyleSetFlexDirection(root, YGFlexDirectionRow);
  YGNodeStyleSetPositionType(root, YGPositionTypeAbsolute);
  YGNodeStyleSetWidth(root, 200);
  YGNodeStyleSetHeight(root, 100);
  // column-gap: calc(10px + 5%) => 10 + 10 = 20
  YGNodeStyleSetGapCalc(root, YGGutterColumn, {10.0f, 5.0f, 0.0f, 0.0f});

  YGNodeRef child0 = YGNodeNewWithConfig(config);
  YGNodeStyleSetWidth(child0, 50);
  YGNodeStyleSetHeight(child0, 50);
  YGNodeInsertChild(root, child0, 0);

  YGNodeRef child1 = YGNodeNewWithConfig(config);
  YGNodeStyleSetWidth(child1, 50);
  YGNodeStyleSetHeight(child1, 50);
  YGNodeInsertChild(root, child1, 1);

  YGNodeCalculateLayout(root, YGUndefined, YGUndefined, YGDirectionLTR);

  ASSERT_FLOAT_EQ(0, YGNodeLayoutGetLeft(child0));
  ASSERT_FLOAT_EQ(70, YGNodeLayoutGetLeft(child1));

  YGNodeFreeRecursive(root);
  YGConfigFree(config);
}

TEST(YGConfigTest, calc_margin_layout) {
  const YGConfigRef config = YGConfigNew();

  const YGNodeRef root = YGNodeNewWithConfig(config);
  YGNodeStyleSetPositionType(root, YGPositionTypeAbsolute);
  YGNodeStyleSetWidth(root, 200);
  YGNodeStyleSetHeight(root, 200);

  YGNodeRef child = YGNodeNewWithConfig(config);
  YGNodeStyleSetWidth(child, 50);
  YGNodeStyleSetHeight(child, 50);
  // margin-top: calc(20px + 10%) => 20 + 20 = 40
  YGNodeStyleSetMarginCalc(child, YGEdgeTop, {20.0f, 10.0f, 0.0f, 0.0f});
  YGNodeInsertChild(root, child, 0);

  YGNodeCalculateLayout(root, YGUndefined, YGUndefined, YGDirectionLTR);

  ASSERT_FLOAT_EQ(40, YGNodeLayoutGetTop(child));

  YGNodeFreeRecursive(root);
  YGConfigFree(config);
}
