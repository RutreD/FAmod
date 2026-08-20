module;
#include <imgui.h>

module patch.share_factory_assist;
import core;

void ShareFactoryAssistPatch::RenderUi() {
  ImGui::Checkbox(tr("Factory Assist Orders",
                     {{Language::Russian, "Приказы ассиста фабрик"},
                      {Language::Chinese, "工厂协助指令"}}),
                  &enabled_);

  if (ImGui::BeginItemTooltip()) {
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 25.0f);
    ImGui::TextUnformatted(
        tr("Allows factories to issue assist orders for produced units (target "
           "or ground rally).",
           {{Language::Russian, "Позволяет фабрикам отдавать приказ ассиста "
                                "для произведённых юнитов "
                                "(цель или точка на земле)."},
            {Language::Chinese,
             "允许工厂为其生产的单位下达协助指令（目标或地面集结点）。"}})
            .c_str());
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
  }
}
