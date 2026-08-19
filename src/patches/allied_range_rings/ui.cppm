module;
#include <imgui.h>

module patch.allied_range_rings;
import core;
import std;

void AlliedRangeRingsPatch::RenderUi() {
  using enum IntelRangeBehavior;
  int val = std::to_underlying(behavior_);

  bool changed = false;
  changed |= ImGui::RadioButton(
      tr("Own units only", {{Language::Russian, "Только свои юниты"},
                            {Language::Chinese, "仅自身单位"}}),
      &val, std::to_underlying(kOwnUnits));
  ImGui::SetItemTooltip(
      "%s", tr("Show intel range rings for your own units only.",
               {{Language::Russian,
                 "Показывать кольца разведки только для своих юнитов."},
                {Language::Chinese, "仅显示自身单位的侦察范围圈。"}})
                .c_str());

  changed |= ImGui::RadioButton(
      tr("Own units and allied buildings",
         {{Language::Russian, "Свои юниты и союзные постройки"},
          {Language::Chinese, "自身单位和盟友建筑"}}),
      &val, std::to_underlying(kOwnUnitsAndAlliedBuildings));
  ImGui::SetItemTooltip(
      "%s",
      tr("Show intel range rings for own units and allied static structures.",
         {{Language::Russian, "Показывать кольца разведки для своих юнитов и "
                              "союзных стационарных построек."},
          {Language::Chinese, "显示自身单位和盟军静态建筑的侦察范围圈。"}})
          .c_str());

  changed |=
      ImGui::RadioButton(tr("Own and all allied units",
                            {{Language::Russian, "Свои и все союзные юниты"},
                             {Language::Chinese, "自身及所有盟友单位"}}),
                         &val, std::to_underlying(kOwnUnitsAndAlliedUnits));
  ImGui::SetItemTooltip(
      "%s", tr("Show intel range rings for all own and allied units (including "
               "mobile).",
               {{Language::Russian, "Показывать кольца разведки для всех своих "
                                    "и союзных юнитов (включая мобильные)."},
                {Language::Chinese,
                 "显示所有自身和盟军单位（包括移动单位）的侦察范围圈。"}})
                .c_str());

  if (changed) {
    behavior_ = static_cast<IntelRangeBehavior>(val);
  }
}
