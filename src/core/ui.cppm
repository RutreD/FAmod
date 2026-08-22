module;
#include <Windows.h>
#include <imgui.h>

export module core:ui;
import :patch_registry;
import :settings;
import :i18n;
import :hotkey;
import imgui_hook;
import fa;
import std;

struct State {
  bool window_open = false;
  std::size_t selected_index = 0;
  float ui_scale = 1.0f;
  float pending_ui_scale = 1.0f;
  core::hotkey::Hotkey menu_hotkey{VK_OEM_3}; // Default to '~'
  ImGuiTextFilter filter;
};

State state;

export namespace core::ui {

[[nodiscard]] bool IsOpen() noexcept { return state.window_open; }
void SetOpen(bool open) noexcept { state.window_open = open; }

void ApplyStyle(float scale) {
  scale = std::clamp(scale, 1.0f, 5.0f);
  ImGuiStyle &s = ImGui::GetStyle();
  s = ImGuiStyle();

  s.WindowRounding = 8.0f;
  s.ChildRounding = 6.0f;
  s.FrameRounding = 5.0f;
  s.PopupRounding = 6.0f;
  s.ScrollbarRounding = 4.0f;
  s.GrabRounding = 4.0f;
  s.TabRounding = 5.0f;

  s.WindowPadding = {14.0f, 14.0f};
  s.FramePadding = {8.0f, 5.0f};
  s.ItemSpacing = {8.0f, 7.0f};
  s.ItemInnerSpacing = {6.0f, 6.0f};
  s.IndentSpacing = 16.0f;
  s.ScrollbarSize = 12.0f;

  s.WindowBorderSize = 1.0f;
  s.ChildBorderSize = 1.0f;
  s.FrameBorderSize = 1.0f;
  s.PopupBorderSize = 1.0f;

  ImVec4 *c = s.Colors;
  c[ImGuiCol_Text] = ImVec4(0.92f, 0.93f, 0.95f, 1.00f);
  c[ImGuiCol_TextDisabled] = ImVec4(0.42f, 0.45f, 0.50f, 1.00f);
  c[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.09f, 0.96f);
  c[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.10f, 0.12f, 0.65f);
  c[ImGuiCol_PopupBg] = ImVec4(0.11f, 0.11f, 0.13f, 0.98f);
  c[ImGuiCol_Border] = ImVec4(0.19f, 0.20f, 0.23f, 0.85f);
  c[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

  c[ImGuiCol_FrameBg] = ImVec4(0.13f, 0.14f, 0.16f, 0.90f);
  c[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.19f, 0.22f, 1.00f);
  c[ImGuiCol_FrameBgActive] = ImVec4(0.22f, 0.24f, 0.28f, 1.00f);

  c[ImGuiCol_TitleBg] = ImVec4(0.07f, 0.07f, 0.08f, 1.00f);
  c[ImGuiCol_TitleBgActive] = ImVec4(0.09f, 0.10f, 0.11f, 1.00f);
  c[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.11f, 0.12f, 1.00f);

  c[ImGuiCol_ScrollbarBg] = ImVec4(0.08f, 0.08f, 0.09f, 0.30f);
  c[ImGuiCol_ScrollbarGrab] = ImVec4(0.22f, 0.23f, 0.26f, 0.80f);
  c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.28f, 0.30f, 0.34f, 0.90f);
  c[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.35f, 0.38f, 0.43f, 1.00f);

  c[ImGuiCol_CheckMark] = ImVec4(0.40f, 0.65f, 0.95f, 1.00f);
  c[ImGuiCol_SliderGrab] = ImVec4(0.30f, 0.33f, 0.38f, 0.90f);
  c[ImGuiCol_SliderGrabActive] = ImVec4(0.45f, 0.50f, 0.58f, 1.00f);

  c[ImGuiCol_Button] = ImVec4(0.15f, 0.16f, 0.18f, 0.85f);
  c[ImGuiCol_ButtonHovered] = ImVec4(0.22f, 0.24f, 0.27f, 1.00f);
  c[ImGuiCol_ButtonActive] = ImVec4(0.27f, 0.29f, 0.33f, 1.00f);

  c[ImGuiCol_Header] = ImVec4(0.18f, 0.20f, 0.23f, 0.70f);
  c[ImGuiCol_HeaderHovered] = ImVec4(0.24f, 0.26f, 0.30f, 0.85f);
  c[ImGuiCol_HeaderActive] = ImVec4(0.28f, 0.31f, 0.36f, 1.00f);

  c[ImGuiCol_Separator] = ImVec4(0.18f, 0.19f, 0.22f, 0.70f);
  c[ImGuiCol_SeparatorHovered] = ImVec4(0.35f, 0.38f, 0.44f, 0.80f);
  c[ImGuiCol_SeparatorActive] = ImVec4(0.45f, 0.50f, 0.58f, 1.00f);

  c[ImGuiCol_Tab] = ImVec4(0.12f, 0.13f, 0.15f, 0.80f);
  c[ImGuiCol_TabHovered] = ImVec4(0.22f, 0.24f, 0.28f, 0.85f);
  c[ImGuiCol_TabSelected] = ImVec4(0.18f, 0.20f, 0.23f, 1.00f);
  c[ImGuiCol_TabSelectedOverline] = ImVec4(0.40f, 0.65f, 0.95f, 1.00f);

  s.ScaleAllSizes(scale);
  s.FontScaleMain = scale;
}

void LoadDefaultFont() {
  ImGuiIO &io = ImGui::GetIO();

  std::string win_dir;
  win_dir.resize_and_overwrite(MAX_PATH, [](char *buf, std::size_t size) {
    const UINT len = GetWindowsDirectoryA(buf, static_cast<UINT>(size));
    if (len == 0 || len >= size)
      return 0u;
    return len;
  });

  if (!win_dir.empty()) {
    const std::string fonts_dir = win_dir + "\\Fonts\\";

    // Primary font (Latin, Cyrillic)
    const std::array primary_fonts = {
        fonts_dir + "segoeui.ttf",
        fonts_dir + "tahoma.ttf",
        fonts_dir + "arial.ttf",
    };

    bool primary_loaded = false;
    for (const auto &font_path : primary_fonts) {
      if (std::filesystem::exists(font_path)) {
        ImFontConfig config;
        config.OversampleH = 2;
        config.OversampleV = 1;
        io.Fonts->AddFontFromFileTTF(font_path.c_str(), 16.0f, &config);
        primary_loaded = true;
        break;
      }
    }

    if (!primary_loaded) {
      io.Fonts->AddFontDefaultVector();
    }

    // Merge Chinese / CJK fallback font into the primary font.
    // In Dear ImGui 1.92+, glyph ranges are no longer required because
    // the dynamic font system rasterizes needed glyphs on demand.
    const std::array cjk_fonts = {
        fonts_dir + "msyh.ttc",    // Microsoft YaHei (Windows Vista+)
        fonts_dir + "msyhl.ttc",   // Microsoft YaHei Light
        fonts_dir + "simhei.ttf",  // SimHei
        fonts_dir + "simsun.ttc",  // SimSun
        fonts_dir + "mingliu.ttc", // MingLiU
    };

    for (const auto &cjk_path : cjk_fonts) {
      if (std::filesystem::exists(cjk_path)) {
        ImFontConfig merge_config;
        merge_config.MergeMode = true;
        merge_config.OversampleH = 2;
        merge_config.OversampleV = 1;
        merge_config.FontNo = 0;
        io.Fonts->AddFontFromFileTTF(cjk_path.c_str(), 16.0f, &merge_config);
        break;
      }
    }
  } else {
    io.Fonts->AddFontDefaultVector();
  }
}

void Initialize() {
  auto &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
  LoadDefaultFont();

  auto &settings = GetSettings();
  auto loader = settings.Loader();
  std::string lang_code;
  loader.Bind("UI.language", lang_code, std::string("en"));
  core::i18n::SetLanguage(core::i18n::LanguageFromCode(lang_code));

  loader.Bind("UI.scale", state.ui_scale, 1.0f);
  state.pending_ui_scale = state.ui_scale;
  ApplyStyle(state.ui_scale);

  loader.Bind("UI.menu_hotkey", state.menu_hotkey.value,
              static_cast<std::uint32_t>(VK_OEM_3));

  ImGuiHook::OnWndProc += [](HWND, UINT msg, WPARAM wp, LPARAM) -> bool {
    if (core::hotkey::FeedCapture(msg, wp)) {
      return true;
    }

    if (state.menu_hotkey.IsPressed(msg, wp)) {
      state.window_open = !state.window_open;
    }
    return false;
  };
  static fa::ConDescReg cmd_open_menu{"famod", "Opens the FAmod UI",
                                      &state.window_open};
}

void Render() {
  if (!state.window_open)
    return;

  auto &registry = PatchRegistry::Instance();
  if (registry.Empty())
    return;

  if (state.selected_index >= registry.PatchCount()) {
    state.selected_index = registry.PatchCount() - 1;
  }

  const float scale = state.ui_scale;
  ImGui::SetNextWindowSize({680.0f * scale, 400.0f * scale},
                           ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSizeConstraints({680.0f * scale, 400.0f * scale},
                                      {9999.0f, 9999.0f});

  if (!ImGui::Begin("FAmod - Patch Manager", &state.window_open)) {
    ImGui::End();
    return;
  }

  float max_patch_name_width = 0.0f;
  for (const auto &patch : registry.Patches()) {
    const auto name = patch->Name();
    const float w =
        ImGui::CalcTextSize(name.data(), name.data() + name.size()).x;
    max_patch_name_width = std::max(max_patch_name_width, w);
  }
  const float min_sidebar_width = 160.0f * scale;
  const float sidebar_width =
      std::max(min_sidebar_width,
               max_patch_name_width + ImGui::GetStyle().FramePadding.x * 2.0f +
                   ImGui::GetStyle().ItemSpacing.x * 2.0f + 24.0f * scale);

  const float footer_height =
      ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y * 2.0f;
  const float content_height = -footer_height;

  ImGui::BeginChild("##patch_list", {sidebar_width, content_height},
                    ImGuiChildFlags_Borders);

  ImGui::SetNextItemWidth(-1.0f);
  if (ImGui::InputTextWithHint(
          "##patch_search",
          tr("Search...",
             {{Language::Russian, "Поиск..."}, {Language::Chinese, "搜索..."}}),
          state.filter.InputBuf, IM_ARRAYSIZE(state.filter.InputBuf))) {
    state.filter.Build();
  }

  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();

  bool any_visible = false;
  for (auto [i, patch] : std::views::enumerate(registry.Patches())) {
    const auto name = patch->Name();
    const auto desc = patch->Description();
    const bool match =
        state.filter.PassFilter(name.data(), name.data() + name.size()) ||
        (!desc.empty() &&
         state.filter.PassFilter(desc.data(), desc.data() + desc.size()));
    if (!match)
      continue;

    any_visible = true;
    const auto idx = static_cast<std::size_t>(i);
    const bool is_selected = (idx == state.selected_index);
    if (ImGui::Selectable(name.data(), is_selected)) {
      state.selected_index = idx;
    }
    if (is_selected)
      ImGui::SetItemDefaultFocus();
  }

  if (!any_visible) {
    ImGui::PushStyleColor(ImGuiCol_Text,
                          ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
    ImGui::TextWrapped("%s", tr("No matching patches",
                                {{Language::Russian, "Патчи не найдены"},
                                 {Language::Chinese, "未找到匹配的补丁"}})
                                 .c_str());
    ImGui::PopStyleColor();
  }

  ImGui::EndChild();

  ImGui::SameLine();

  ImGui::BeginChild("##patch_settings", {0.0f, content_height},
                    ImGuiChildFlags_Borders);

  IPatch &patch = registry.PatchAt(state.selected_index);

  const auto patch_name = patch.Name();
  ImGui::TextUnformatted(patch_name.data(),
                         patch_name.data() + patch_name.size());

  const auto desc = patch.Description();
  if (!desc.empty()) {
    ImGui::PushStyleColor(ImGuiCol_Text,
                          ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextUnformatted(desc.data(), desc.data() + desc.size());
    ImGui::PopTextWrapPos();
    ImGui::PopStyleColor();
  }

  ImGui::Separator();
  ImGui::Spacing();

  patch.RenderUi();

  if (ImGui::IsWindowHovered() &&
      ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
    auto &settings = GetSettings();
    registry.SavePatch(patch, settings);
    settings.Save(kSettingsPath);
  }

  ImGui::EndChild();

  ImGui::Spacing();
  ImGui::AlignTextToFramePadding();
  ImGui::TextDisabled("%s", tr("UI Scale:", {{Language::Russian, "Масштаб:"},
                                             {Language::Chinese, "界面缩放:"}})
                                .c_str());
  ImGui::SameLine();

  bool scale_changed = false;
  const float old_scale = state.ui_scale;

  ImGui::SetNextItemWidth(110.0f * scale);
  ImGui::SliderFloat("##ui_scale", &state.pending_ui_scale, 1.0f, 5.0f,
                     "%.2fx");
  if (ImGui::IsItemDeactivatedAfterEdit()) {
    state.ui_scale = state.pending_ui_scale;
    scale_changed = true;
  }

  ImGui::SameLine();
  ImGui::Spacing();
  ImGui::SameLine();
  ImGui::TextDisabled("%s", tr("Language:", {{Language::Russian, "Язык:"},
                                             {Language::Chinese, "语言:"}})
                                .c_str());
  ImGui::SameLine();
  ImGui::SetNextItemWidth(100.0f * scale);
  const Language current_lang = core::i18n::GetLanguage();
  const auto current_idx = static_cast<std::size_t>(current_lang);
  const auto &current_info = (current_idx < kSupportedLanguages.size())
                                 ? kSupportedLanguages[current_idx]
                                 : kSupportedLanguages[0];

  if (ImGui::BeginCombo("##language_selector",
                        current_info.display_name.data())) {
    for (const auto &info : kSupportedLanguages) {
      const bool is_selected = (info.id == current_lang);
      if (ImGui::Selectable(info.display_name.data(), is_selected)) {
        core::i18n::SetLanguage(info.id);
        auto &settings = GetSettings();
        auto saver = settings.Saver();
        std::string code = std::string(info.code);
        saver.Bind("UI.language", code);
        settings.Save(kSettingsPath);
      }
      if (is_selected)
        ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }

  ImGui::SameLine();
  if (core::hotkey::RenderUi(
          tr("Menu Hotkey:", {{Language::Russian, "Горячая клавиша меню:"},
                              {Language::Chinese, "菜单快捷键:"}})
              .c_str(),
          state.menu_hotkey)) {
    auto &settings = GetSettings();
    settings.Saver().Bind("UI.menu_hotkey", state.menu_hotkey.value);
    settings.Save(kSettingsPath);
  }

  ImGui::SameLine();
  ImGui::TextDisabled("(?)");
  ImGui::SetItemTooltip(
      "%s",
      tr("To unbind the hotkey, press Delete or Backspace while "
         "rebinding.\nAlternatively, type 'famod' in the game console (~).",
         {{Language::Russian,
           "Чтобы удалить хоткей, нажмите Delete или Backspace во время "
           "переназначения.\nТакже можете ввести 'famod' в консоли игры (~)."},
          {Language::Chinese,
           "要取消绑定快捷键，请在重新绑定时按 Delete 或 "
           "Backspace。\n或者在游戏控制台 (~) 中输入 'famod'。"}})
          .c_str());

  if (scale_changed && old_scale > 0.0f) {
    ApplyStyle(state.ui_scale);
    const ImVec2 cur_size = ImGui::GetWindowSize();
    const float ratio = state.ui_scale / old_scale;
    ImGui::SetWindowSize(ImVec2(cur_size.x * ratio, cur_size.y * ratio));
    auto &settings = GetSettings();
    auto saver = settings.Saver();
    saver.Bind("UI.scale", state.ui_scale);
    settings.Save(kSettingsPath);
  }

  ImGui::End();
}

} // namespace core::ui
