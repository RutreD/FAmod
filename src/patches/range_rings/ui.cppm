module;
#include <imgui.h>

module patch.range_rings:ui;

import :allied;
import :strategic_defense;
import :capture;
import core;
import std;

namespace patch::range_rings {

void RenderRangeRingsUi() {
  // 1. Intel Range Rings section
  ImGui::Text(
      "%s",
      tr("Intel Range Rings (Radar / Omni / Sonar)",
         {{Language::Russian, "Кольца радиуса разведки (Радар / Омни / Сонар)"},
          {Language::Chinese, "侦察范围圈（雷达 / 全向 / 声呐）"}})
          .data());

  int intel_val = std::to_underlying(allied_behavior_);
  bool intel_changed = false;

  intel_changed |= ImGui::RadioButton(
      tr("Own units only##Intel",
         {{Language::Russian, "Только свои юниты##Intel"},
          {Language::Chinese, "仅自身单位##Intel"}})
          .data(),
      &intel_val, std::to_underlying(IntelRangeBehavior::kOwnUnits));
  ImGui::SetItemTooltip(
      "%s", tr("Show intel range rings for your own units only.",
               {{Language::Russian,
                 "Показывать кольца разведки только для своих юнитов."},
                {Language::Chinese, "仅显示自身单位的侦察范围圈。"}})
                .c_str());

  intel_changed |= ImGui::RadioButton(
      tr("Own units and allied buildings##Intel",
         {{Language::Russian, "Свои юниты и союзные постройки##Intel"},
          {Language::Chinese, "自身单位和盟友建筑##Intel"}})
          .data(),
      &intel_val,
      std::to_underlying(IntelRangeBehavior::kOwnUnitsAndAlliedBuildings));
  ImGui::SetItemTooltip(
      "%s",
      tr("Show intel range rings for own units and allied static structures.",
         {{Language::Russian, "Показывать кольца разведки для своих юнитов и "
                              "союзных стационарных построек."},
          {Language::Chinese, "显示自身单位和盟军静态建筑的侦察范围圈。"}})
          .c_str());

  intel_changed |= ImGui::RadioButton(
      tr("Own and all allied units##Intel",
         {{Language::Russian, "Свои и все союзные юниты##Intel"},
          {Language::Chinese, "自身及所有盟友单位##Intel"}})
          .data(),
      &intel_val,
      std::to_underlying(IntelRangeBehavior::kOwnUnitsAndAlliedUnits));
  ImGui::SetItemTooltip(
      "%s", tr("Show intel range rings for all own and allied units (including "
               "mobile).",
               {{Language::Russian, "Показывать кольца разведки для всех своих "
                                    "и союзных юнитов (включая мобильные)."},
                {Language::Chinese,
                 "显示所有自身和盟军单位（包括移动单位）的侦察范围圈。"}})
                .c_str());

  if (intel_changed) {
    allied_behavior_ = static_cast<IntelRangeBehavior>(intel_val);
  }

  ImGui::Separator();

  // 2. Strategic Nuclear Defense section
  ImGui::Checkbox(
      tr("Strategic Nuclear Defense (Anti-Nuke / SMD)",
         {{Language::Russian, "Стратегическая ПРО (Anti-Nuke / SMD)"},
          {Language::Chinese, "战略核防御（反导 / SMD）"}})
          .data(),
      &smd_enabled_);
  ImGui::SetItemTooltip(
      "%s", tr("Shows nuclear defense coverage radius on the map.",
               {{Language::Russian,
                 "Отображает радиус покрытия ядерного щита (ПРО) на карте."},
                {Language::Chinese, "在地图上显示反导防御覆盖范围。"}})
                .c_str());

  if (smd_enabled_) {
    ImGui::Indent();
    int smd_val = std::to_underlying(smd_behavior_);
    bool smd_changed = false;

    smd_changed |= ImGui::RadioButton(
        tr("Own units only##SMD",
           {{Language::Russian, "Только свои юниты##SMD"},
            {Language::Chinese, "仅自身单位##SMD"}})
            .data(),
        &smd_val, std::to_underlying(StrategicDefenseBehavior::kOwnUnits));
    ImGui::SetItemTooltip(
        "%s", tr("Show anti-nuke range rings for your own units only.",
                 {{Language::Russian,
                   "Показывать радиус ПРО только для своих юнитов."},
                  {Language::Chinese, "仅显示自身单位的反导范围圈。"}})
                  .c_str());

    smd_changed |= ImGui::RadioButton(
        tr("Own and allied units##SMD",
           {{Language::Russian, "Свои и союзные юниты##SMD"},
            {Language::Chinese, "自身及盟友单位##SMD"}})
            .data(),
        &smd_val,
        std::to_underlying(StrategicDefenseBehavior::kOwnAndAlliedUnits));
    ImGui::SetItemTooltip(
        "%s", tr("Show anti-nuke range rings for own and allied units.",
                 {{Language::Russian,
                   "Показывать радиус ПРО для своих и союзных юнитов."},
                  {Language::Chinese, "显示自身及盟军单位的反导范围圈。"}})
                  .c_str());

    if (smd_changed) {
      smd_behavior_ = static_cast<StrategicDefenseBehavior>(smd_val);
    }

    ImGui::Spacing();
    ImGui::ColorEdit4(
        tr("Ring Color##SMD", {{Language::Russian, "Цвет кольца##SMD"},
                               {Language::Chinese, "范围圈颜色##SMD"}})
            .data(),
        smd_color_.data(),
        ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    ImGui::SetItemTooltip(
        "%s",
        tr("Color changes take effect upon starting a new match / reloading "
           "the game.",
           {{Language::Russian, "Изменение цвета вступит в силу в следующем "
                                "матче / при следующей загрузке игры."},
            {Language::Chinese, "颜色更改将在下一局比赛/重新加载游戏时生效。"}})
            .c_str());

    ImGui::Unindent();
  }

  ImGui::Separator();

  // 3. Capture Range Ring section
  ImGui::Checkbox(tr("Capture Range Ring",
                     {{Language::Russian, "Кольцо радиуса захвата (Capture)"},
                      {Language::Chinese, "捕获范围圈"}})
                      .data(),
                  &capture_enabled_);
  ImGui::SetItemTooltip(
      "%s", tr("Shows capture radius for ACUs, SACUs and Engineers.",
               {{Language::Russian,
                 "Отображает радиус захвата ACU, SACU и Engineers."},
                {Language::Chinese, "显示 ACU、SACU 和工程兵的捕获范围圈。"}})
                .c_str());

  if (capture_enabled_) {
    ImGui::Indent();
    ImGui::ColorEdit4(
        tr("Ring Color##Capture", {{Language::Russian, "Цвет кольца##Capture"},
                                   {Language::Chinese, "范围圈颜色##Capture"}})
            .data(),
        capture_color_.data(),
        ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    ImGui::SetItemTooltip(
        "%s",
        tr("Color changes take effect upon starting a new match / reloading "
           "the game.",
           {{Language::Russian, "Изменение цвета вступит в силу в следующем "
                                "матче / при следующей загрузке игры."},
            {Language::Chinese, "颜色更改将在下一局比赛/重新加载游戏时生效。"}})
            .c_str());
    ImGui::Unindent();
  }
}

} // namespace patch::range_rings
