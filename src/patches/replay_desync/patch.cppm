module;
#include <imgui.h>
#include <xbyak/xbyak.h>

export module patch.replay_desync;
import core;

struct PatchReplayDesync : public Xbyak::CodeGenerator {
  PatchReplayDesync()
      : Xbyak::CodeGenerator(11, reinterpret_cast<void *>(0x0053C71E)) {
    setProtectMode(PROTECT_RWE);
    constexpr int hasCommandSource = -0x8E;
    mov(byte[esp + 0x0A4 + hasCommandSource], 1);
    jmp(reinterpret_cast<const void *>(0x0053C7D7));
    nop();
    setProtectModeRE();
  }
};

export class ReplayDesyncPatch : public IPatch {
public:
  [[nodiscard]] std::string_view Name() const noexcept override {
    return tr("Replay Desync Fix",
              {{Language::Russian, "Исправление рассинхронизации повторов"},
               {Language::Chinese, "录像不同步修复"}});
  }

  [[nodiscard]] std::string_view Description() const noexcept override {
    return tr(
        "Fixes replay desync caused by CommandSourceTerminated making the game "
        "skip subsequent replay ticks when a player leaves.",
        {{Language::Russian,
          "Исправляет рассинхронизацию (десинхрон) в повторах, возникающую "
          "из-за "
          "CommandSourceTerminated при выходе игрока, когда игра пропускает "
          "следующий тик."},
         {Language::Chinese,
          "修复因 CommandSourceTerminated 导致玩家退出时跳过后续 Tick 并引发的"
          "录像不同步问题。"}});
  }

  void RenderUi() override {
    const char *url = "https://github.com/FAForever/FA-Binary-Patches/pull/172";
    const auto label =
        tr("GitHub: Pull Request #172 (FA-Binary-Patches)",
           {{Language::Russian, "GitHub: Пул-реквест #172 (FA-Binary-Patches)"},
            {Language::Chinese, "GitHub: 拉取请求 #172 (FA-Binary-Patches)"}});

    ImGui::TextLinkOpenURL(label.c_str(), url);
  }

  void Apply() override { PatchReplayDesync patch; }
};
