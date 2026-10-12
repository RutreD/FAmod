module;
#include <imgui.h>

module patch.assist_construction;
import core;

void AssistConstructionOrdersPatch::RenderUi() {
  ImGui::Checkbox(tr("Assist Construction Orders",
                     {{Language::Russian, "Ассист запланированных построек"},
                      {Language::Chinese, "协助规划建造"}}),
                  &enabled_);

  if (ImGui::BeginItemTooltip()) {
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 25.0f);
    ImGui::TextUnformatted(
        tr("Allows engineers to assist queued or unbuilt building blueprints "
           "(ghosts) by right-clicking them directly, or by issuing explicit "
           "Repair orders.\n\n"
           "• Vanilla: RMB on a blueprint issues Move; Repair ignores "
           "blueprints.\n"
           "• Patch: RMB or Repair directly assists the blueprint "
           "construction.",
           {{Language::Russian,
             "Позволяет инженерам ассистировать запланированные призраки "
             "построек "
             "(чертежи в очереди) кликом ПКМ напрямую или через приказ починки "
             "(Repair).\n\n"
             "• В оригинале: ПКМ по призраку отдаёт приказ движения (Move); "
             "Repair не работает по призракам.\n"
             "• С патчем: ПКМ или Repair сразу отправляют инженера строить "
             "этот чертёж."},
            {Language::Chinese,
             "允许工程单位通过直接右键点击规划中的建筑蓝图（幽灵虚影）或使用修"
             "理指令（Repair）协助建造。\n\n"
             "• 原版表现：右键蓝图会变为移动指令；修理指令忽略蓝图。\n"
             "• 补丁效果：右键或修理直接协助建造该蓝图。"}})
            .c_str());
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
  }
}
