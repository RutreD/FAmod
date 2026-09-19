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
           "or ground rally). Assisting another factory as the first command "
           "copies its build queue, while queuing with Shift sets produced units "
           "to assist it.",
           {{Language::Russian,
             "Позволяет фабрикам отдавать приказ ассиста для произведённых юнитов "
             "(на цель или точку на земле). Первый приказ на другую фабрику "
             "копирует её очередь постройки, а через Shift — задаёт ассист "
             "произведёнными юнитами."},
            {Language::Chinese,
             "允许工厂为其生产的单位下达协助指令（目标或地面集结点）。"
             "对另一座工厂的首个指令将复制其建造队列，通过 Shift 添加则让生产出的单位协助该工厂。"}})
            .c_str());
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
  }
}
