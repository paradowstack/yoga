# calc() Feature — Code Review

## Executive Summary

The calc() feature adds support for CSS-style `calc()` expressions with mixed units (px, %, vw, vh) across Yoga's layout system. The implementation is generally sound but has several issues that should be addressed before opening a PR.

---

## Critical Issues

### 1. **Bug: `capture/NodeToString.cpp` — `appendYGValueIfNotDefault` does not handle YGUnitCalc**

**Location:** `capture/NodeToString.cpp:27-42`

**Problem:** When `value.unit == YGUnitCalc`, the code falls through to the `else` branch which:
- Sets `unit = "pct"` (incorrect — should be `"calc"`)
- Does `j[key]["value"] = value.value` — assigns the union; for `YGUnitCalc` the active member is `value.value.calc`, but nlohmann::json may not correctly serialize a C union/struct

**Impact:** Capture/serialization of trees with calc values produces wrong or broken JSON. Border values (now using `YGNodeStyleGetBorder` which returns `YGValue`) can also be calc.

**Fix:** Add explicit handling for `YGUnitCalc`:

```cpp
} else if (value.unit == YGUnitCalc) {
  j[key]["value"] = {{"px", value.value.calc.px},
                     {"percent", value.value.calc.percent},
                     {"vw", value.value.calc.vw},
                     {"vh", value.value.calc.vh}};
  j[key]["unit"] = "calc";
} else if (value.unit == YGUnitPoint) {
  j[key]["value"] = value.value.scalar;
  j[key]["unit"] = "px";
} else if (value.unit == YGUnitPercent) {
  j[key]["value"] = value.value.scalar;
  j[key]["unit"] = "pct";
} else {
  // MaxContent, FitContent, Stretch — handle as keywords
  j[key] = YGUnitToString(value.unit);
}
```

Also consider handling `YGUnitMaxContent`, `YGUnitFitContent`, `YGUnitStretch` explicitly for dimension values.

---

### 2. **Layout capture uses style values instead of computed layout**

**Location:** `capture/NodeToString.cpp:138-141`

**Problem:** When `PrintOptions::Layout` is set:
```cpp
j["layout"]["width"] = YGNodeStyleGetWidth(node).value;
j["layout"]["height"] = YGNodeStyleGetHeight(node).value;
```
This reads **style** dimensions, not **computed layout** dimensions. For calc values, `value.value` is a union — accessing `.value` when unit is Calc yields undefined behavior (wrong union member).

**Impact:** Undefined behavior when capturing layout of nodes with calc dimensions. Potentially wrong/crashes.

**Fix:** The layout section should likely use `YGNodeLayoutGetWidth`/`YGNodeLayoutGetHeight` (computed layout), not style getters. If the capture format intentionally stores style in the "layout" section, document it and ensure safe access for all unit types (e.g. resolve calc to a float before storing).

---

### 3. **Viewport default is 0 — silent wrong results for vw/vh**

**Location:** `yoga/config/Config.h:91-92` (default `viewportWidth_ = 0.0f`, `viewportHeight_ = 0.0f`)

**Problem:** With default config, `calc(100vw)` and `calc(100vh)` resolve to 0. Callers who use vw/vh but forget to set viewport get zero-sized elements.

**Recommendation:**
- Document in `YGConfigSetViewportWidth`/`YGConfigSetViewportHeight` that vw/vh require a non-zero viewport
- Consider asserting or logging when resolving calc with vw/vh and viewport is 0
- Or use a sentinel (e.g. NaN) to indicate "viewport not set" and fail more visibly

---

## Potential Issues

### 4. **StyleValuePool::storeCalc — replace logic when previous value was single 64-bit**

**Location:** `yoga/style/StyleValuePool.cpp:241-253`

**Observation:** When replacing a 32-bit value with a calc:
- `buffer_.replace(handle.value(), first)` sees a narrow slot and pushes, returning a new index
- We then `push(second)`
- Old buffer slots at `handle.value()` are orphaned

The buffer does not compact. Repeated style updates (e.g. flex-basis: calc(...) then 10px then calc(...)) can grow the buffer without reclaiming. This is a general pool concern, not calc-specific, but calc increases the chance of value-size changes.

**Recommendation:** Document as acceptable trade-off, or consider future compaction/indirection if this becomes a problem.

---

### 5. **YGCalc uses plain floats — no NaN for "undefined" component**

**Location:** `yoga/YGCalc.h:17-23`

**Problem:** `YGCalc` has `float px, percent, vw, vh` with no way to represent "this component is absent." By contrast, `StyleCalcLength` uses `FloatOptional` which can be undefined.

**Impact:** In `YGCalcToStyleCalcLength` (YGNodeStyle.cpp:18-24), we always construct `FloatOptional{calc.px}` etc. Uninitialized or zero values are indistinguishable from "explicit 0" — e.g. `calc(50%)` vs `calc(50% + 0px)`.

**Recommendation:** Either:
- Document that all four components must always be set (0 for absent)
- Or extend `YGCalc` to support optional components (e.g. sentinel values)

---

### 6. **Missing `vmin` / `vmax` support**

**Problem:** CSS supports `vmin` and `vmax`; the implementation only has `vw` and `vh`.

**Impact:** Not a bug, but a limitation vs. the spec. Worth documenting.

---

### 7. **CSS calc() allows `min()`/`max()` — not implemented**

**Problem:** Real CSS calc supports `min()`, `max()`, `clamp()`. This implementation only supports additive combinations of px, %, vw, vh.

**Impact:** Again a spec limitation. Should be documented.

---

## Minor / Style

### 8. **StyleCalcLength::inexactEquals — correct but subtle**

**Location:** `yoga/style/StyleCalcLength.h:124-130`

Using `unwrap()` for `inexactEquals` is correct: `yoga::inexactEquals` treats NaN as equal when both are NaN, and distinct when one is NaN. No change needed.

---

### 9. **StyleSizeLength — unused ValueType enum**

**Location:** `yoga/style/StyleSizeLength.h:176-179`

`enum class ValueType` is declared but never used. Safe to remove.

---

### 10. **Border API change — capture compatibility**

**Location:** `capture/NodeToString.cpp` (diff)

The change from `borderFloatToYGValue` to `YGNodeStyleGetBorder` is correct because border now supports calc. Ensure all capture consumers expect `YGValue` for border.

---

## Architecture Notes

### Strengths

1. **Config-driven viewport** — viewport lives on config and invalidates layout (`configUpdateInvalidatesLayout`), so cache invalidation is correct.
2. **Consistent resolution** — `StyleCalcLength::resolve()` is used via `StyleValuePool::resolveCalc` and `Style::resolve*` throughout layout.
3. **Storage** — `StyleValuePool` stores calc as two 64-bit values in `SmallValueBuffer`; the replace logic correctly handles both 32-bit and 64-bit prior values.
4. **Bindings** — C API, Java, and JS enums include `YGUnitCalc`; config viewport getters/setters are exposed.

### Areas to double-check

- **Java/Kotlin:** Confirm `YGValue`/`YGCalc` marshaling and use of viewport config.
- **JavaScript:** Confirm `YGValue` handling for Calc in the TS/JS layer.
- **Node cloning:** Styles (including calc) are copied via `Style` copy; `StyleValuePool` is part of `Style`, so calc should be cloned correctly.

---

## Plan Before PR

1. **Must fix**
   - [ ] Fix `appendYGValueIfNotDefault` for `YGUnitCalc` (and other non-point/percent units)
   - [ ] Fix layout capture to avoid UB when style uses calc (or document and use safe access)
   - [ ] Add documentation for viewport defaults (0) and vw/vh behavior

2. **Should consider**
   - [ ] Document `YGCalc` semantics (all components always set vs. optional)
   - [ ] Document lack of `vmin`/`vmax` and `min()`/`max()`/`clamp()`
   - [ ] Remove unused `ValueType` enum in `StyleSizeLength`

3. **Sanity checks**
   - [ ] Run full test suite (including layout tests with calc)
   - [ ] Exercise capture/serialization with calc values
   - [ ] Test Java and JS bindings with calc and viewport config

---

## Files Reviewed

| File | Status |
|------|--------|
| `yoga/YGCalc.h` | OK (see note on component semantics) |
| `yoga/YGValue.h` | OK |
| `yoga/YGValue.cpp` | OK |
| `yoga/style/StyleCalcLength.h` | OK |
| `yoga/style/StyleValuePool.h` | OK |
| `yoga/style/StyleValuePool.cpp` | OK |
| `yoga/style/StyleValueHandle.h` | OK |
| `yoga/style/StyleLength.h` | OK |
| `yoga/style/StyleSizeLength.h` | OK (minor: unused enum) |
| `yoga/style/Style.h` | OK |
| `yoga/config/Config.*` | OK |
| `yoga/YGConfig.*` | OK |
| `yoga/YGEnums.*` | OK |
| `yoga/YGNodeStyle.cpp` | OK |
| `yoga/algorithm/*` | OK (viewport passed correctly) |
| `capture/NodeToString.cpp` | **Issues #1, #2** |
| `yoga/Yoga.h` | OK (YGCalc.h included) |
