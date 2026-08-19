export module core:i18n;
import std;

export enum class Language : int {
  English = 0,
  Russian = 1,
  Chinese = 2,
};

export struct LanguageInfo {
  Language id;
  std::string_view code;
  std::string_view display_name;
};

export inline constexpr std::array<LanguageInfo, 3> kSupportedLanguages = {{
    {Language::English, "en", "English"},
    {Language::Russian, "ru", "Русский"},
    {Language::Chinese, "zh", "简体中文"},
}};

namespace {
Language g_current_language = Language::English;
}

export namespace core::i18n {

[[nodiscard]] Language GetLanguage() noexcept { return g_current_language; }

void SetLanguage(Language lang) noexcept { g_current_language = lang; }

[[nodiscard]] inline std::string_view LanguageToCode(Language lang) noexcept {
  for (const auto &info : kSupportedLanguages) {
    if (info.id == lang)
      return info.code;
  }
  return "en";
}

[[nodiscard]] inline Language LanguageFromCode(std::string_view code) noexcept {
  for (const auto &info : kSupportedLanguages) {
    if (info.code == code)
      return info.id;
  }
  return Language::English;
}

} // namespace core::i18n

export struct LocalizedString {
  std::string_view view;

  [[nodiscard]] constexpr const char *data() const noexcept {
    return view.data();
  }
  [[nodiscard]] constexpr const char *c_str() const noexcept {
    return view.data();
  }
  [[nodiscard]] constexpr std::size_t size() const noexcept {
    return view.size();
  }
  [[nodiscard]] constexpr bool empty() const noexcept { return view.empty(); }

  // Implicit cast to const char* for ImGui widgets and C-strings
  constexpr operator const char *() const noexcept { return view.data(); }

  // Implicit cast to std::string_view for std APIs, Name(), Description()
  constexpr operator std::string_view() const noexcept { return view; }
};

export [[nodiscard]] inline LocalizedString
tr(std::string_view en,
   std::initializer_list<std::pair<Language, std::string_view>> translations =
       {}) noexcept {
  const Language current = core::i18n::GetLanguage();
  if (current == Language::English) {
    return LocalizedString{en};
  }

  for (const auto &[lang, text] : translations) {
    if (lang == current && !text.empty()) {
      return LocalizedString{text};
    }
  }

  return LocalizedString{en}; // Fallback to English
}
