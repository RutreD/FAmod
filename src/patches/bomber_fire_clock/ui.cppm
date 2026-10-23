module;
#include <imgui.h>

module patch.bomber_fire_clock;
import core;

void BomberFireClockPatch::RenderUi() {
  ImGui::Checkbox(tr("Bomber Reload Bar (Fire Clock)",
                     {{Language::Russian,
                       "Полоса перезарядки бомбардировщиков"},
                      {Language::Chinese, "轰炸机装填进度条"}}),
                  &enabled_);

  if (ImGui::BeginItemTooltip()) {
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 25.0f);
    ImGui::TextUnformatted(
        tr("Displays a yellow reload progress bar (fire clock) below bombers' "
           "health bars for their primary bomb weapon.",
           {{Language::Russian,
             "Отображает жёлтую полосу перезарядки (fire clock) под "
             "полосой здоровья бомбардировщиков для их основного вооружения."},
            {Language::Chinese,
             "在轰炸机生命条下方为主武器显示黄色装填进度条（fire clock）。"}})
            .c_str());
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
  }
}
