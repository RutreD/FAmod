module patch.range_rings;

import :allied;
import :strategic_defense;
import :capture;
import :ui;
import core;
import std;

void RangeRingsPatch::Apply() {
  patch::range_rings::ApplyAllied();
  patch::range_rings::ApplyStrategicDefense();
  patch::range_rings::ApplyCapture();
}

void RangeRingsPatch::RenderUi() { patch::range_rings::RenderRangeRingsUi(); }

void RangeRingsPatch::BindSettings(SettingsBinder &b) {
  b.Bind("RangeRings.allied_behavior", patch::range_rings::allied_behavior_,
         patch::range_rings::IntelRangeBehavior::kOwnUnitsAndAlliedBuildings);
  b.Bind("RangeRings.smd_enabled", patch::range_rings::smd_enabled_, true);
  b.Bind("RangeRings.smd_behavior", patch::range_rings::smd_behavior_,
         patch::range_rings::StrategicDefenseBehavior::kOwnAndAlliedUnits);
  b.Bind("RangeRings.smd_color", patch::range_rings::smd_color_,
         std::array<float, 4>{0.0f, 0.0f, 1.0f, 0.6f});
  b.Bind("RangeRings.capture_enabled", patch::range_rings::capture_enabled_,
         true);
  b.Bind("RangeRings.capture_color", patch::range_rings::capture_color_,
         std::array<float, 4>{0.0f, 1.0f, 1.0f, 0.5f});
}
