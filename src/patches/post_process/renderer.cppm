module;
#include <Windows.h>
#include <d3d9.h>
#include <d3dcompiler.h>

module patch.post_process:renderer;

import std;
import imgui_hook;
import :utils;
import :settings;
import :shader;

namespace patch::post_process {

class PostProcessRenderer {
private:
  ComPtr<IDirect3DTexture9> intermediate_texture_;
  ComPtr<IDirect3DSurface9> intermediate_surface_;
  ComPtr<IDirect3DVertexBuffer9> vertex_buffer_;
  ComPtr<IDirect3DStateBlock9> state_block_;
  ComPtr<IDirect3DPixelShader9> pixel_shader_;
  UINT texture_width_{0};
  UINT texture_height_{0};
  bool shader_compilation_failed_{false};
  UINT frame_count_{0};

  struct ScreenVertex {
    float x{0.0f}, y{0.0f}, z{0.0f}, rhw{1.0f};
    float u{0.0f}, v{0.0f};
  };

  struct alignas(16) ShaderConstants {
    std::array<float, 4> scale_bias;      // c0: scale, bias, saturation, gamma
    std::array<float, 4> color_temp_tint; // c1: temp_r, temp_g, temp_b, tonemap_mode
    std::array<float, 4> effects;         // c2: sharpening, fxaa, vignette, grain
    std::array<float, 4> screen_size;     // c3: 1/w, 1/h, w, h
    std::array<float, 4> shadows;         // c4: shadow_r, shadow_g, shadow_b, split_intensity
    std::array<float, 4> highlights;      // c5: highlight_r, highlight_g, highlight_b, split_balance
    std::array<float, 4> extra;           // c6: vibrance, bleach_bypass, black_level, s_curve
    std::array<float, 4> film;            // c7: technicolor, dpx_film, frame_seed, 0

    [[nodiscard]] static ShaderConstants
    FromSettings(const Settings &s, float width, float height,
                 float frame_seed) noexcept {
      const float scale = std::exp2(s.exposure) * s.contrast;
      const float bias = (s.brightness - 0.5f) * s.contrast + 0.5f;

      const float temp_r = s.color_temp * 0.1f + s.tint * 0.05f;
      const float temp_g = -s.tint * 0.1f;
      const float temp_b = -s.color_temp * 0.1f + s.tint * 0.05f;

      return ShaderConstants{
          .scale_bias = {scale, bias, s.saturation, s.gamma},
          .color_temp_tint =
              {temp_r, temp_g, temp_b,
               static_cast<float>(std::to_underlying(s.tonemap_mode))},
          .effects = {s.sharpening, s.fxaa_enabled ? 1.0f : 0.0f,
                      s.vignette_intensity, s.film_grain},
          .screen_size = {1.0f / width, 1.0f / height, width, height},
          .shadows = {s.shadow_tint[0], s.shadow_tint[1], s.shadow_tint[2],
                      s.split_intensity},
          .highlights = {s.highlight_tint[0], s.highlight_tint[1],
                         s.highlight_tint[2], s.split_balance},
          .extra = {s.vibrance, s.bleach_bypass, s.black_level, s.s_curve},
          .film = {s.technicolor, s.dpx_film, frame_seed, 0.0f},
      };
    }

    [[nodiscard]] const float *data() const noexcept {
      return scale_bias.data();
    }

    [[nodiscard]] static constexpr UINT register_count() noexcept {
      return sizeof(ShaderConstants) / sizeof(float[4]);
    }
  };

  static constexpr DWORD kFVF = D3DFVF_XYZRHW | D3DFVF_TEX1;

  [[nodiscard]] bool CompileShader(IDirect3DDevice9 *device) {
    if (pixel_shader_)
      return true;
    if (shader_compilation_failed_)
      return false;

    auto d3d_compile = D3DCompiler::GetCompileFunction();
    if (!d3d_compile) {
      shader_compilation_failed_ = true;
      return false;
    }

    ComPtr<ID3DBlob> code_blob;
    ComPtr<ID3DBlob> error_blob;
    const HRESULT hr = d3d_compile(
        kShaderSource.data(), kShaderSource.size(), nullptr, nullptr, nullptr,
        "main", "ps_3_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0,
        code_blob.ReleaseAndGetAddressOf(), error_blob.ReleaseAndGetAddressOf());

    if (FAILED(hr) || !code_blob) {
      shader_compilation_failed_ = true;
      return false;
    }

    const HRESULT ps_hr = device->CreatePixelShader(
        reinterpret_cast<const DWORD *>(code_blob->GetBufferPointer()),
        pixel_shader_.ReleaseAndGetAddressOf());

    return SUCCEEDED(ps_hr) && pixel_shader_;
  }

  [[nodiscard]] bool EnsureResources(IDirect3DDevice9 *device, UINT width,
                                     UINT height, D3DFORMAT format) {
    if (intermediate_texture_ && intermediate_surface_ && vertex_buffer_ &&
        state_block_ && texture_width_ == width && texture_height_ == height) {
      return true;
    }

    ReleaseResources();

    // 1. Create render target texture
    if (FAILED(device->CreateTexture(
            width, height, 1, D3DUSAGE_RENDERTARGET, format, D3DPOOL_DEFAULT,
            intermediate_texture_.ReleaseAndGetAddressOf(), nullptr)) ||
        !intermediate_texture_) {
      return false;
    }

    if (FAILED(intermediate_texture_->GetSurfaceLevel(
            0, intermediate_surface_.ReleaseAndGetAddressOf())) ||
        !intermediate_surface_) {
      ReleaseResources();
      return false;
    }

    // 2. Create static GPU vertex buffer for fullscreen triangle
    if (FAILED(device->CreateVertexBuffer(
            sizeof(ScreenVertex) * 3, D3DUSAGE_WRITEONLY, kFVF, D3DPOOL_DEFAULT,
            vertex_buffer_.ReleaseAndGetAddressOf(), nullptr)) ||
        !vertex_buffer_) {
      ReleaseResources();
      return false;
    }

    const float w = static_cast<float>(width);
    const float h = static_cast<float>(height);
    const ScreenVertex triangle[3] = {
        {-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f},
        {2.0f * w - 0.5f, -0.5f, 0.0f, 1.0f, 2.0f, 0.0f},
        {-0.5f, 2.0f * h - 0.5f, 0.0f, 1.0f, 0.0f, 2.0f},
    };

    if (ScopedVertexBufferLock lock(vertex_buffer_.Get(), 0, sizeof(triangle),
                                    0);
        lock) {
      std::memcpy(lock.data(), triangle, sizeof(triangle));
    } else {
      ReleaseResources();
      return false;
    }

    // 3. Create persistent D3D9 StateBlock
    if (FAILED(device->CreateStateBlock(
            D3DSBT_ALL, state_block_.ReleaseAndGetAddressOf())) ||
        !state_block_) {
      ReleaseResources();
      return false;
    }

    texture_width_ = width;
    texture_height_ = height;
    return true;
  }

  void SetupRenderStates(IDirect3DDevice9 *device,
                         IDirect3DSurface9 *backbuffer, UINT width,
                         UINT height) noexcept {
    device->SetRenderTarget(0, backbuffer);
    device->SetTexture(0, intermediate_texture_.Get());

    // Explicitly set viewport to cover the entire render target surface
    const D3DVIEWPORT9 vp{
        .X = 0,
        .Y = 0,
        .Width = width,
        .Height = height,
        .MinZ = 0.0f,
        .MaxZ = 1.0f,
    };
    device->SetViewport(&vp);

    // Disable programmable vertex shader so FVF (XYZRHW) fixed-function path is used
    device->SetVertexShader(nullptr);

    device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
    device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    device->SetSamplerState(0, D3DSAMP_SRGBTEXTURE, FALSE);

    device->SetRenderState(D3DRS_ZENABLE, FALSE);
    device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    device->SetRenderState(D3DRS_STENCILENABLE, FALSE);
    device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
    device->SetRenderState(D3DRS_CLIPPLANEENABLE, 0);
    device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    device->SetRenderState(D3DRS_LIGHTING, FALSE);
    device->SetRenderState(D3DRS_FOGENABLE, FALSE);
    device->SetRenderState(D3DRS_COLORWRITEENABLE,
                           D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN |
                               D3DCOLORWRITEENABLE_BLUE |
                               D3DCOLORWRITEENABLE_ALPHA);
    device->SetRenderState(D3DRS_SRGBWRITEENABLE, FALSE);

  }

public:
  [[nodiscard]] static PostProcessRenderer &Instance() noexcept {
    static PostProcessRenderer instance;
    return instance;
  }

  void ReleaseResources() noexcept {
    state_block_.Reset();
    vertex_buffer_.Reset();
    intermediate_surface_.Reset();
    intermediate_texture_.Reset();
    texture_width_ = 0;
    texture_height_ = 0;
  }

  void OnReset() noexcept {
    ReleaseResources();
    pixel_shader_.Reset();
    shader_compilation_failed_ = false;
  }

  void Render(IDirect3DDevice9 *device) {
    if (!device || !g_settings.enabled)
      return;

    if (device->TestCooperativeLevel() != D3D_OK)
      return;

    ComPtr<IDirect3DSurface9> backbuffer;
    if (FAILED(
            device->GetRenderTarget(0, backbuffer.ReleaseAndGetAddressOf())) ||
        !backbuffer) {
      return;
    }

    D3DSURFACE_DESC desc{};
    if (FAILED(backbuffer->GetDesc(&desc))) {
      return;
    }

    if (!EnsureResources(device, desc.Width, desc.Height, desc.Format) ||
        !CompileShader(device)) {
      return;
    }

    // 1. Capture entire D3D9 pipeline state BEFORE any modifications
    state_block_->Capture();

    // 3. Copy 3D world render output into the intermediate texture (fast GPU blit)
    if (FAILED(device->StretchRect(backbuffer.Get(), nullptr,
                                  intermediate_surface_.Get(), nullptr,
                                  D3DTEXF_NONE))) {
      state_block_->Apply();
      return;
    }

    // 4. Setup render states for post-process fullscreen blit
    SetupRenderStates(device, backbuffer.Get(), desc.Width, desc.Height);

    // 5. Precompute linear math on CPU & pack into constant buffer
    const auto constants = ShaderConstants::FromSettings(
        g_settings, static_cast<float>(desc.Width),
        static_cast<float>(desc.Height),
        static_cast<float>(frame_count_++));

    device->SetPixelShader(pixel_shader_.Get());
    device->SetPixelShaderConstantF(0, constants.data(),
                                    constants.register_count());

    // 6. Draw single fullscreen triangle from GPU VRAM
    device->SetStreamSource(0, vertex_buffer_.Get(), 0, sizeof(ScreenVertex));
    device->SetFVF(kFVF);
    device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);

    // 7. Restore original pipeline state
    state_block_->Apply();
  }
};

} // namespace patch::post_process
