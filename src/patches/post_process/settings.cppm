module patch.post_process:settings;

import std;

namespace patch::post_process {

enum class TonemapMode : std::uint8_t {
  None = 0,
  Aces = 1,
  Hable = 2,
  ReinhardJodie = 3,
  AgX = 4,
  KhronosNeutral = 5,
  Uchimura = 6,
  Lottes = 7,
};

enum class Preset : std::uint8_t {
  Custom = 0,
  Default = 1,
  Cinematic = 2,
  TealOrange = 3,
  Vibrant = 4,
  ClarityPunch = 5,
  Grimdark = 6,
  MilitaryBleach = 7,
  EmeraldSciFi = 8,
  Cyberpunk = 9,
  StudioCinema = 10,
  TechnicolorRetro = 11,
  KodakDPX = 12,
  WarmSunlight = 13,
  CoolArctic = 14,
};

struct PresetValues {
  bool fxaa_enabled{true};
  float brightness{0.0f};
  float contrast{1.0f};
  float saturation{1.0f};
  float vibrance{0.0f};
  float gamma{1.0f};
  float exposure{0.0f};
  float color_temp{0.0f};
  float tint{0.0f};
  TonemapMode tonemap_mode{TonemapMode::None};
  float split_intensity{0.0f};
  float split_balance{0.0f};
  std::array<float, 3> shadow_tint{0.92f, 0.96f, 1.05f};
  std::array<float, 3> highlight_tint{1.08f, 1.04f, 0.95f};
  float sharpening{0.0f};
  float vignette_intensity{0.0f};
  float film_grain{0.0f};
  float bleach_bypass{0.0f};
  float black_level{0.0f};
  float s_curve{0.0f};
  float technicolor{0.0f};
  float dpx_film{0.0f};
};

// Preset data table: kPresets[i] corresponds to Preset(i + 1), skipping Custom.
inline constexpr std::array<PresetValues, 14> kPresets = {
    // [1. Default] — Neutral baseline
    PresetValues{
        .fxaa_enabled = true,
        .brightness = 0.0f,
        .contrast = 1.0f,
        .saturation = 1.0f,
        .vibrance = 0.0f,
        .gamma = 1.0f,
        .exposure = 0.0f,
        .color_temp = 0.0f,
        .tint = 0.0f,
        .tonemap_mode = TonemapMode::None,
        .split_intensity = 0.0f,
        .split_balance = 0.0f,
        .shadow_tint = {0.92f, 0.96f, 1.05f},
        .highlight_tint = {1.08f, 1.04f, 0.95f},
        .sharpening = 0.0f,
        .vignette_intensity = 0.0f,
        .film_grain = 0.0f,
        .bleach_bypass = 0.0f,
        .black_level = 0.0f,
        .s_curve = 0.0f,
        .technicolor = 0.0f,
        .dpx_film = 0.0f,
    },

    // [2. Cinematic] — Rich filmic tone curve with ACES, warm highlights, soft shadows
    PresetValues{
        .fxaa_enabled = true,
        .brightness = 0.02f,
        .contrast = 1.06f,
        .saturation = 1.08f,
        .vibrance = 0.22f,
        .gamma = 1.02f,
        .exposure = 0.08f,
        .color_temp = 0.05f,
        .tint = -0.01f,
        .tonemap_mode = TonemapMode::Aces,
        .split_intensity = 0.25f,
        .split_balance = 0.05f,
        .shadow_tint = {0.92f, 0.96f, 1.05f},
        .highlight_tint = {1.08f, 1.04f, 0.95f},
        .sharpening = 0.35f,
        .vignette_intensity = 0.15f,
        .film_grain = 0.03f,
        .bleach_bypass = 0.0f,
        .black_level = 0.01f,
        .s_curve = 0.18f,
        .technicolor = 0.0f,
        .dpx_film = 0.0f,
    },

    // [3. Teal & Orange] — Hollywood blockbuster grade, crisp cool shadows & golden lights
    PresetValues{
        .fxaa_enabled = true,
        .brightness = 0.02f,
        .contrast = 1.08f,
        .saturation = 1.12f,
        .vibrance = 0.25f,
        .gamma = 1.02f,
        .exposure = 0.06f,
        .color_temp = 0.02f,
        .tint = 0.0f,
        .tonemap_mode = TonemapMode::Aces,
        .split_intensity = 0.38f,
        .split_balance = 0.08f,
        .shadow_tint = {0.84f, 0.96f, 1.10f},
        .highlight_tint = {1.14f, 1.04f, 0.88f},
        .sharpening = 0.40f,
        .vignette_intensity = 0.16f,
        .film_grain = 0.03f,
        .bleach_bypass = 0.0f,
        .black_level = 0.01f,
        .s_curve = 0.20f,
        .technicolor = 0.0f,
        .dpx_film = 0.0f,
    },

    // [4. Vibrant & Clear] — Bright, highly saturated, maximum tactical clarity with Khronos Neutral
    PresetValues{
        .fxaa_enabled = true,
        .brightness = 0.04f,
        .contrast = 1.05f,
        .saturation = 1.20f,
        .vibrance = 0.40f,
        .gamma = 1.04f,
        .exposure = 0.06f,
        .color_temp = 0.03f,
        .tint = 0.0f,
        .tonemap_mode = TonemapMode::KhronosNeutral,
        .split_intensity = 0.0f,
        .split_balance = 0.0f,
        .shadow_tint = {0.92f, 0.96f, 1.05f},
        .highlight_tint = {1.08f, 1.04f, 0.95f},
        .sharpening = 0.45f,
        .vignette_intensity = 0.08f,
        .film_grain = 0.0f,
        .bleach_bypass = 0.0f,
        .black_level = 0.0f,
        .s_curve = 0.10f,
        .technicolor = 0.15f,
        .dpx_film = 0.0f,
    },

    // [5. Clarity & Punch] — Crisp definition and dynamic punch with Uchimura GT clarity
    PresetValues{
        .fxaa_enabled = true,
        .brightness = 0.02f,
        .contrast = 1.10f,
        .saturation = 1.10f,
        .vibrance = 0.20f,
        .gamma = 1.02f,
        .exposure = 0.05f,
        .color_temp = 0.0f,
        .tint = 0.0f,
        .tonemap_mode = TonemapMode::Uchimura,
        .split_intensity = 0.15f,
        .split_balance = 0.0f,
        .shadow_tint = {0.94f, 0.96f, 1.04f},
        .highlight_tint = {1.06f, 1.04f, 0.96f},
        .sharpening = 0.50f,
        .vignette_intensity = 0.16f,
        .film_grain = 0.03f,
        .bleach_bypass = 0.0f,
        .black_level = 0.01f,
        .s_curve = 0.22f,
        .technicolor = 0.0f,
        .dpx_film = 0.0f,
    },

    // [6. Grimdark Battlefield] — Desaturated warzone ambiance with dense Lottes contrast
    PresetValues{
        .fxaa_enabled = true,
        .brightness = 0.03f,
        .contrast = 1.08f,
        .saturation = 0.80f,
        .vibrance = -0.05f,
        .gamma = 1.04f,
        .exposure = 0.04f,
        .color_temp = -0.06f,
        .tint = 0.02f,
        .tonemap_mode = TonemapMode::Lottes,
        .split_intensity = 0.30f,
        .split_balance = 0.0f,
        .shadow_tint = {0.88f, 0.94f, 1.02f},
        .highlight_tint = {1.05f, 0.98f, 0.92f},
        .sharpening = 0.50f,
        .vignette_intensity = 0.20f,
        .film_grain = 0.05f,
        .bleach_bypass = 0.25f,
        .black_level = 0.02f,
        .s_curve = 0.20f,
        .technicolor = 0.0f,
        .dpx_film = 0.0f,
    },

    // [7. Military (Bleach)] — Gritty metallic battlefield with Hable (Uncharted) film curve
    PresetValues{
        .fxaa_enabled = true,
        .brightness = 0.03f,
        .contrast = 1.08f,
        .saturation = 0.88f,
        .vibrance = 0.12f,
        .gamma = 1.02f,
        .exposure = 0.05f,
        .color_temp = -0.04f,
        .tint = 0.01f,
        .tonemap_mode = TonemapMode::Hable,
        .split_intensity = 0.25f,
        .split_balance = -0.02f,
        .shadow_tint = {0.90f, 0.94f, 1.04f},
        .highlight_tint = {1.06f, 1.02f, 0.95f},
        .sharpening = 0.45f,
        .vignette_intensity = 0.18f,
        .film_grain = 0.05f,
        .bleach_bypass = 0.35f,
        .black_level = 0.02f,
        .s_curve = 0.20f,
        .technicolor = 0.0f,
        .dpx_film = 0.0f,
    },

    // [8. Emerald Sci-Fi] — Cool emerald cyber atmosphere with AgX glowing shield protection
    PresetValues{
        .fxaa_enabled = true,
        .brightness = 0.02f,
        .contrast = 1.08f,
        .saturation = 1.02f,
        .vibrance = 0.22f,
        .gamma = 1.02f,
        .exposure = 0.05f,
        .color_temp = -0.06f,
        .tint = -0.08f,
        .tonemap_mode = TonemapMode::AgX,
        .split_intensity = 0.30f,
        .split_balance = -0.02f,
        .shadow_tint = {0.86f, 1.06f, 0.94f},
        .highlight_tint = {0.95f, 1.08f, 0.96f},
        .sharpening = 0.40f,
        .vignette_intensity = 0.18f,
        .film_grain = 0.04f,
        .bleach_bypass = 0.12f,
        .black_level = 0.01f,
        .s_curve = 0.18f,
        .technicolor = 0.0f,
        .dpx_film = 0.0f,
    },

    // [9. Cyberpunk Neon] — Vivid cyber-blue shadows, neon-magenta lights with AgX saturation
    PresetValues{
        .fxaa_enabled = true,
        .brightness = 0.03f,
        .contrast = 1.10f,
        .saturation = 1.20f,
        .vibrance = 0.38f,
        .gamma = 1.02f,
        .exposure = 0.06f,
        .color_temp = -0.03f,
        .tint = 0.05f,
        .tonemap_mode = TonemapMode::AgX,
        .split_intensity = 0.36f,
        .split_balance = 0.08f,
        .shadow_tint = {0.82f, 0.92f, 1.20f},
        .highlight_tint = {1.18f, 0.92f, 1.10f},
        .sharpening = 0.45f,
        .vignette_intensity = 0.18f,
        .film_grain = 0.04f,
        .bleach_bypass = 0.0f,
        .black_level = 0.01f,
        .s_curve = 0.20f,
        .technicolor = 0.15f,
        .dpx_film = 0.08f,
    },

    // [10. Studio Cinema] — Multi-stage grading (ACES + S-curve + Technicolor + DPX)
    PresetValues{
        .fxaa_enabled = true,
        .brightness = 0.03f,
        .contrast = 1.07f,
        .saturation = 1.10f,
        .vibrance = 0.28f,
        .gamma = 1.03f,
        .exposure = 0.06f,
        .color_temp = 0.02f,
        .tint = 0.0f,
        .tonemap_mode = TonemapMode::Aces,
        .split_intensity = 0.22f,
        .split_balance = 0.0f,
        .shadow_tint = {0.92f, 0.96f, 1.06f},
        .highlight_tint = {1.08f, 1.04f, 0.95f},
        .sharpening = 0.45f,
        .vignette_intensity = 0.16f,
        .film_grain = 0.03f,
        .bleach_bypass = 0.0f,
        .black_level = 0.01f,
        .s_curve = 0.22f,
        .technicolor = 0.20f,
        .dpx_film = 0.15f,
    },

    // [11. Technicolor 1960s] — Classic vintage 3-strip color film with analog Hable curve
    PresetValues{
        .fxaa_enabled = true,
        .brightness = 0.03f,
        .contrast = 1.08f,
        .saturation = 1.14f,
        .vibrance = 0.28f,
        .gamma = 1.02f,
        .exposure = 0.06f,
        .color_temp = 0.05f,
        .tint = 0.0f,
        .tonemap_mode = TonemapMode::Hable,
        .split_intensity = 0.18f,
        .split_balance = 0.0f,
        .shadow_tint = {0.94f, 0.96f, 1.04f},
        .highlight_tint = {1.08f, 1.04f, 0.94f},
        .sharpening = 0.40f,
        .vignette_intensity = 0.18f,
        .film_grain = 0.04f,
        .bleach_bypass = 0.0f,
        .black_level = 0.01f,
        .s_curve = 0.20f,
        .technicolor = 0.45f,
        .dpx_film = 0.0f,
    },

    // [12. Kodak DPX Film] — Rich Kodak/Cineon scan density with Reinhard-Jodie luma rolloff
    PresetValues{
        .fxaa_enabled = true,
        .brightness = 0.02f,
        .contrast = 1.08f,
        .saturation = 1.10f,
        .vibrance = 0.25f,
        .gamma = 1.02f,
        .exposure = 0.05f,
        .color_temp = 0.0f,
        .tint = 0.0f,
        .tonemap_mode = TonemapMode::ReinhardJodie,
        .split_intensity = 0.20f,
        .split_balance = 0.0f,
        .shadow_tint = {0.92f, 0.95f, 1.05f},
        .highlight_tint = {1.06f, 1.03f, 0.96f},
        .sharpening = 0.45f,
        .vignette_intensity = 0.18f,
        .film_grain = 0.04f,
        .bleach_bypass = 0.0f,
        .black_level = 0.01f,
        .s_curve = 0.22f,
        .technicolor = 0.0f,
        .dpx_film = 0.45f,
    },

    // [13. Warm Sunlight (Desert)] — Sun-drenched atmosphere with Hable film highlights
    PresetValues{
        .fxaa_enabled = true,
        .brightness = 0.03f,
        .contrast = 1.06f,
        .saturation = 1.12f,
        .vibrance = 0.25f,
        .gamma = 1.02f,
        .exposure = 0.08f,
        .color_temp = 0.18f,
        .tint = 0.04f,
        .tonemap_mode = TonemapMode::Hable,
        .split_intensity = 0.28f,
        .split_balance = 0.06f,
        .shadow_tint = {0.96f, 0.94f, 1.02f},
        .highlight_tint = {1.12f, 1.05f, 0.90f},
        .sharpening = 0.35f,
        .vignette_intensity = 0.16f,
        .film_grain = 0.03f,
        .bleach_bypass = 0.0f,
        .black_level = 0.01f,
        .s_curve = 0.16f,
        .technicolor = 0.10f,
        .dpx_film = 0.0f,
    },

    // [14. Cool Arctic (Tundra)] — Crisp cool blue atmosphere with pure white Khronos Neutral snow
    PresetValues{
        .fxaa_enabled = true,
        .brightness = 0.03f,
        .contrast = 1.08f,
        .saturation = 1.06f,
        .vibrance = 0.22f,
        .gamma = 1.02f,
        .exposure = 0.05f,
        .color_temp = -0.16f,
        .tint = 0.02f,
        .tonemap_mode = TonemapMode::KhronosNeutral,
        .split_intensity = 0.30f,
        .split_balance = -0.04f,
        .shadow_tint = {0.88f, 0.96f, 1.10f},
        .highlight_tint = {0.96f, 1.05f, 1.12f},
        .sharpening = 0.45f,
        .vignette_intensity = 0.16f,
        .film_grain = 0.03f,
        .bleach_bypass = 0.0f,
        .black_level = 0.01f,
        .s_curve = 0.18f,
        .technicolor = 0.0f,
        .dpx_film = 0.0f,
    },
};

struct Settings : PresetValues {
  bool enabled{true};
  Preset preset{Preset::Default};

  void ApplyPreset(Preset p) noexcept {
    preset = p;
    if (p == Preset::Custom) {
      return;
    }
    const auto idx = static_cast<std::size_t>(std::to_underlying(p) - 1);
    if (idx < kPresets.size()) {
      static_cast<PresetValues &>(*this) = kPresets[idx];
    }
  }
};

inline Settings g_settings;

} // namespace patch::post_process
