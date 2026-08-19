module;
#include <imgui.h>

export module core:patch;
export import :settings;
export import :i18n;
import std;

export class IPatch {
public:
  IPatch() = default;
  virtual ~IPatch() = default;

  IPatch(const IPatch &) = delete;
  IPatch &operator=(const IPatch &) = delete;
  IPatch(IPatch &&) = default;
  IPatch &operator=(IPatch &&) = default;

  [[nodiscard]] virtual std::string_view Name() const noexcept = 0;
  [[nodiscard]] virtual std::string_view Description() const noexcept = 0;
  virtual void Apply() = 0;

  virtual void RenderUi() {
    ImGui::PushStyleColor(ImGuiCol_Text,
                          ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
    ImGui::TextWrapped(
        "%s",
        tr("This patch has no configurable settings.",
           {{Language::Russian, "У этого патча нет настраиваемых параметров."},
            {Language::Chinese, "该补丁没有可配置的设置。"}})
            .c_str());
    ImGui::PopStyleColor();
  }

  virtual void BindSettings(SettingsBinder & /*binder*/) {}
};
