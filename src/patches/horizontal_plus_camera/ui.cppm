module;
#include <imgui.h>

module patch.horizontal_plus_camera;
import core;

void HorizontalPlusCameraPatch::RenderUi() {
  ImGui::Checkbox(tr("Use Hor+ world camera"), &enabled_);
  ImGui::SetItemTooltip(
      "%s",
      tr("Uses 4:3 as the reference framing. Widescreen and ultrawide world "
         "views retain the vanilla 4:3 vertical view and reveal additional "
         "world horizontally. Each split-screen viewport is adjusted "
         "independently.")
          .c_str());
}
