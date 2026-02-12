/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <memory>
#include <vector>

#include <capture/NodeToString.h>

namespace facebook::yoga {

using namespace nlohmann;

static void appendFloatIfNotDefault(
    json& j,
    std::string_view key,
    float num,
    float defaultNum) {
  if (num != defaultNum && !YGFloatIsUndefined(num)) {
    j[key] = num;
  }
}

static void appendYGValueIfNotDefault(
    json& j,
    std::string_view key,
    const YGValue& value,
    const YGValue& defaultValue) {
  if (value != defaultValue) {
    if (value.unit == YGUnitAuto) {
      j[key] = "auto";
    } else if (value.unit == YGUnitUndefined) {
      j[key] = "undefined";
    } else if (value.unit == YGUnitCalc) {
      j[key]["value"] = {
          {"px", value.value.calc.px},
          {"percent", value.value.calc.percent},
          {"vw", value.value.calc.vw},
          {"vh", value.value.calc.vh}};
      j[key]["unit"] = "calc";
    } else {
      std::string unit = value.unit == YGUnitPoint ? "px" : "pct";
      j[key]["value"] = value.value.scalar;
      j[key]["unit"] = unit;
    }
  }
}

static void appendEnumValueIfNotDefault(
    json& j,
    std::string_view key,
    std::string_view value,
    std::string_view defaultValue) {
  if (value != defaultValue) {
    j[key] = value;
  }
}

static void appendBoolIfNotDefault(
    json& j,
    std::string_view key,
    bool value,
    bool defaultValue) {
  if (value != defaultValue) {
    j[key] = value;
  }
}

template <auto Field>
static void appendEdges(
    json& j,
    const std::string& key,
    YGNodeRef node,
    YGNodeRef defaultNode) {
  appendYGValueIfNotDefault(
      j["style"],
      key + "-left",
      (*Field)(node, YGEdgeLeft),
      (*Field)(defaultNode, YGEdgeLeft));
  appendYGValueIfNotDefault(
      j["style"],
      key + "-right",
      (*Field)(node, YGEdgeRight),
      (*Field)(defaultNode, YGEdgeRight));
  appendYGValueIfNotDefault(
      j["style"],
      key + "-top",
      (*Field)(node, YGEdgeTop),
      (*Field)(defaultNode, YGEdgeTop));
  appendYGValueIfNotDefault(
      j["style"],
      key + "-bottom",
      (*Field)(node, YGEdgeBottom),
      (*Field)(defaultNode, YGEdgeBottom));
  appendYGValueIfNotDefault(
      j["style"],
      key + "-all",
      (*Field)(node, YGEdgeAll),
      (*Field)(defaultNode, YGEdgeAll));
  appendYGValueIfNotDefault(
      j["style"],
      key + "-start",
      (*Field)(node, YGEdgeStart),
      (*Field)(defaultNode, YGEdgeStart));
  appendYGValueIfNotDefault(
      j["style"],
      key + "-end",
      (*Field)(node, YGEdgeEnd),
      (*Field)(defaultNode, YGEdgeEnd));
  appendYGValueIfNotDefault(
      j["style"],
      key + "-vertical",
      (*Field)(node, YGEdgeVertical),
      (*Field)(defaultNode, YGEdgeVertical));
  appendYGValueIfNotDefault(
      j["style"],
      key + "-horizontal",
      (*Field)(node, YGEdgeHorizontal),
      (*Field)(defaultNode, YGEdgeHorizontal));
}

static void serializeMeasureFuncResults(
    json& j,
    std::vector<SerializedMeasureFunc>& measureFuncs) {
  for (auto measureFunc : measureFuncs) {
    j["measure-funcs"].push_back(
        {{"width", measureFunc.inputWidth},
         {"width-mode", YGMeasureModeToString(measureFunc.widthMode)},
         {"height", measureFunc.inputHeight},
         {"height-mode", YGMeasureModeToString(measureFunc.heightMode)},
         {"output-width", measureFunc.outputWidth},
         {"output-height", measureFunc.outputHeight},
         {"duration-ns", measureFunc.durationNs}});
  }
}

static void serializeTreeImpl(
    json& j,
    SerializedMeasureFuncMap& nodesToMeasureFuncs,
    YGNodeRef node,
    PrintOptions options) {
  std::unique_ptr<YGNode, decltype(&YGNodeFree)> defaultNode(
      YGNodeNew(), YGNodeFree);

  if ((options & PrintOptions::Layout) == PrintOptions::Layout) {
    appendYGValueIfNotDefault(
        j["layout"],
        "width",
        YGNodeStyleGetWidth(node),
        YGNodeStyleGetWidth(YGNodeNew()));
    appendYGValueIfNotDefault(
        j["layout"],
        "height",
        YGNodeStyleGetHeight(node),
        YGNodeStyleGetHeight(YGNodeNew()));
    appendYGValueIfNotDefault(
        j["layout"],
        "top",
        YGNodeStyleGetPosition(node, YGEdgeTop),
        YGNodeStyleGetPosition(YGNodeNew(), YGEdgeTop));
    appendYGValueIfNotDefault(
        j["layout"],
        "left",
        YGNodeStyleGetPosition(node, YGEdgeLeft),
        YGNodeStyleGetPosition(YGNodeNew(), YGEdgeLeft));
  }

  if ((options & PrintOptions::Style) == PrintOptions::Style) {
    appendEnumValueIfNotDefault(
        j["style"],
        "flex-direction",
        YGFlexDirectionToString(YGNodeStyleGetFlexDirection(node)),
        YGFlexDirectionToString(
            YGNodeStyleGetFlexDirection(defaultNode.get())));
    appendEnumValueIfNotDefault(
        j["style"],
        "justify-content",
        YGJustifyToString(YGNodeStyleGetJustifyContent(node)),
        YGJustifyToString(YGNodeStyleGetJustifyContent(defaultNode.get())));
    appendEnumValueIfNotDefault(
        j["style"],
        "align-items",
        YGAlignToString(YGNodeStyleGetAlignItems(node)),
        YGAlignToString(YGNodeStyleGetAlignItems(defaultNode.get())));
    appendEnumValueIfNotDefault(
        j["style"],
        "align-content",
        YGAlignToString(YGNodeStyleGetAlignContent(node)),
        YGAlignToString(YGNodeStyleGetAlignContent(defaultNode.get())));
    appendEnumValueIfNotDefault(
        j["style"],
        "align-self",
        YGAlignToString(YGNodeStyleGetAlignSelf(node)),
        YGAlignToString(YGNodeStyleGetAlignSelf(defaultNode.get())));
    appendEnumValueIfNotDefault(
        j["style"],
        "flex-wrap",
        YGWrapToString(YGNodeStyleGetFlexWrap(node)),
        YGWrapToString(YGNodeStyleGetFlexWrap(defaultNode.get())));
    appendEnumValueIfNotDefault(
        j["style"],
        "overflow",
        YGOverflowToString(YGNodeStyleGetOverflow(node)),
        YGOverflowToString(YGNodeStyleGetOverflow(defaultNode.get())));
    appendEnumValueIfNotDefault(
        j["style"],
        "display",
        YGDisplayToString(YGNodeStyleGetDisplay(node)),
        YGDisplayToString(YGNodeStyleGetDisplay(defaultNode.get())));
    appendEnumValueIfNotDefault(
        j["style"],
        "position-type",
        YGPositionTypeToString(YGNodeStyleGetPositionType(node)),
        YGPositionTypeToString(YGNodeStyleGetPositionType(defaultNode.get())));

    appendFloatIfNotDefault(
        j["style"],
        "flex-grow",
        YGNodeStyleGetFlexGrow(node),
        YGNodeStyleGetFlexGrow(defaultNode.get()));
    appendFloatIfNotDefault(
        j["style"],
        "flex-shrink",
        YGNodeStyleGetFlexShrink(node),
        YGNodeStyleGetFlexShrink(defaultNode.get()));
    appendFloatIfNotDefault(
        j["style"],
        "flex",
        YGNodeStyleGetFlex(node),
        YGNodeStyleGetFlex(defaultNode.get()));
    appendYGValueIfNotDefault(
        j["style"],
        "flex-basis",
        YGNodeStyleGetFlexBasis(node),
        YGNodeStyleGetFlexBasis(defaultNode.get()));

    appendEdges<&YGNodeStyleGetMargin>(j, "margin", node, defaultNode.get());
    appendEdges<&YGNodeStyleGetPadding>(j, "padding", node, defaultNode.get());
    appendEdges<&YGNodeStyleGetBorder>(j, "border", node, defaultNode.get());
    appendEdges<&YGNodeStyleGetPosition>(
        j, "position", node, defaultNode.get());

    appendYGValueIfNotDefault(
        j["style"],
        "gap",
        YGNodeStyleGetGap(node, YGGutterAll),
        YGNodeStyleGetGap(defaultNode.get(), YGGutterAll));
    appendYGValueIfNotDefault(
        j["style"],
        "column-gap",
        YGNodeStyleGetGap(node, YGGutterColumn),
        YGNodeStyleGetGap(defaultNode.get(), YGGutterColumn));
    appendYGValueIfNotDefault(
        j["style"],
        "row-gap",
        YGNodeStyleGetGap(node, YGGutterRow),
        YGNodeStyleGetGap(defaultNode.get(), YGGutterRow));

    appendYGValueIfNotDefault(
        j["style"],
        "width",
        YGNodeStyleGetWidth(node),
        YGNodeStyleGetWidth(defaultNode.get()));
    appendYGValueIfNotDefault(
        j["style"],
        "height",
        YGNodeStyleGetHeight(node),
        YGNodeStyleGetHeight(defaultNode.get()));
    appendYGValueIfNotDefault(
        j["style"],
        "max-width",
        YGNodeStyleGetMaxWidth(node),
        YGNodeStyleGetMaxWidth(defaultNode.get()));
    appendYGValueIfNotDefault(
        j["style"],
        "max-height",
        YGNodeStyleGetMaxHeight(node),
        YGNodeStyleGetMaxHeight(defaultNode.get()));
    appendYGValueIfNotDefault(
        j["style"],
        "min-width",
        YGNodeStyleGetMinWidth(node),
        YGNodeStyleGetMinWidth(defaultNode.get()));
    appendYGValueIfNotDefault(
        j["style"],
        "min-height",
        YGNodeStyleGetMinHeight(node),
        YGNodeStyleGetMinHeight(defaultNode.get()));
  }

  if ((options & PrintOptions::Config) == PrintOptions::Config) {
    YGConfigConstRef config = YGNodeGetConfig(node);
    YGConfigConstRef defaultConfig = YGConfigGetDefault();

    appendBoolIfNotDefault(
        j["config"],
        "use-web-defaults",
        YGConfigGetUseWebDefaults(config),
        YGConfigGetUseWebDefaults(defaultConfig));
    appendFloatIfNotDefault(
        j["config"],
        "point-scale-factor",
        YGConfigGetPointScaleFactor(config),
        YGConfigGetPointScaleFactor(defaultConfig));
    YGErrata errata = YGConfigGetErrata(config);
    if (errata == YGErrataNone || errata == YGErrataAll ||
        errata == YGErrataClassic) {
      appendEnumValueIfNotDefault(
          j["config"],
          "errata",
          YGErrataToString(errata),
          YGErrataToString(YGConfigGetErrata(defaultConfig)));
    }

    if (YGConfigIsExperimentalFeatureEnabled(
            config, YGExperimentalFeatureWebFlexBasis) !=
        YGConfigIsExperimentalFeatureEnabled(
            defaultConfig, YGExperimentalFeatureWebFlexBasis)) {
      j["config"]["experimental-features"].push_back(
          YGExperimentalFeatureToString(YGExperimentalFeatureWebFlexBasis));
    }
  }

  if ((options & PrintOptions::Node) == PrintOptions::Node) {
    appendBoolIfNotDefault(
        j["node"],
        "always-forms-containing-block",
        YGNodeGetAlwaysFormsContainingBlock(node),
        YGNodeGetAlwaysFormsContainingBlock(defaultNode.get()));
    if (YGNodeHasMeasureFunc(node)) {
      auto measureFuncIt = nodesToMeasureFuncs.find(node);
      if (measureFuncIt == nodesToMeasureFuncs.end()) {
        j["node"]["measure-funcs"];
      } else {
        serializeMeasureFuncResults(j["node"], measureFuncIt->second);
      }
    }
  }

  const size_t childCount = YGNodeGetChildCount(node);
  if ((options & PrintOptions::Children) == PrintOptions::Children &&
      childCount > 0) {
    for (size_t i = 0; i < childCount; i++) {
      j["children"].push_back({});
      serializeTreeImpl(
          j["children"][i],
          nodesToMeasureFuncs,
          YGNodeGetChild(node, i),
          options);
    }
  }
}

void serializeTree(
    json& j,
    SerializedMeasureFuncMap& nodesToMeasureFuncs,
    YGNodeRef node,
    PrintOptions options) {
  serializeTreeImpl(j["tree"], nodesToMeasureFuncs, node, options);
}

void serializeLayoutInputs(
    json& j,
    float availableWidth,
    float availableHeight,
    YGDirection ownerDirection) {
  j["layout-inputs"] = {
      {"available-width", availableWidth},
      {"available-height", availableHeight},
      {"owner-direction", YGDirectionToString(ownerDirection)},
  };
}

} // namespace facebook::yoga
