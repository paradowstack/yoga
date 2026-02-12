/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>
#include <yoga/Yoga.h>

TEST(YogaTest, calc_width_height_layout) {
  YGConfigRef config = YGConfigNew();

  YGNodeRef root = YGNodeNewWithConfig(config);
  YGNodeStyleSetPositionType(root, YGPositionTypeAbsolute);
  // width: calc(50px + 50%) with available width 200 => 50 + 100 = 150
  YGNodeStyleSetWidthCalc(root, {50.0f, 50.0f, 0.0f, 0.0f});
  YGNodeStyleSetHeight(root, 100);

  YGNodeRef root_child0 = YGNodeNewWithConfig(config);
  YGNodeStyleSetWidth(root_child0, 50);
  // height: calc(10px + 25%) with parent height 100 => 10 + 25 = 35
  YGNodeStyleSetHeightCalc(root_child0, {10.0f, 25.0f, 0.0f, 0.0f});
  YGNodeInsertChild(root, root_child0, 0);

  YGNodeCalculateLayout(root, 200, YGUndefined, YGDirectionLTR);

  ASSERT_FLOAT_EQ(150, YGNodeLayoutGetWidth(root));
  ASSERT_FLOAT_EQ(100, YGNodeLayoutGetHeight(root));
  ASSERT_FLOAT_EQ(50, YGNodeLayoutGetWidth(root_child0));
  ASSERT_FLOAT_EQ(35, YGNodeLayoutGetHeight(root_child0));

  YGNodeFreeRecursive(root);
  YGConfigFree(config);
}

TEST(YogaTest, calc_padding_layout) {
  YGConfigRef config = YGConfigNew();

  YGNodeRef root = YGNodeNewWithConfig(config);
  YGNodeStyleSetPositionType(root, YGPositionTypeAbsolute);
  YGNodeStyleSetWidth(root, 200);
  YGNodeStyleSetHeight(root, 200);

  YGNodeRef root_child0 = YGNodeNewWithConfig(config);
  YGNodeStyleSetWidth(root_child0, 200);
  YGNodeStyleSetHeight(root_child0, 200);
  // padding-left: calc(10px + 5%) => 10 + (5% of 200) = 20
  YGNodeStyleSetPaddingCalc(root_child0, YGEdgeLeft, {10.0f, 5.0f, 0.0f, 0.0f});
  YGNodeInsertChild(root, root_child0, 0);

  YGNodeRef root_child0_child0 = YGNodeNewWithConfig(config);
  YGNodeStyleSetWidth(root_child0_child0, 50);
  YGNodeStyleSetHeight(root_child0_child0, 50);
  YGNodeInsertChild(root_child0, root_child0_child0, 0);

  YGNodeCalculateLayout(root, YGUndefined, YGUndefined, YGDirectionLTR);

  ASSERT_FLOAT_EQ(20, YGNodeLayoutGetLeft(root_child0_child0));

  YGNodeFreeRecursive(root);
  YGConfigFree(config);
}

TEST(YogaTest, calc_gap_layout) {
  YGConfigRef config = YGConfigNew();

  YGNodeRef root = YGNodeNewWithConfig(config);
  YGNodeStyleSetFlexDirection(root, YGFlexDirectionRow);
  YGNodeStyleSetPositionType(root, YGPositionTypeAbsolute);
  YGNodeStyleSetWidth(root, 200);
  YGNodeStyleSetHeight(root, 100);
  // column-gap: calc(10px + 5%) => 10 + (5% of 200) = 20
  YGNodeStyleSetGapCalc(root, YGGutterColumn, {10.0f, 5.0f, 0.0f, 0.0f});

  YGNodeRef root_child0 = YGNodeNewWithConfig(config);
  YGNodeStyleSetWidth(root_child0, 50);
  YGNodeStyleSetHeight(root_child0, 50);
  YGNodeInsertChild(root, root_child0, 0);

  YGNodeRef root_child1 = YGNodeNewWithConfig(config);
  YGNodeStyleSetWidth(root_child1, 50);
  YGNodeStyleSetHeight(root_child1, 50);
  YGNodeInsertChild(root, root_child1, 1);

  YGNodeCalculateLayout(root, YGUndefined, YGUndefined, YGDirectionLTR);

  ASSERT_FLOAT_EQ(0, YGNodeLayoutGetLeft(root_child0));
  ASSERT_FLOAT_EQ(70, YGNodeLayoutGetLeft(root_child1));

  YGNodeFreeRecursive(root);
  YGConfigFree(config);
}

TEST(YogaTest, calc_margin_layout) {
  YGConfigRef config = YGConfigNew();

  YGNodeRef root = YGNodeNewWithConfig(config);
  YGNodeStyleSetPositionType(root, YGPositionTypeAbsolute);
  YGNodeStyleSetWidth(root, 200);
  YGNodeStyleSetHeight(root, 200);

  YGNodeRef root_child0 = YGNodeNewWithConfig(config);
  YGNodeStyleSetWidth(root_child0, 50);
  YGNodeStyleSetHeight(root_child0, 50);
  // margin-top: calc(20px + 10%) => 20 + (10% of 200) = 40
  YGNodeStyleSetMarginCalc(root_child0, YGEdgeTop, {20.0f, 10.0f, 0.0f, 0.0f});
  YGNodeInsertChild(root, root_child0, 0);

  YGNodeCalculateLayout(root, YGUndefined, YGUndefined, YGDirectionLTR);

  ASSERT_FLOAT_EQ(40, YGNodeLayoutGetTop(root_child0));

  YGNodeFreeRecursive(root);
  YGConfigFree(config);
}
