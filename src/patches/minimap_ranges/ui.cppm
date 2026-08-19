module;
#include <imgui.h>

module patch.minimap_ranges;
import core;

void MinimapRangesPatch::RenderUi() {
  ImGui::Checkbox(
      tr("Show range rings on minimap",
         {{Language::Russian, "Показывать кольца радиуса на миникарте"},
          {Language::Chinese, "在小地图上显示范围圈"}}),
      &render_ranges_);
  ImGui::SetItemTooltip(
      "%s",
      tr("When disabled, range rings are hidden on the minimap.",
         {{Language::Russian, "Если отключено, кольца скрыты на миникарте."},
          {Language::Chinese, "禁用时，小地图上将隐藏范围圈。"}})
          .c_str());
}
