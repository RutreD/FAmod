// NOTE: AI-GENERATED, CAREFUL. NO TESTING DONE.
module;
#include <Windows.h>
#include <imgui.h>

export module core:hotkey;
import std;
import :i18n;

export namespace core::hotkey {

class Hotkey {
public:
  std::uint32_t value{0};

  constexpr Hotkey() noexcept = default;
  constexpr explicit Hotkey(std::uint32_t v) noexcept : value(v) {}
  constexpr Hotkey(std::uint32_t vk, bool ctrl, bool shift, bool alt) noexcept {
    value = vk & 0xFF;
    if (ctrl) value |= (1 << 8);
    if (shift) value |= (1 << 9);
    if (alt) value |= (1 << 10);
  }

  [[nodiscard]] constexpr std::uint32_t VkCode() const noexcept { return value & 0xFF; }
  [[nodiscard]] constexpr bool HasCtrl() const noexcept { return (value & (1 << 8)) != 0; }
  [[nodiscard]] constexpr bool HasShift() const noexcept { return (value & (1 << 9)) != 0; }
  [[nodiscard]] constexpr bool HasAlt() const noexcept { return (value & (1 << 10)) != 0; }
  [[nodiscard]] constexpr bool IsEmpty() const noexcept { return value == 0; }

  [[nodiscard]] bool IsPressed(UINT msg, WPARAM wp) const noexcept {
    if (msg != WM_KEYDOWN && msg != WM_SYSKEYDOWN) {
      return false;
    }
    if (wp != VkCode()) {
      return false;
    }

    const bool ctrl_pressed = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
    const bool shift_pressed = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
    const bool alt_pressed = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;

    return ctrl_pressed == HasCtrl() && shift_pressed == HasShift() && alt_pressed == HasAlt();
  }

  [[nodiscard]] std::string ToString() const {
    if (IsEmpty()) {
      return std::string(tr("None", {{Language::Russian, "Нет"}, {Language::Chinese, "无"}}).view);
    }
    std::string result;
    if (HasCtrl()) result += "Ctrl+";
    if (HasShift()) result += "Shift+";
    if (HasAlt()) result += "Alt+";

    const auto vk = VkCode();
    if (vk >= '0' && vk <= '9') {
      result += static_cast<char>(vk);
    } else if (vk >= 'A' && vk <= 'Z') {
      result += static_cast<char>(vk);
    } else {
      switch (vk) {
        case VK_OEM_3: result += "~"; break;
        case VK_OEM_MINUS: result += "-"; break;
        case VK_OEM_PLUS: result += "="; break;
        case VK_OEM_4: result += "["; break;
        case VK_OEM_6: result += "]"; break;
        case VK_OEM_1: result += ";"; break;
        case VK_OEM_7: result += "'"; break;
        case VK_OEM_COMMA: result += ","; break;
        case VK_OEM_PERIOD: result += "."; break;
        case VK_OEM_2: result += "/"; break;
        case VK_OEM_5: result += "\\"; break;
        case VK_SPACE: result += "Space"; break;
        case VK_RETURN: result += "Enter"; break;
        case VK_TAB: result += "Tab"; break;
        case VK_BACK: result += "Backspace"; break;
        case VK_ESCAPE: result += "Esc"; break;
        case VK_UP: result += "Up"; break;
        case VK_DOWN: result += "Down"; break;
        case VK_LEFT: result += "Left"; break;
        case VK_RIGHT: result += "Right"; break;
        case VK_INSERT: result += "Ins"; break;
        case VK_DELETE: result += "Del"; break;
        case VK_HOME: result += "Home"; break;
        case VK_END: result += "End"; break;
        case VK_PRIOR: result += "PgUp"; break;
        case VK_NEXT: result += "PgDn"; break;
        case VK_F1: result += "F1"; break;
        case VK_F2: result += "F2"; break;
        case VK_F3: result += "F3"; break;
        case VK_F4: result += "F4"; break;
        case VK_F5: result += "F5"; break;
        case VK_F6: result += "F6"; break;
        case VK_F7: result += "F7"; break;
        case VK_F8: result += "F8"; break;
        case VK_F9: result += "F9"; break;
        case VK_F10: result += "F10"; break;
        case VK_F11: result += "F11"; break;
        case VK_F12: result += "F12"; break;
        case VK_NUMPAD0: result += "Num 0"; break;
        case VK_NUMPAD1: result += "Num 1"; break;
        case VK_NUMPAD2: result += "Num 2"; break;
        case VK_NUMPAD3: result += "Num 3"; break;
        case VK_NUMPAD4: result += "Num 4"; break;
        case VK_NUMPAD5: result += "Num 5"; break;
        case VK_NUMPAD6: result += "Num 6"; break;
        case VK_NUMPAD7: result += "Num 7"; break;
        case VK_NUMPAD8: result += "Num 8"; break;
        case VK_NUMPAD9: result += "Num 9"; break;
        case VK_MULTIPLY: result += "Num *"; break;
        case VK_ADD: result += "Num +"; break;
        case VK_SEPARATOR: result += "Num ,"; break;
        case VK_SUBTRACT: result += "Num -"; break;
        case VK_DECIMAL: result += "Num ."; break;
        case VK_DIVIDE: result += "Num /"; break;
        case VK_PAUSE: result += "Pause"; break;
        case VK_CAPITAL: result += "CapsLock"; break;
        case VK_SCROLL: result += "ScrollLock"; break;
        default:
          result += std::format("VK_{:02X}", vk);
          break;
      }
    }
    return result;
  }
};

namespace capture {
  inline Hotkey* target_hotkey = nullptr;
  inline bool just_captured = false;
}

[[nodiscard]] bool IsCapturing() noexcept {
  return capture::target_hotkey != nullptr;
}

// Feeds the WndProc message to the capturing system.
// Returns true if the message was consumed.
bool FeedCapture(UINT msg, WPARAM wp) noexcept {
  if (!IsCapturing()) return false;

  if (msg != WM_KEYDOWN && msg != WM_SYSKEYDOWN) {
    return false;
  }

  const auto vk = static_cast<std::uint32_t>(wp);
  if (vk == VK_CONTROL || vk == VK_SHIFT || vk == VK_MENU || 
      vk == VK_LWIN || vk == VK_RWIN || 
      vk == VK_LCONTROL || vk == VK_RCONTROL || 
      vk == VK_LSHIFT || vk == VK_RSHIFT || 
      vk == VK_LMENU || vk == VK_RMENU) {
      return true; // Ignore modifier only presses, but swallow them to prevent game processing
  }

  if (vk == VK_ESCAPE) {
    // Cancel capture
    capture::target_hotkey = nullptr;
    return true;
  }

  if (vk == VK_BACK || vk == VK_DELETE) {
    // Clear hotkey
    if (capture::target_hotkey) {
      *capture::target_hotkey = Hotkey(0);
      capture::just_captured = true;
    }
    capture::target_hotkey = nullptr;
    return true;
  }

  const bool ctrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
  const bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
  const bool alt = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;

  if (capture::target_hotkey) {
    *capture::target_hotkey = Hotkey(vk, ctrl, shift, alt);
    capture::just_captured = true;
  }
  
  capture::target_hotkey = nullptr;
  return true;
}

bool RenderUi(const char* label, Hotkey& hotkey) {
  bool changed = false;

  ImGui::PushID(&hotkey);
  
  std::string btn_text = hotkey.ToString();
  const bool is_waiting = (capture::target_hotkey == &hotkey);

  if (is_waiting) {
    btn_text = "...";
    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
  }

  ImGui::TextDisabled("%s", label);
  ImGui::SameLine();

  if (ImGui::Button(btn_text.c_str(), ImVec2(150.0f, 0.0f))) {
    if (is_waiting) {
      capture::target_hotkey = nullptr; // Cancel
    } else {
      capture::target_hotkey = &hotkey;
      capture::just_captured = false;
    }
  }
  ImGui::SetItemTooltip("%s", std::string(tr("Click to rebind. Press Esc to cancel, Delete or Backspace to clear.",
                                             {{Language::Russian, "Нажмите для переназначения. Esc для отмены, Delete или Backspace для очистки."},
                                              {Language::Chinese, "点击重新绑定。按 Esc 取消，按 Delete 或 Backspace 清除。"}}).view).c_str());

  if (is_waiting) {
    ImGui::PopStyleColor();
  }

  if (capture::just_captured && !is_waiting && capture::target_hotkey == nullptr) {
      changed = true;
      capture::just_captured = false;
  }

  ImGui::PopID();
  return changed;
}

} // namespace core::hotkey
