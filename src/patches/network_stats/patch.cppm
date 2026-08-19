module;
#include <xbyak/xbyak.h>

export module patch.network_stats;
import fa;
import core;
import std;

using namespace fa;

constexpr std::array<std::uint32_t, 21> kNetworkColors = {
    0xFF00FF00, 0xFF1AFF00, 0xFF33FF00, 0xFF4DFF00, 0xFF66FF00, 0xFF80FF00,
    0xFF99FF00, 0xFFB3FF00, 0xFFCCFF00, 0xFFE6FF00, 0xFFFFFF00, 0xFFFFE600,
    0xFFFFCC00, 0xFFFFB300, 0xFFFF9900, 0xFFFF8000, 0xFFFF6600, 0xFFFF4D00,
    0xFFFF3300, 0xFFF00000, 0xFFE00000,
};

int __fastcall PickColorForConnections(const fa::string *s, void *, int row,
                                       int index) {
  if (row > 0 && (index == 4 || index >= 7)) {
    int v = 0;
    if (s && s->data()) {
      std::from_chars(s->data(), s->data() + s->size(), v);
    }
    const auto clamped_idx = static_cast<std::size_t>(
        std::clamp(v, 0, static_cast<int>(kNetworkColors.size() - 1)));
    return static_cast<int>(kNetworkColors[clamped_idx]);
  }
  return static_cast<int>(0xFFFFFFFFu);
}

struct NetworkStatsHookBody : public Xbyak::CodeGenerator {
  NetworkStatsHookBody() {
    fstp(dword[esp]);
    mov(eax, dword[esp + 0x48]);
    pushfd();
    push(ecx);
    push(edx);
    push(esi);
    push(eax);
    call(reinterpret_cast<const void *>(PickColorForConnections));
    pop(edx);
    pop(ecx);
    popfd();
    push(eax);
    jmp(reinterpret_cast<const void *>(0x0073EFC1));
  }
};

struct PatchNetworkStatsJump : public Xbyak::CodeGenerator {
  PatchNetworkStatsJump(const void *trampoline_address)
      : Xbyak::CodeGenerator(5, reinterpret_cast<void *>(0x0073EFBC)) {
    setProtectMode(PROTECT_RWE);
    jmp(trampoline_address);
    setProtectModeRE();
  }
};

export class NetworkStatsPatch : public IPatch {
public:
  [[nodiscard]] std::string_view Name() const noexcept override {
    return tr("Network Stats", {{Language::Russian, "Сетевая статистика"},
                                {Language::Chinese, "网络统计"}});
  }

  [[nodiscard]] std::string_view Description() const noexcept override {
    return tr(
        "Colors connectivity stats dynamically: green for good connection, red "
        "for lagging players.",
        {{Language::Russian,
          "Окрашивает пинг и задержку в окне подключений: зелёный — стабильное "
          "соединение, красный — игрок лагает."},
         {Language::Chinese,
          "动态着色连接界面统计数据：绿色表示连接良好，红色表示玩家卡顿。"}});
  }

  void Apply() override {
    static NetworkStatsHookBody
        body; // Keep the generated JIT code buffer alive in memory across the
              // process lifetime
    PatchNetworkStatsJump patch(body.getCode<const void *>());
  }
};
