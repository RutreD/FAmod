export module patch.range_rings;

import :allied;
import :strategic_defense;
import :capture;
import :ui;

import core;
import std;

export class RangeRingsPatch : public IPatch {
public:
  [[nodiscard]] std::string_view Name() const noexcept override {
    return tr("Range Rings", {{Language::Russian, "Кольца радиусов"},
                              {Language::Chinese, "范围圈设置"}});
  }

  [[nodiscard]] std::string_view Description() const noexcept override {
    return tr(
        "Configures extra range rings and overlays (Allied intel & defense, "
        "Strategic Nuclear Defense, Capture range).",
        {{Language::Russian,
          "Настройка дополнительных колец радиусов и оверлеев (разведка и ПРО "
          "союзников, "
          "стратегическая противоракетная оборона, радиус захвата)."},
         {Language::Chinese, "配置额外的范围圈和覆盖层（友军侦察与反导、战略核"
                             "防御、捕获范围）。"}});
  }

  void Apply() override {
    patch::range_rings::ApplyAllied();
    patch::range_rings::ApplyStrategicDefense();
    patch::range_rings::ApplyCapture();
  }

  void RenderUi() override { patch::range_rings::RenderRangeRingsUi(); }

  void BindSettings(SettingsBinder &b) override {
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
};
