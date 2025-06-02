#pragma once
#include <common/path.h>
#include <string>
#include <vector>

typedef std::vector<std::string> Stringlist;

struct UISettings
{
    Stringlist recentFiles;
    bool enableModuleConfiguration;
    Path languageDir;
    std::string languageDirValue;
    std::string languageBase;
    std::string languageCurrent;
    bool sciterConsole;
};

extern UISettings uiSettings;

void LoadUISetting(void);
void SaveUISetting(void);