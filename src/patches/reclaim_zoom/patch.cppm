module;
#include <imgui.h>
#include <xbyak/xbyak.h>

export module patch.reclaim_zoom;
import std;
import fa;
import core;

using namespace fa;

export class ReclaimZoomPatch : public IPatch {
public:
  inline static float reclaim_zoom_threshold_{150.0f};

  [[nodiscard]] std::string_view Name() const noexcept override {
    return tr("Reclaim Prop Selection Zoom",
              {{Language::Russian, "Дистанция выбора пропсов для реклейма"},
               {Language::Chinese, "残骸回收选择缩放距离"}});
  }

  [[nodiscard]] std::string_view Description() const noexcept override {
    return tr(
        "Adjusts the camera zoom threshold for selecting props (trees, rocks, "
        "wrecks) to reclaim. Can affect framerate.",
        {{Language::Russian,
          "Регулирует порог приближения камеры для выбора пропсов (деревья, "
          "камни, остовы) для реклейма. Может влиять на FPS."},
         {Language::Chinese,
          "调整框选残骸/道具（树木、岩石、残骸）进行回收的相机缩放距离阈值。"
          "可能会影响帧率。"}});
  }

  void Apply() override;
  void RenderUi() override;

  void BindSettings(SettingsBinder &b) override {
    b.Bind("ReclaimZoom.threshold", reclaim_zoom_threshold_, 150.0f);
  }
};

void ReclaimZoomPatch::RenderUi() {
  ImGui::SliderFloat(
      tr("Zoom Threshold", {{Language::Russian, "Порог приближения"},
                            {Language::Chinese, "缩放阈值"}})
          .c_str(),
      &reclaim_zoom_threshold_, 10.0f, 999.0f, "%.0f");

  ImGui::SetItemTooltip(
      "%s",
      tr("Camera zoom threshold for selecting props (trees, wrecks, rocks) for "
         "reclaim.\n"
         "Can affect framerate.\n"
         "Default: 150, Maximum: 999.",
         {{Language::Russian, "Порог приближения камеры для выбора пропсов "
                              "(деревья, остовы, камни) для реклейма.\n"
                              "Может влиять на FPS.\n"
                              "По умолчанию: 150, Максимум: 999."},
          {Language::Chinese,
           "框选残骸/道具（树木、岩石、残骸）进行回收的相机缩放距离阈值。\n"
           "可能会影响帧率。\n"
           "默认值：150，最大值：999。"}})
          .c_str());
}

struct PatchReclaimZoomInstruction : public Xbyak::CodeGenerator {
  explicit PatchReclaimZoomInstruction(const void *p_threshold)
      : Xbyak::CodeGenerator(6, reinterpret_cast<void *>(0x0086F754)) {
    setProtectMode(PROTECT_RWE);
    fld(dword[p_threshold]);
    setProtectModeRE();
  }
};

int LuaGetReclaimZoomThreshold(fa::lua_State *L) {
  fa::lua_pushnumber(L, ReclaimZoomPatch::reclaim_zoom_threshold_);
  return 1;
}

void ReclaimZoomPatch::Apply() {
  PatchReclaimZoomInstruction patch(&reclaim_zoom_threshold_);
  static UIRegFunc reg_get(
      "GetReclaimZoomThreshold",
      "Returns minimum zoom distance for selecting reclaim props",
      &LuaGetReclaimZoomThreshold);

  core::events::OnMemoryMapFile +=
      [](fa::ConstMemBuffer *buffer, const char *fname) {
        if (fname && std::string_view(fname).ends_with(
                         "\\lua\\ui\\controls\\worldview.lua")) {
          buffer->append(R"(
    WorldView.CanIssueReclaimOrders = function(self)
        if not self.Camera then
            self.Camera = GetCamera('WorldCamera')
        end

        return self.Camera:GetZoom() < GetReclaimZoomThreshold()
    end
)");
        }
      };
}
