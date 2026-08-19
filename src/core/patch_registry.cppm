export module core:patch_registry;
export import :patch;
import :settings;
import std;

export class PatchRegistry {
public:
  [[nodiscard]] static PatchRegistry &Instance() noexcept {
    static PatchRegistry instance;
    return instance;
  }

  template <typename T, typename... Args>
    requires std::derived_from<T, IPatch>
  T &RegisterPatch(Args &&...args) {
    auto patch = std::make_unique<T>(std::forward<Args>(args)...);
    T &ref = *patch;
    patches_.push_back(std::move(patch));
    return ref;
  }

  inline void RegisterPatch(std::unique_ptr<IPatch> patch) {
    if (patch) {
      patches_.push_back(std::move(patch));
    }
  }

  [[nodiscard]] std::span<const std::unique_ptr<IPatch>>
  Patches() const noexcept {
    return patches_;
  }

  [[nodiscard]] IPatch &PatchAt(std::size_t index) const noexcept {
    return *patches_[index];
  }

  [[nodiscard]] std::size_t PatchCount() const noexcept {
    return patches_.size();
  }

  [[nodiscard]] bool Empty() const noexcept { return patches_.empty(); }

  void LoadAll(SettingsStore &settings) {
    auto binder = settings.Loader();
    for (auto &patch : patches_) {
      patch->BindSettings(binder);
    }
  }

  void ApplyAll() {
    for (auto &patch : patches_) {
      patch->Apply();
    }
  }

  void SavePatch(IPatch &patch, SettingsStore &settings) {
    auto binder = settings.Saver();
    patch.BindSettings(binder);
  }

private:
  PatchRegistry() = default;
  ~PatchRegistry() = default;
  PatchRegistry(const PatchRegistry &) = delete;
  PatchRegistry &operator=(const PatchRegistry &) = delete;
  PatchRegistry(PatchRegistry &&) = delete;
  PatchRegistry &operator=(PatchRegistry &&) = delete;

  std::vector<std::unique_ptr<IPatch>> patches_;
};
