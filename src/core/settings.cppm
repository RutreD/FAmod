export module core:settings;
import std;
import nlohmann.json;

export inline constexpr std::string_view kSettingsPath = "famod_settings.json";

export class SettingsBinder {
public:
  enum class Mode { Load, Save };

  SettingsBinder(Mode mode, nlohmann::json &data) : mode_(mode), data_(data) {}

  template <typename T>
  void Bind(std::string_view key, T &field, const T &default_value = T{}) {
    if (mode_ == Mode::Load) {
      try {
        field = data_.value(key, default_value);
      } catch (...) {
        field = default_value;
      }
    } else {
      data_[key] = field;
    }
  }

private:
  Mode mode_;
  nlohmann::json &data_;
};

export class SettingsStore {
public:
  void Save(const std::filesystem::path &path) const {
    if (std::ofstream out(path); out.is_open()) {
      out << data_.dump(4);
    }
  }

  [[nodiscard]] bool Load(const std::filesystem::path &path) {
    std::ifstream in(path);
    if (!in.is_open())
      return false;
    try {
      in >> data_;
      return true;
    } catch (...) {
      return false;
    }
  }

  [[nodiscard]] SettingsBinder Loader() {
    return {SettingsBinder::Mode::Load, data_};
  }

  [[nodiscard]] SettingsBinder Saver() {
    return {SettingsBinder::Mode::Save, data_};
  }

private:
  nlohmann::json data_ = nlohmann::json::object();
};

export [[nodiscard]] SettingsStore &GetSettings() noexcept {
  static SettingsStore instance = [] {
    SettingsStore s;
    std::ignore = s.Load(kSettingsPath);
    return s;
  }();
  return instance;
}
