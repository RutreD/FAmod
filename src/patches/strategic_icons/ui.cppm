module;
#include <imgui.h>

module patch.strategic_icons;
import core;

void StrategicIconsPatch::RenderUi() {
  ImGui::SliderFloat(
      tr("Icon scale",
         {{Language::Russian, "Масштаб иконок"},
          {Language::Chinese, "图标缩放"}}),
      &scale_factor_, 0.5f, 3.0f, "%.2fx");
  ImGui::SetItemTooltip(
      "%s",
      tr("Multiplier for the on-screen size of strategic icons.",
         {{Language::Russian,
           "Множитель экранного размера стратегических иконок."},
          {Language::Chinese, "战略图标在屏幕上的缩放倍率。"}})
          .c_str());

  ImGui::Checkbox(
      tr("Normalize HD textures",
         {{Language::Russian, "Нормализация HD-текстур"},
          {Language::Chinese, "高清纹理规格化"}}),
      &normalize_high_res_);
  ImGui::SetItemTooltip(
      "%s",
      tr("Prevents high-resolution icon textures (e.g. 128x128 or 256x256) "
         "from blowing up on screen, keeping clean vector-like sharpness.",
         {{Language::Russian,
           "Предотвращает раздувание иконок высокого разрешения (128x128, "
           "256x256 и т.д.) на весь экран, сохраняя идеальную четкость "
           "линий."},
          {Language::Chinese,
           "防止高分辨率图标纹理在屏幕上过大，保持矢量级清晰度。"}})
          .c_str());

  if (normalize_high_res_) {
    ImGui::SliderFloat(
        tr("Base pixel size",
           {{Language::Russian, "Базовый размер (px)"},
            {Language::Chinese, "基础像素尺寸"}}),
        &base_size_, 12.0f, 64.0f, "%.0f px");
    ImGui::SetItemTooltip(
        "%s",
        tr("Reference display size for normalized high-resolution icons (default "
           "is 24px).",
           {{Language::Russian,
             "Базовый размер отображения для нормализованных HD-иконок (по "
             "умолчанию 24px)."},
            {Language::Chinese, "规格化高清图标的基础参考像素（默认为24px）。"}})
            .c_str());
  }

  if (ImGui::Button(tr("Reset to default",
                       {{Language::Russian, "Сбросить по умолчанию"},
                        {Language::Chinese, "恢复默认"}}))) {
    scale_factor_ = 1.0f;
    normalize_high_res_ = true;
    base_size_ = 24.0f;
  }
}
