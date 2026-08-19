module;
#include <imgui.h>

module patch.health_bars;
import core;

void HealthBarsPatch::RenderUi() {
  ImGui::Checkbox(
      tr("Show health bars at full health",
         {{Language::Russian, "Показывать полосы здоровья при полном HP"},
          {Language::Chinese, "满血时显示生命条"}}),
      &render_full_health_);

  ImGui::SetItemTooltip(
      "%s",
      tr("When disabled, health bars are hidden for undamaged units (100% "
         "health).",
         {{Language::Russian,
           "Если отключено, полосы здоровья скрыты для неповрежденных юнитов "
           "(100% HP)."},
          {Language::Chinese,
           "禁用时，满血单位（100%生命值）将隐藏生命值条。"}})
          .c_str());
}
