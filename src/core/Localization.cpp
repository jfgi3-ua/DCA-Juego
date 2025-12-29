#include "Localization.hpp"
#include "Localization.hpp"
#include <libintl.h>
#include <clocale>
#include <string>

static std::string currentLang = "es";

void InitLocalization(const std::string& lang) {
    setlocale(LC_ALL, "");
    currentLang = lang;
    bindtextdomain("messages", "./locale");
    textdomain("messages");
    bind_textdomain_codeset("messages", "UTF-8");
    setenv("LANGUAGE", lang.c_str(), 1);
}

void SwitchLocalization() {
    if (currentLang == "es") InitLocalization("en");
    else InitLocalization("es");
}

std::string GetCurrentLanguage() {
    return currentLang;
}

std::string GetButtonSpriteLangSuffix() {
    return (currentLang == "en") ? "_en" : "";
}
