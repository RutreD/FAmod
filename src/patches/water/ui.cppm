module;
#include <imgui.h>

module patch.water;

import core;

void WaterPatch::RenderUi() {
  bool restart_needed = false;

  if (ImGui::Checkbox(
          tr("Enable Mirror Water Reflections",
             {{Language::Russian, "Включить зеркальные отражения воды"},
              {Language::Chinese, "启用全水面镜像反射"}})
              .data(),
          &enabled_)) {
    restart_needed = true;
  }

  ImGui::SetItemTooltip(
      "%s",
      tr("Master toggle for water reflections and rendering pipeline modifications.",
         {{Language::Russian,
           "Главный переключатель отражений воды и модификаций конвейера рендеринга."},
          {Language::Chinese,
           "水面反射和渲染管线修改的主开关。"}})
          .c_str());

  if (enabled_) {
    ImGui::Indent(20.0f);

    if (ImGui::Checkbox(
            tr("Override game water shader (SSR)",
               {{Language::Russian, "Заменять оригинальный шейдер воды (SSR)"},
                {Language::Chinese, "替换游戏水体着色器 (SSR)"}})
                .data(),
            &override_shader_)) {
      restart_needed = true;
    }

    ImGui::SetItemTooltip(
        "%s",
        tr("When enabled, replaces the game's water2.fx shader with Shader Model 3.0 SSR.\n"
           "Uncheck if you want to use the original shader or custom water2.fx from game archives.",
           {{Language::Russian,
             "Если включено, подменяет water2.fx на шейдер SM 3.0 с экранными отражениями.\n"
             "Снимите галочку, если хотите оставить оригинальный шейдер или использовать файл water2.fx из архивов игры."},
            {Language::Chinese,
             "启用时，将游戏 water2.fx 着色器替换为带有屏幕空间反射的 SM 3.0 着色器。\n"
             "如果希望保留原版着色器或使用游戏归档中的自定义 water2.fx，请取消勾选。"}})
            .c_str());

    if (ImGui::Checkbox(
            tr("Reflect projectiles, lasers & effects in water (pipeline reorder)",
               {{Language::Russian, "Отражать снаряды, лазеры и взрывы в воде (изменение порядка рендера)"},
                {Language::Chinese, "在水面反射弹道、激光与特效 (调整渲染管线顺序)"}})
                .data(),
            &reflect_projectiles_)) {
      restart_needed = true;
    }

    ImGui::SetItemTooltip(
        "%s",
        tr("Reorders the rendering pipeline so projectiles, lasers, and weapon effects are captured and reflected in water.\n"
           "Requires game restart to apply.",
           {{Language::Russian,
             "Изменяет порядок этапов конвейера рендера, чтобы летящие снаряды, лазеры и взрывы попадали в буфер отражений воды.\n"
             "Требуется перезапуск игры для применения."},
            {Language::Chinese,
             "调整渲染管线顺序，使飞行的弹道、激光和武器特效能够被捕获并在水面上形成反射。\n"
             "需要重启游戏以生效。"}})
            .c_str());

    ImGui::Unindent(20.0f);
  }

  ImGui::Spacing();
  ImGui::TextColored(
      ImVec4(1.0f, 0.78f, 0.28f, 1.0f), "%s",
      tr("(*) Requires game restart to apply",
         {{Language::Russian, "(*) Требуется перезапуск игры для применения"},
          {Language::Chinese, "(*) 需要重启游戏以生效"}})
          .data());

  if (restart_needed) {
    ImGui::OpenPopup("WaterRestartModal");
  }

  // Modal notification popup when toggling any setting
  ImVec2 center = ImGui::GetMainViewport()->GetCenter();
  ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

  if (ImGui::BeginPopupModal(
          "WaterRestartModal", nullptr,
          ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
    ImGui::Text(
        "%s",
        tr("Game Restart Required",
           {{Language::Russian, "Требуется перезапуск игры"},
            {Language::Chinese, "需要重启游戏"}})
            .data());
    ImGui::Separator();

    ImGui::Spacing();
    ImGui::Text(
        "%s",
        tr("Changes to water reflections and rendering pipeline will take full effect after restarting the game.",
           {{Language::Russian,
             "Изменения шейдера и конвейера отражений воды вступят в силу после перезапуска игры."},
            {Language::Chinese,
             "水面着色器和渲染管线的更改将在重启游戏后完全生效。"}})
            .data());
    ImGui::Spacing();

    ImGui::Separator();
    if (ImGui::Button("OK", ImVec2(120, 0))) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::SetItemDefaultFocus();
    ImGui::EndPopup();
  }
}
