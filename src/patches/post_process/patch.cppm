export module patch.post_process;

import std;
import core;

export class PostProcessPatch : public IPatch {
public:
  [[nodiscard]] std::string_view Name() const noexcept override {
    return tr("World Post-Processing",
              {{Language::Russian, "Пост-процессинг мира"},
               {Language::Chinese, "世界后处理"}});
  }

  [[nodiscard]] std::string_view Description() const noexcept override {
    return tr(
        "Adds custom post-processing shaders (FXAA, AMD FidelityFX CAS, "
        "split toning, bloom boost, color grading, tonemapping) applied only "
        "to the map and units, leaving strategic icons, health bars, and "
        "game UI completely untouched and sharp.",
        {{Language::Russian,
          "Добавляет кастомные шейдеры пост-процессинга (FXAA сглаживание, "
          "AMD FidelityFX CAS резкость, раздельное тонирование, усиление "
          "свечения лазеров/взрывов, сочность, тонемаппинг), действующие только "
          "на карту и юнитов, не затрагивая стратегические иконки, полосы "
          "здоровья (HP) и игровой интерфейс (UI)."},
         {Language::Chinese,
          "添加自定义后处理着色器（FXAA抗锯齿、AMD CAS锐化、分离色调、辉光增强、"
          "自然饱和度、色调映射），仅作用于地图和单位，完全不影响战略图标、"
          "生命条和游戏UI界面。"}});
  }

  void Apply() override;
  void RenderUi() override;
  void BindSettings(SettingsBinder &b) override;
};
