#include "settings.h"
#include <common/file.h>
#include <common/json.h>
#include <common/path.h>

std::unique_ptr<SettingsStore> SettingsStore::s_instance;

SettingsStore::SettingsStore()
{
    Path congFilePath(Path::MODULE_DIRECTORY, "NxEmu.config");
    congFilePath.AppendDirectory("config");
    m_configPath = (const char *)congFilePath;
}

bool SettingsStore::Initialize()
{
    m_details = JsonValue();
    for (size_t i = 0; i < 100; i++)
    {
        File configFile;
        if (!configFile.Open(m_configPath.c_str(), IFile::modeRead))
        {
            break;
        }
        uint32_t fileLen = (uint32_t)configFile.GetLength();
        if (fileLen <= 0)
        {
            break;
        }
        std::unique_ptr<char[]> data = std::make_unique<char[]>(fileLen);
        uint32_t Size = configFile.Read(data.get(), fileLen);
        if (Size != fileLen)
        {
            break;
        }

        JsonReader reader;
        JsonValue root;
        if (!reader.Parse(data.get(), data.get() + Size, root))
        {
            return false;
        }
        const JsonValue * value = root.Find("ConfigFile");
        if (value == nullptr)
        {
            m_details = std::move(root);
            break;
        }
        else
        {
            Path newConfigfile(value != nullptr ? value->asString() : "");
            newConfigfile.DirectoryNormalize(Path(Path::MODULE_DIRECTORY));
            m_configPath = (const char *)newConfigfile;
        }
    }
    return true;
}

const char * SettingsStore::GetConfigFile(void) const
{
    return m_configPath.c_str();
}

void SettingsStore::SetChanged(const char * setting, bool changed)
{
    if (setting == nullptr)
    {
        return;
    }
    m_settingsChanged[setting] = changed;
}

JsonValue SettingsStore::GetSettings(const char * section) const
{
    const JsonValue * value = m_details.Find(section);
    if (value != nullptr)
    {
        return *value;
    }
    return JsonValue();
}

void SettingsStore::SetSettings(const char * section, JsonValue & json)
{
    if (json.isNull())
    {
        m_details.removeMember(section);
    }
    else
    {
        m_details[section] = json;
    }
}

const char * SettingsStore::GetDefaultString(const char * setting) const
{
    SettingsMapString::const_iterator itr = m_settingsDefaultString.find(setting);
    if (itr == m_settingsDefaultString.end())
    {
        return "";
    }
    return itr->second.c_str();
}

bool SettingsStore::GetDefaultBool(const char * setting) const
{
    SettingsMapBool::const_iterator itr = m_settingsDefaultBool.find(setting);
    if (itr == m_settingsDefaultBool.end())
    {
        return false;
    }
    return itr->second;
}

int32_t SettingsStore::GetDefaultInt(const char* setting) const
{
    SettingsMapInt::const_iterator itr = m_settingsDefaultInt.find(setting);
    if (itr == m_settingsDefaultInt.end())
    {
        return false;
    }
    return itr->second;
}

const char * SettingsStore::GetString(const char * setting) const
{
    SettingsMapString::const_iterator itr = m_settingsString.find(setting);
    if (itr == m_settingsString.end())
    {
        return GetDefaultString(setting);
    }
    return itr->second.c_str();
}

bool SettingsStore::GetBool(const char * setting) const
{
    SettingsMapBool::const_iterator itr = m_settingsBool.find(setting);
    if (itr == m_settingsBool.end())
    {
        return GetDefaultBool(setting);
    }
    return itr->second;
}

bool SettingsStore::GetChanged(const char * setting) const
{
    SettingsMapBool::const_iterator itr = m_settingsChanged.find(setting);
    if (itr == m_settingsChanged.end())
    {
        return false;
    }
    return itr->second;
}

int32_t SettingsStore::GetInt(const char* setting) const
{
    SettingsMapInt::const_iterator itr = m_settingsInt.find(setting);
    if (itr == m_settingsInt.end())
    {
        return GetDefaultInt(setting);
    }
    return itr->second;
}

void SettingsStore::SetDefaultString(const char * setting, const char * value)
{
    if (setting == nullptr || value == nullptr)
    {
        return;
    }
    m_settingsDefaultString[setting] = value;
}

void SettingsStore::SetDefaultBool(const char * setting, bool value)
{
    m_settingsDefaultBool[setting] = value;
}

void SettingsStore::SetDefaultInt(const char * setting, int32_t value)
{
    m_settingsDefaultInt[setting] = value;
}

void SettingsStore::SetString(const char * setting, const char * value)
{
    if (setting == nullptr || value == nullptr)
    {
        return;
    }

    SettingsMapString::const_iterator it = m_settingsString.find(setting);
    if (it != m_settingsString.end() && it->second.compare(value) == 0)
    {
        return;
    }
    m_settingsString[setting] = value;
    NotifyChange(setting);
}

void SettingsStore::SetBool(const char * setting, bool value)
{
    if (setting == nullptr)
    {
        return;
    }

    SettingsMapBool::const_iterator it = m_settingsBool.find(setting);
    if (it != m_settingsBool.end() && it->second == value)
    {
        return;
    }
    m_settingsBool[setting] = value;
    NotifyChange(setting);
}

void SettingsStore::SetInt(const char * setting, int32_t value)
{
    if (setting == nullptr)
    {
        return;
    }

    SettingsMapInt::const_iterator it = m_settingsInt.find(setting);
    if (it != m_settingsInt.end() && it->second == value)
    {
        return;
    }
    m_settingsInt[setting] = value;
    NotifyChange(setting);
}

void SettingsStore::Save(void)
{
    std::string jsonStr = JsonStyledWriter().write(m_details);
    Path(m_configPath).DirectoryCreate();
    File configFile;
    if (!configFile.Open(m_configPath.c_str(), IFile::modeWrite | IFile::modeCreate))
    {
        return;
    }
    configFile.Write(jsonStr.c_str(), (uint32_t)jsonStr.length());
}

void SettingsStore::RegisterCallback(const std::string & setting, SettingChangeCallback callback, void * userData)
{
    if (callback == nullptr)
    {
        return;
    }

    CallbackInfo info = { callback, userData };
    m_notification[setting].emplace_back(info);
}

SettingsStore & SettingsStore::GetInstance()
{
    if (s_instance == nullptr)
    {
        s_instance = std::make_unique<SettingsStore>();
    }
    return *s_instance;
}

void SettingsStore::CleanUp()
{
    s_instance.reset();
}

void SettingsStore::NotifyChange(const char * setting)
{
    if (setting == nullptr)
    {
        return;
    }
    NotificationMap::const_iterator itr = m_notification.find(setting);
    if (itr != m_notification.end())
    {
        const NotificationCallbacks & callbacks = itr->second;
        for (NotificationCallbacks::const_iterator callItr = callbacks.begin(); callItr != callbacks.end(); callItr++)
        {
            callItr->callback(setting, callItr->userData);
        }
    }
}