#include "Localization.hpp"
#include <libintl.h>
#include <clocale>
#include <string>
#include <cstdlib>
#include <filesystem>

static std::string currentLang = "es";

void InitLocalization(const std::string& lang) {
    setlocale(LC_ALL, "");
    currentLang = lang;
    
    // Buscar locale primero en directorio local (desarrollo), luego en instalación del sistema
    const char* localeDirs[] = {
        "./locale",
        "/usr/share/game/locale",
        "/usr/local/share/game/locale"
    };
    
    const char* localeDir = nullptr;
    for (const char* dir : localeDirs) {
        if (std::filesystem::exists(dir)) {
            localeDir = dir;
            break;
        }
    }
    
    if (!localeDir) {
        localeDir = "./locale"; // fallback
    }
    
    bindtextdomain("messages", localeDir);
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
