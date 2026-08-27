module patch.post_process:shader;

import std;

namespace patch::post_process {

inline constexpr std::string_view kShaderSource = R"(
sampler2D s0 : register(s0);

// c0: scale, bias, saturation, gamma
float4 c_ScaleBias : register(c0);
// c1: temp_r, temp_g, temp_b, tonemap_mode (0=none, 1=ACES)
float4 c_ColorGrade : register(c1);
// c2: sharpening, fxaa_enabled, vignette_intensity, film_grain
float4 c_Effects : register(c2);
// c3: inv_width, inv_height, width, height
float4 c_ScreenSize : register(c3);
// c4: shadow_r, shadow_g, shadow_b, split_intensity
float4 c_Shadows : register(c4);
// c5: highlight_r, highlight_g, highlight_b, split_balance
float4 c_Highlights : register(c5);
// c6: vibrance, bleach_bypass, black_level, s_curve
float4 c_Extra : register(c6);
// c7: technicolor, dpx_film, unused, unused
float4 c_Film : register(c7);

// Fast perceptual luma (Rec. 709)
float FxaaLuma(float3 rgb) {
    return dot(rgb, float3(0.2126f, 0.7152f, 0.0722f));
}

// FXAA 3.11 edge-directed antialiasing
float3 ApplyFXAA(sampler2D tex, float2 uv, float2 rcpFrame) {
    float3 rgbM = tex2D(tex, uv).rgb;
    float lumaM = FxaaLuma(rgbM);
    
    float lumaNW = FxaaLuma(tex2D(tex, uv + float2(-1.0f, -1.0f) * rcpFrame).rgb);
    float lumaNE = FxaaLuma(tex2D(tex, uv + float2( 1.0f, -1.0f) * rcpFrame).rgb);
    float lumaSW = FxaaLuma(tex2D(tex, uv + float2(-1.0f,  1.0f) * rcpFrame).rgb);
    float lumaSE = FxaaLuma(tex2D(tex, uv + float2( 1.0f,  1.0f) * rcpFrame).rgb);
    
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
    
    float range = lumaMax - lumaMin;
    if (range < max(0.035f, lumaMax * 0.125f)) {
        return rgbM;
    }
    
    float2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));
    
    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25f * 0.125f), 0.0078125f);
    float rcpDirMin = 1.0f / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = min(float2(8.0f, 8.0f), max(float2(-8.0f, -8.0f), dir * rcpDirMin)) * rcpFrame;
    
    float3 rgbA = 0.5f * (
        tex2D(tex, uv + dir * (1.0f / 3.0f - 0.5f)).rgb +
        tex2D(tex, uv + dir * (2.0f / 3.0f - 0.5f)).rgb);
    float3 rgbB = rgbA * 0.5f + 0.25f * (
        tex2D(tex, uv + dir * (0.0f / 3.0f - 0.5f)).rgb +
        tex2D(tex, uv + dir * (3.0f / 3.0f - 0.5f)).rgb);
    float lumaB = FxaaLuma(rgbB);
    
    if ((lumaB < lumaMin) || (lumaB > lumaMax)) {
        return rgbA;
    }
    return rgbB;
}

// Optimized AMD FidelityFX CAS (Contrast Adaptive Sharpening)
float3 ApplyCAS(sampler2D tex, float2 uv, float2 rcpFrame, float3 center, float sharpness) {
    float3 a = tex2D(tex, uv + float2( 0.0f, -rcpFrame.y)).rgb;
    float3 b = tex2D(tex, uv + float2(-rcpFrame.x,  0.0f)).rgb;
    float3 c = tex2D(tex, uv + float2( rcpFrame.x,  0.0f)).rgb;
    float3 d = tex2D(tex, uv + float2( 0.0f,  rcpFrame.y)).rgb;
    
    float3 minRGB = min(center, min(min(a, b), min(c, d)));
    float3 maxRGB = max(center, max(max(a, b), max(c, d)));
    
    // Adaptive weight: reduces sharpening on high-contrast edges to prevent haloing
    float3 ampRGB = saturate(min(minRGB, 2.0f - maxRGB) / max(maxRGB, 0.001f));
    float3 wRGB = -sqrt(ampRGB) * (sharpness * 0.2f);
    
    // Vectorized cross sum
    float3 crossSum = a + b + c + d;
    float3 color = (crossSum * wRGB + center) / (1.0f + 4.0f * wRGB);
    return saturate(color);
}

// S-Curve Contrast (Smooth midtone contrast curve without highlight/shadow clipping)
float3 ApplySCurve(float3 color, float amount) {
    float3 s = color * color * (3.0f - 2.0f * color);
    return lerp(color, s, amount);
}

// 3-Strip Technicolor process (Classic vintage Hollywood color grading)
float3 ApplyTechnicolor(float3 color, float strength) {
    float3 negative = 1.0f - saturate(color);
    float3 tc = 1.0f - float3(
        negative.g * negative.b,
        negative.r * negative.b,
        negative.r * negative.g
    );
    return lerp(color, saturate(tc), strength);
}

// Cineon DPX Film Tone & Color Density Curve
float3 ApplyDPX(float3 color, float strength) {
    float3 x = saturate(color);
    float3 dpx = (x * (x * (x * 0.4f + 0.6f)));
    dpx = pow(max(0.0f, dpx), 0.85f);
    return lerp(color, saturate(dpx), strength);
}

// ACES Filmic Tone Mapping (Fitted Curve)
float3 TonemapACES(float3 x) {
    return saturate((x * (2.51f * x + 0.03f)) / (x * (2.43f * x + 0.59f) + 0.14f));
}

// Uncharted 2 / Hable Filmic Tone Mapping
float3 HablePartial(float3 x) {
    const float A = 0.15f;
    const float B = 0.50f;
    const float C = 0.10f;
    const float D = 0.20f;
    const float E = 0.02f;
    const float F = 0.30f;
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float3 TonemapHable(float3 x) {
    const float W = 11.2f;
    float3 curr = HablePartial(x * 2.0f);
    float3 whiteScale = 1.0f / HablePartial(float3(W, W, W));
    return saturate(curr * whiteScale);
}

// Reinhard-Jodie Tone Mapping (Luma-based color preservation)
float3 TonemapReinhardJodie(float3 v) {
    float luma = FxaaLuma(v);
    float3 tv = v / (1.0f + v);
    return saturate(lerp(v / (1.0f + luma), tv, tv));
}

// AgX Minimal Fast Realtime Tone Mapping
float3 TonemapAgX(float3 val) {
    const float3x3 agx_mat = float3x3(
        0.842479f, 0.078434f, 0.079224f,
        0.097677f, 0.873837f, 0.028516f,
        0.015748f, 0.054016f, 0.930208f
    );
    const float3x3 agx_mat_inv = float3x3(
        1.196879f, -0.052897f, -0.052972f,
        -0.098021f, 1.151903f, -0.098043f,
        -0.099030f, -0.075440f, 1.151074f
    );
    val = mul(agx_mat, max(0.0f, val));
    val = clamp(log2(max(1e-6f, val)), -10.0f, 6.5f);
    val = (val + 10.0f) / 16.5f;
    float3 val2 = val * val;
    val = 15.5f * val2 * val2 * val - 40.14f * val2 * val2 + 31.96f * val2 * val - 6.868f * val2 + 0.429f * val + 0.119f;
    val = mul(agx_mat_inv, max(0.0f, val));
    return saturate(val);
}

// Khronos PBR Neutral Tone Mapping (2024 standard)
float3 TonemapKhronosNeutral(float3 color) {
    const float startCompression = 0.76f;
    const float desaturation = 0.15f;
    float x = min(color.r, min(color.g, color.b));
    float offset = x < 0.08f ? x - 6.25f * x * x : 0.04f;
    color -= offset;
    float peak = max(color.r, max(color.g, color.b));
    if (peak < startCompression) return saturate(color);
    const float d = 1.0f - startCompression;
    float newPeak = 1.0f - d * d / (peak + d - startCompression);
    color *= newPeak / max(peak, 0.0001f);
    float g = 1.0f - 1.0f / (desaturation * (peak - newPeak) + 1.0f);
    return saturate(lerp(color, newPeak.xxx, g));
}

// Uchimura (Gran Turismo) Tone Mapping
float3 TonemapUchimura(float3 x) {
    const float P = 1.0f, a = 1.0f, m = 0.22f, l = 0.4f, c = 1.33f;
    float l0 = ((P - m) * l) / a;
    float S0 = m + l0;
    float S1 = m + a * l0;
    float C2 = (a * P) / (P - S1);
    float CP = -C2 / P;
    
    float3 w0 = 1.0f - smoothstep(0.0f, m, x);
    float3 w2 = step(S0, x);
    float3 w1 = 1.0f - w0 - w2;
    
    float3 T = m * pow(max(1e-4f, x / m), c);
    float3 S = P - (P - S1) * exp(CP * (x - S0));
    float3 L = m + a * (x - m);
    
    return saturate(T * w0 + L * w1 + S * w2);
}

// Timothy Lottes (AMD) Tone Mapping
float3 TonemapLottes(float3 x) {
    float3 z = pow(max(0.0f, x), 1.6f);
    return saturate(z / (pow(max(0.0f, x), 1.6f * 0.977f) * 0.8824f + 0.1698f));
}

float4 main(float2 texCoord : TEXCOORD0) : COLOR0
{
    float2 rcpFrame = c_ScreenSize.xy;
    float4 color;
    
    // 1. World FXAA (if enabled) or base texture sample
    if (c_Effects.y > 0.5f) {
        color.rgb = ApplyFXAA(s0, texCoord, rcpFrame);
        color.a = tex2D(s0, texCoord).a;
    } else {
        color = tex2D(s0, texCoord);
    }
    
    // 2. Optimized AMD FidelityFX CAS Sharpening
    float sharpening = c_Effects.x;
    if (sharpening > 0.001f) {
        color.rgb = ApplyCAS(s0, texCoord, rcpFrame, color.rgb, sharpening);
    }
    
    // 3. Combined Exposure, Brightness & Contrast (Single MAD instruction folded on CPU)
    color.rgb = color.rgb * c_ScaleBias.x + c_ScaleBias.y;
    
    // 4. Black Level / Shadow Lift
    float blackLevel = c_Extra.z;
    if (abs(blackLevel) > 0.001f) {
        color.rgb = max(0.0f, color.rgb + blackLevel);
    }
    
    // 5. Cinematic S-Curve Contrast
    float sCurve = c_Extra.w;
    if (sCurve > 0.001f) {
        color.rgb = ApplySCurve(saturate(color.rgb), sCurve);
    }
    
    // 6. Saturation & Vibrance
    float luma = FxaaLuma(color.rgb);
    float saturation = c_ScaleBias.z;
    color.rgb = lerp(float3(luma, luma, luma), color.rgb, saturation);
    
    float vibrance = c_Extra.x;
    if (abs(vibrance) > 0.001f) {
        float maxC = max(color.r, max(color.g, color.b));
        float minC = min(color.r, min(color.g, color.b));
        float colorSat = maxC - minC;
        float vibFactor = 1.0f + vibrance * (1.0f - saturate(colorSat));
        color.rgb = lerp(float3(luma, luma, luma), color.rgb, max(0.0f, vibFactor));
    }
    
    // 7. Color Temperature & Tint (Single 3D vector addition pre-folded on CPU)
    color.rgb += c_ColorGrade.xyz;
    
    // 8. Technicolor 3-Strip Process
    float tech = c_Film.x;
    if (tech > 0.001f) {
        color.rgb = ApplyTechnicolor(color.rgb, tech);
    }
    
    // 9. Cineon DPX Film Curve
    float dpx = c_Film.y;
    if (dpx > 0.001f) {
        color.rgb = ApplyDPX(color.rgb, dpx);
    }
    
    // 10. Bleach Bypass (Silver retention / gritty military look)
    float bleach = c_Extra.y;
    if (bleach > 0.001f) {
        float l = FxaaLuma(color.rgb);
        float3 blend = color.rgb < 0.5f ? (2.0f * color.rgb * l) : (1.0f - 2.0f * (1.0f - color.rgb) * (1.0f - l));
        color.rgb = lerp(color.rgb, blend, bleach);
    }
    
    // 11. Split Toning (Shadows & Highlights color separation)
    luma = FxaaLuma(color.rgb);  // Recalculate after all color grading
    float splitInt = c_Shadows.w;
    if (splitInt > 0.001f) {
        float mid = 0.5f + c_Highlights.w * 0.25f;
        float shadowWeight = saturate((mid - luma) / max(mid, 0.01f));
        float highlightWeight = saturate((luma - mid) / max(1.0f - mid, 0.01f));
        
        color.rgb = lerp(color.rgb, color.rgb * c_Shadows.rgb, shadowWeight * splitInt);
        color.rgb = lerp(color.rgb, color.rgb * c_Highlights.rgb, highlightWeight * splitInt);
    }
    
    // 12. Tonemapping Operator Dispatch
    int tonemapMode = (int)(c_ColorGrade.w + 0.5f);
    if (tonemapMode == 1) {
        color.rgb = TonemapACES(max(0.0f, color.rgb));
    } else if (tonemapMode == 2) {
        color.rgb = TonemapHable(max(0.0f, color.rgb));
    } else if (tonemapMode == 3) {
        color.rgb = TonemapReinhardJodie(max(0.0f, color.rgb));
    } else if (tonemapMode == 4) {
        color.rgb = TonemapAgX(max(0.0f, color.rgb));
    } else if (tonemapMode == 5) {
        color.rgb = TonemapKhronosNeutral(max(0.0f, color.rgb));
    } else if (tonemapMode == 6) {
        color.rgb = TonemapUchimura(max(0.0f, color.rgb));
    } else if (tonemapMode == 7) {
        color.rgb = TonemapLottes(max(0.0f, color.rgb));
    }
    
    // 13. Gamma correction
    float gamma = c_ScaleBias.w;
    if (abs(gamma - 1.0f) > 0.001f && gamma > 0.01f) {
        color.rgb = pow(max(0.0f, color.rgb), 1.0f / gamma);
    }
    
    // 14. Cinematic Vignette (Smooth quadratic edge darkening)
    float vignette = c_Effects.z;
    if (vignette > 0.001f) {
        float2 coord = (texCoord - 0.5f) * 2.0f;
        float dist = dot(coord, coord);
        float vigFactor = saturate(1.0f - dist * vignette * 0.5f);
        color.rgb *= vigFactor * vigFactor;
    }
    
    // 15. Textured Film Grain & Anti-Banding Dither
    float grain = c_Effects.w;
    if (grain > 0.001f) {
        float2 screenPos = texCoord * c_ScreenSize.zw;
        float noise = frac(sin(dot(screenPos + c_Film.z, float2(12.9898f, 78.233f))) * 43758.5453f);
        float lumaG = FxaaLuma(color.rgb);
        float grainWeight = 1.0f - lumaG * 0.4f;
        float dither = (noise * 2.0f - 1.0f) * (grain * 0.18f) * grainWeight;
        color.rgb += dither;
    }
    
    return saturate(color);
}
)";

} // namespace patch::post_process
