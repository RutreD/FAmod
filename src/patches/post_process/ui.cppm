module;
#include <imgui.h>

module patch.post_process:ui;

import std;
import core;
import :settings;

namespace patch::post_process {

void RenderPostProcessUi() {
  ImGui::Checkbox(tr("Enable World Post-Processing",
                     {{Language::Russian, "Включить пост-процессинг мира"},
                      {Language::Chinese, "启用世界后处理"}}),
                  &g_settings.enabled);

  if (!g_settings.enabled) {
    ImGui::BeginDisabled();
  }

  ImGui::Spacing();

  // 1. Presets & Quick Actions
  constexpr auto kPresetNames = std::to_array<const char *>({
      "Custom",
      "Default (Neutral)",
      "Cinematic (ACES)",
      "Teal & Orange",
      "Vibrant & Clear",
      "Clarity & Punch",
      "Grimdark Battlefield",
      "Military (Bleach)",
      "Emerald Sci-Fi",
      "Cyberpunk Neon",
      "Studio Cinema",
      "Technicolor 1960s",
      "Kodak DPX Film",
      "Warm Sunlight (Desert)",
      "Cool Arctic (Tundra)",
  });

  int current_preset = std::to_underlying(g_settings.preset);
  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.65f);
  if (ImGui::Combo(tr("Preset", {{Language::Russian, "Пресет"},
                                 {Language::Chinese, "预设"}}),
                   &current_preset, kPresetNames.data(),
                   static_cast<int>(kPresetNames.size()))) {
    g_settings.ApplyPreset(static_cast<Preset>(current_preset));
  }
  ImGui::SameLine();
  if (ImGui::Button(tr("Reset", {{Language::Russian, "Сброс"},
                                 {Language::Chinese, "重置"}}))) {
    g_settings.ApplyPreset(Preset::Default);
  }

  ImGui::Separator();

  // Helper lambdas to modify settings and mark preset as Custom
  auto CustomSliderFloat = [](const char *label, float *value, float min,
                              float max, const char *format = "%.2f") {
    if (ImGui::SliderFloat(label, value, min, max, format)) {
      g_settings.preset = Preset::Custom;
    }
  };

  auto CustomCheckbox = [](const char *label, bool *value) {
    if (ImGui::Checkbox(label, value)) {
      g_settings.preset = Preset::Custom;
    }
  };

  auto CustomColorEdit3 = [](const char *label, std::array<float, 3> &color) {
    if (ImGui::ColorEdit3(label, color.data())) {
      g_settings.preset = Preset::Custom;
    }
  };

  // 2. Primary / Most Frequently Used Controls
  ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s",
                     tr("Map & Scene Lighting",
                        {{Language::Russian, "Освещение и карта"},
                         {Language::Chinese, "地图与场景光照"}})
                         .c_str());

  CustomSliderFloat(tr("Brightness", {{Language::Russian, "Яркость"},
                                      {Language::Chinese, "亮度"}}),
                    &g_settings.brightness, -0.4f, 0.4f);

  CustomSliderFloat(tr("Contrast", {{Language::Russian, "Контраст"},
                                    {Language::Chinese, "对比度"}}),
                    &g_settings.contrast, 0.6f, 1.6f);

  CustomSliderFloat(tr("S-Curve Contrast",
                       {{Language::Russian, "S-кривая контраста"},
                        {Language::Chinese, "S曲线对比度"}}),
                    &g_settings.s_curve, 0.0f, 1.0f);

  CustomSliderFloat(
      tr("Black Level",
         {{Language::Russian, "Точка чёрного (Матовый чёрный)"},
          {Language::Chinese, "黑电平（哑光黑）"}}),
      &g_settings.black_level, -0.15f, 0.15f);

  CustomSliderFloat(
      tr("Vibrance", {{Language::Russian, "Сочность (Vibrance)"},
                      {Language::Chinese, "自然饱和度"}}),
      &g_settings.vibrance, -0.8f, 0.8f);

  CustomSliderFloat(tr("Exposure", {{Language::Russian, "Экспозиция"},
                                    {Language::Chinese, "曝光"}}),
                    &g_settings.exposure, -1.0f, 1.0f);

  CustomSliderFloat(tr("Gamma", {{Language::Russian, "Гамма"},
                                 {Language::Chinese, "伽马"}}),
                    &g_settings.gamma, 0.6f, 1.5f);

  CustomSliderFloat(tr("Saturation", {{Language::Russian, "Насыщенность"},
                                      {Language::Chinese, "饱和度"}}),
                    &g_settings.saturation, 0.2f, 1.8f);

  ImGui::Separator();

  // 3. Clarity & Anti-Aliasing
  ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s",
                     tr("Clarity & Anti-Aliasing",
                        {{Language::Russian, "Чёткость и сглаживание"},
                         {Language::Chinese, "清晰度与抗锯齿"}})
                         .c_str());

  CustomCheckbox(
      tr("FXAA (World Only)",
         {{Language::Russian, "Сглаживание FXAA (только мир)"},
          {Language::Chinese, "FXAA 抗锯齿（仅世界）"}}),
      &g_settings.fxaa_enabled);

  CustomSliderFloat(tr("AMD CAS Sharpness",
                       {{Language::Russian, "Резкость (AMD CAS)"},
                        {Language::Chinese, "AMD CAS 锐化"}}),
                    &g_settings.sharpening, 0.0f, 1.2f);

  ImGui::Separator();

  // 4. Atmosphere & Cinematic Effects
  ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s",
                     tr("Atmosphere & Cinematic Effects",
                        {{Language::Russian, "Атмосфера и киноэффекты"},
                         {Language::Chinese, "氛围与电影效果"}})
                         .c_str());

  CustomSliderFloat(
      tr("Technicolor 3-Strip",
         {{Language::Russian, "Плёнка Technicolor (Ретро-кино)"},
          {Language::Chinese, "特艺七彩胶片效果"}}),
      &g_settings.technicolor, 0.0f, 1.0f);

  CustomSliderFloat(
      tr("Cineon DPX Film",
         {{Language::Russian, "Плёночная плотность (Cineon DPX)"},
          {Language::Chinese, "Cineon DPX 胶片扫描色彩"}}),
      &g_settings.dpx_film, 0.0f, 1.0f);

  CustomSliderFloat(
      tr("Bleach Bypass",
         {{Language::Russian, "Серебряный киноэффект (Bleach Bypass)"},
          {Language::Chinese, "漂白效果（金属高对比）"}}),
      &g_settings.bleach_bypass, 0.0f, 1.0f);

  CustomSliderFloat(
      tr("Vignette", {{Language::Russian, "Виньетка (затемнение краёв)"},
                      {Language::Chinese, "晕影（边缘暗角）"}}),
      &g_settings.vignette_intensity, 0.0f, 1.0f);

  CustomSliderFloat(
      tr("Film Grain", {{Language::Russian, "Плёночное зерно (Film Grain)"},
                        {Language::Chinese, "胶片颗粒"}}),
      &g_settings.film_grain, 0.0f, 1.0f);

  CustomSliderFloat(
      tr("Color Temp",
         {{Language::Russian, "Цветовая температура (Тепло/Холод)"},
          {Language::Chinese, "色温 (冷/暖)"}}),
      &g_settings.color_temp, -0.8f, 0.8f);

  CustomSliderFloat(
      tr("Color Tint",
         {{Language::Russian, "Цветовой оттенок (Зелёный/Пурпурный)"},
          {Language::Chinese, "色调 (绿/品红)"}}),
      &g_settings.tint, -0.8f, 0.8f);

  constexpr auto kTonemapNames = std::to_array<const char *>({
      "None",
      "ACES Filmic",
      "Uncharted 2 (Hable)",
      "Reinhard-Jodie",
      "AgX Filmic",
      "Khronos PBR Neutral",
      "Uchimura (Gran Turismo)",
      "Lottes (AMD)",
  });
  int current_tonemap = std::to_underlying(g_settings.tonemap_mode);
  if (ImGui::Combo(tr("Tonemapping", {{Language::Russian, "Тонемаппинг"},
                                      {Language::Chinese, "色调映射"}}),
                   &current_tonemap, kTonemapNames.data(),
                   static_cast<int>(kTonemapNames.size()))) {
    g_settings.tonemap_mode = static_cast<TonemapMode>(current_tonemap);
    g_settings.preset = Preset::Custom;
  }

  ImGui::Separator();

  // 5. Fine Color Tuning & Split Toning
  if (ImGui::CollapsingHeader(
          tr("Advanced Color Grading (Split Toning)",
             {{Language::Russian, "Продвинутая стилизация (Split Toning)"},
              {Language::Chinese, "高级调色（色调分离）"}}))) {
    CustomSliderFloat(
        tr("Split Intensity", {{Language::Russian, "Интенсивность тонирования"},
                               {Language::Chinese, "色调分离强度"}}),
        &g_settings.split_intensity, 0.0f, 1.0f);

    if (g_settings.split_intensity > 0.001f) {
      CustomColorEdit3(tr("Shadows Tint", {{Language::Russian, "Цвет теней"},
                                           {Language::Chinese, "阴影颜色"}}),
                       g_settings.shadow_tint);

      CustomColorEdit3(tr("Highlights Tint",
                          {{Language::Russian, "Цвет светов"},
                           {Language::Chinese, "高光颜色"}}),
                       g_settings.highlight_tint);

      CustomSliderFloat(
          tr("Balance", {{Language::Russian, "Баланс теней/светов"},
                         {Language::Chinese, "阴影/高光平衡"}}),
          &g_settings.split_balance, -1.0f, 1.0f);
    }
  }

  if (!g_settings.enabled) {
    ImGui::EndDisabled();
  }
}

} // namespace patch::post_process
