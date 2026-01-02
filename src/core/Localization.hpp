
#pragma once
#include <string>
// Devuelve el sufijo de idioma para los sprites de botones ("" o "_en")
std::string GetButtonSpriteLangSuffix();

void InitLocalization(const std::string& lang);
void SwitchLocalization();
std::string GetCurrentLanguage();
