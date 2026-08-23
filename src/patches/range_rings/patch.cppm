export module patch.range_rings;
import std;
import core;

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

  void Apply() override;
  void RenderUi() override;
  void BindSettings(SettingsBinder &b) override;
};
