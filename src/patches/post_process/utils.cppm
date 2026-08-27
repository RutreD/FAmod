module;
#include <Windows.h>
#include <d3d9.h>
#include <d3dcompiler.h>

module patch.post_process:utils;

import std;

namespace patch::post_process {

template <typename T>
class ComPtr {
public:
  constexpr ComPtr() noexcept = default;
  constexpr ComPtr(std::nullptr_t) noexcept : ptr_(nullptr) {}

  explicit ComPtr(T *ptr) noexcept : ptr_(ptr) {}

  ComPtr(const ComPtr &other) noexcept : ptr_(other.ptr_) {
    if (ptr_) {
      ptr_->AddRef();
    }
  }

  ComPtr(ComPtr &&other) noexcept : ptr_(other.ptr_) {
    other.ptr_ = nullptr;
  }

  ~ComPtr() {
    Reset();
  }

  ComPtr &operator=(std::nullptr_t) noexcept {
    Reset();
    return *this;
  }

  ComPtr &operator=(const ComPtr &other) noexcept {
    if (this != &other) {
      if (other.ptr_) {
        other.ptr_->AddRef();
      }
      Reset();
      ptr_ = other.ptr_;
    }
    return *this;
  }

  ComPtr &operator=(ComPtr &&other) noexcept {
    if (this != &other) {
      Reset();
      ptr_ = other.ptr_;
      other.ptr_ = nullptr;
    }
    return *this;
  }

  void Reset() noexcept {
    if (ptr_) {
      T *temp = ptr_;
      ptr_ = nullptr;
      temp->Release();
    }
  }

  void Attach(T *ptr) noexcept {
    Reset();
    ptr_ = ptr;
  }

  [[nodiscard]] T *Detach() noexcept {
    T *temp = ptr_;
    ptr_ = nullptr;
    return temp;
  }

  [[nodiscard]] T *Get() const noexcept { return ptr_; }
  [[nodiscard]] T **GetAddressOf() noexcept { return &ptr_; }
  [[nodiscard]] T **ReleaseAndGetAddressOf() noexcept {
    Reset();
    return &ptr_;
  }

  [[nodiscard]] T *operator->() const noexcept { return ptr_; }
  [[nodiscard]] T &operator*() const noexcept { return *ptr_; }
  [[nodiscard]] explicit operator bool() const noexcept { return ptr_ != nullptr; }

  [[nodiscard]] auto operator<=>(const ComPtr &) const = default;

private:
  T *ptr_{nullptr};
};

class ScopedVertexBufferLock {
public:
  ScopedVertexBufferLock(IDirect3DVertexBuffer9 *vb, UINT offset, UINT size,
                         DWORD flags) noexcept
      : vb_(vb) {
    if (vb_) {
      if (FAILED(vb_->Lock(offset, size, &data_, flags))) {
        data_ = nullptr;
      }
    }
  }

  ~ScopedVertexBufferLock() {
    if (data_ && vb_) {
      vb_->Unlock();
    }
  }

  ScopedVertexBufferLock(const ScopedVertexBufferLock &) = delete;
  ScopedVertexBufferLock &operator=(const ScopedVertexBufferLock &) = delete;
  ScopedVertexBufferLock(ScopedVertexBufferLock &&) = delete;
  ScopedVertexBufferLock &operator=(ScopedVertexBufferLock &&) = delete;

  [[nodiscard]] void *data() const noexcept { return data_; }
  [[nodiscard]] explicit operator bool() const noexcept {
    return data_ != nullptr;
  }

private:
  IDirect3DVertexBuffer9 *vb_{nullptr};
  void *data_{nullptr};
};

struct D3DCompiler {
  using CompileFn = HRESULT(WINAPI *)(
      LPCVOID, SIZE_T, LPCSTR, const D3D_SHADER_MACRO *, ID3DInclude *,
      LPCSTR, LPCSTR, UINT, UINT, ID3DBlob **, ID3DBlob **);

  [[nodiscard]] static CompileFn GetCompileFunction() noexcept {
    static const CompileFn fn = []() -> CompileFn {
      constexpr std::array<const char *, 3> kDllNames = {
          "d3dcompiler_47.dll", "d3dcompiler_43.dll", "d3dcompiler.dll"};
      for (const char *dll_name : kDllNames) {
        if (HMODULE mod = LoadLibraryA(dll_name)) {
          if (auto proc = reinterpret_cast<CompileFn>(
                  GetProcAddress(mod, "D3DCompile"))) {
            return proc;
          }
          FreeLibrary(mod);
        }
      }
      return nullptr;
    }();
    return fn;
  }
};

} // namespace patch::post_process
