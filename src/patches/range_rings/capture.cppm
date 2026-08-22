module;

module patch.range_rings:capture;

import fa;
import core;
import std;

namespace patch::range_rings {

inline bool capture_enabled_{true};
inline std::array<float, 4> capture_color_{0.0f, 1.0f, 1.0f, 0.5f};

// Formats float RGBA to Forged Alliance Lua 'AARRGGBB' hex string
[[nodiscard]] inline std::string ToFaColorHex(const std::array<float, 4> &col) {
  const auto r =
      static_cast<uint8_t>(std::clamp(col[0], 0.0f, 1.0f) * 255.0f + 0.5f);
  const auto g =
      static_cast<uint8_t>(std::clamp(col[1], 0.0f, 1.0f) * 255.0f + 0.5f);
  const auto b =
      static_cast<uint8_t>(std::clamp(col[2], 0.0f, 1.0f) * 255.0f + 0.5f);
  const auto a =
      static_cast<uint8_t>(std::clamp(col[3], 0.0f, 1.0f) * 255.0f + 0.5f);
  return std::format("{:02X}{:02X}{:02X}{:02X}", a, r, g, b);
}

[[nodiscard]] bool
HasCaptureCapability(const fa::RUnitBlueprint *blueprint) noexcept {
  return blueprint && blueprint->mEconomy.mMaxBuildDistance > 0.0f;
}

struct CaptureExtractor : public fa::RangeExtractor {
  bool GetRange(fa::RangeInfo *out, const fa::RUnitBlueprint *blueprint,
                const fa::Vector3f *pos) override {
    if (!capture_enabled_ || !HasCaptureCapability(blueprint) || !out || !pos)
      return false;

    out->x = pos->x;
    out->z = pos->z;
    out->min_range = 0.0f;
    out->max_range = blueprint->mEconomy.mMaxBuildDistance;
    return true;
  }

  bool Extract(fa::RangeInfo *out, fa::UserUnit *unit,
               float /*interp*/) override {
    if (!capture_enabled_ || !unit || !out)
      return false;

    auto blueprint =
        static_cast<const fa::RUnitBlueprint *>(unit->mParams.mBlueprint);
    if (!HasCaptureCapability(blueprint))
      return false;

    out->x = unit->mTransform.pos.x;
    out->z = unit->mTransform.pos.z;
    out->min_range = 0.0f;
    out->max_range = blueprint->mEconomy.mMaxBuildDistance;
    return true;
  }
};

void ApplyCapture() {
  // 1. Register native C++ extractor for "Capture" range calculations
  fa::RangeExtractor::Register<CaptureExtractor>("Capture");

  // 2. Subscribe to VFS memory map event to inject overlay definition
  core::events::OnMemoryMapFile += [](fa::ConstMemBuffer *buffer,
                                      const char *fname) {
    if (fname && std::string_view(fname).ends_with("rangeoverlayparams.lua")) {
      const auto hex = ToFaColorHex(capture_color_);
      buffer->append(std::format(R"(
        if RangeOverlayParams.Capture == nil then
          RangeOverlayParams.Capture = {{
              key = 'capture',
              Label = "<LOC map_options_0010>Capture",
              Categories = categories.CAPTURE,
              NormalColor = '{0}',
              SelectColor = '{0}',
              RolloverColor = '{0}',
              Inner = {{0.02, 2.0}},
              Outer = {{0.04, 4.0}},
              Type = 1,
              Tooltip = "overlay_capture",
          }}
        end
      )",
                                 hex));
    }
  };
}

} // namespace patch::range_rings
