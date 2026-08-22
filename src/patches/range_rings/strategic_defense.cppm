module;

module patch.range_rings:strategic_defense;

import fa;
import core;
import std;

namespace patch::range_rings {

enum class StrategicDefenseBehavior : int {
  kOwnUnits = 0,
  kOwnAndAlliedUnits = 1,
};

inline bool smd_enabled_{true};
inline StrategicDefenseBehavior smd_behavior_{
    StrategicDefenseBehavior::kOwnAndAlliedUnits};
inline std::array<float, 4> smd_color_{0.0f, 0.0f, 1.0f, 0.6f};

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

[[nodiscard]] const fa::RUnitBlueprintWeapon *
FindStrategicDefenseWeapon(const fa::RUnitBlueprint *blueprint) noexcept {
  if (!blueprint || !blueprint->mWeapon.begin || !blueprint->mWeapon.end)
    return nullptr;

  for (const auto *it = blueprint->mWeapon.begin; it != blueprint->mWeapon.end;
       ++it) {
    if (it->mRangeCategory == fa::UWRC_Countermeasure &&
        it->mCountedProjectile && !it->mNukeWeapon) {
      return it;
    }
  }
  return nullptr;
}

struct StrategicDefenseExtractor : public fa::RangeExtractor {
  bool GetRange(fa::RangeInfo *out, const fa::RUnitBlueprint *blueprint,
                const fa::Vector3f *pos) override {
    if (!smd_enabled_ || !blueprint || !out || !pos)
      return false;

    const auto *weapon = FindStrategicDefenseWeapon(blueprint);
    if (!weapon)
      return false;

    out->x = pos->x;
    out->z = pos->z;
    out->min_range = weapon->mMinRadius;
    out->max_range = weapon->mMaxRadius;
    return true;
  }

  bool Extract(fa::RangeInfo *out, fa::UserUnit *unit,
               float /*interp*/) override {
    if (!smd_enabled_ || !unit || !out)
      return false;

    auto blueprint =
        static_cast<const fa::RUnitBlueprint *>(unit->mParams.mBlueprint);
    if (!blueprint)
      return false;

    const auto *weapon = FindStrategicDefenseWeapon(blueprint);
    if (!weapon)
      return false;

    out->x = unit->mTransform.pos.x;
    out->z = unit->mTransform.pos.z;
    out->min_range = weapon->mMinRadius;
    out->max_range = weapon->mMaxRadius;
    return true;
  }
};

void ApplyStrategicDefense() {
  // 1. Register native C++ extractor for "StrategicDefense" range calculations
  fa::RangeExtractor::Register<StrategicDefenseExtractor>("StrategicDefense");

  // 2. Subscribe to VFS memory map event to inject overlay definition
  core::events::OnMemoryMapFile += [](fa::ConstMemBuffer *buffer,
                                      const char *fname) {
    if (fname && std::string_view(fname).ends_with("rangeoverlayparams.lua")) {
      const auto hex = ToFaColorHex(smd_color_);
      buffer->append(std::format(R"(
        if RangeOverlayParams.StrategicDefense == nil then
          RangeOverlayParams.StrategicDefense = {{
              key = 'strategic_defense',
              Label = "<LOC map_options_0011>Anti-Nuke",
              Categories = categories.ANTIMISSILE * (categories.STRATEGIC + categories.TECH3 + categories.EXPERIMENTAL),
              NormalColor = '{0}',
              SelectColor = '{0}',
              RolloverColor = '{0}',
              Inner = {{0.02, 2.0}},
              Outer = {{0.04, 4.0}},
              Type = 1,
              Tooltip = "overlay_strategic_defense",
          }}
        end
      )",
                                 hex));
    }
  };
}

} // namespace patch::range_rings
