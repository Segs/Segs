/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup Components
 * @{
 */

#include "Components/Settings.h"
#include "Common/Utils/IServiceLocator.h"
#include "Components/Logging.h"

#include <ace/Configuration.h>
#include <ace/Configuration_Import_Export.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <filesystem>

String Settings::s_segs_dir;
String Settings::s_settings_path = ("settings.cfg"); // default path 'settings.cfg' from args
String Settings::s_default_tpl_dir = ("default_setup"); // default folder 'default_setup'
String Settings::s_default_settings_path = Settings::s_default_tpl_dir + '/' + "settings_template.cfg"; // default template from folder 'default_setup'


// Template function to convert string to various types
template<typename T>
T fromString(const String& str);

// Specializations for different types
template<>
int fromString<int>(const String& str) { return std::stoi(str.c_str()); }

template<>
float fromString<float>(const String& str) { return std::stof(str.c_str()); }

template<>
double fromString<double>(const String& str) { return std::stod(str.c_str()); }

template<>
bool fromString<bool>(const String& str) { return str == "true" || str == "1"; }

template<>
String fromString<String>(const String& str) { return str; }


class SettingsImpl
{
public:
    ACE_Configuration_Heap config;
    ACE_Configuration_Section_Key current_section;
    Vector<String> group_stack;
    ACE_Recursive_Thread_Mutex mutex;

    SettingsImpl() : config(), current_section(ACE_Configuration_Section_Key())
    {
        config.open();
        current_section = config.root_section();
    }
};


bool fileExists(const String &path)
{
    auto fs = SEGS::getServiceLocator()->getFS();
    SEGS::FileStats stat = fs->stat(path);
    return stat.exists && !stat.is_dir;
}

bool dirExists(const String &path)
{
    auto fs = SEGS::getServiceLocator()->getFS();
    SEGS::FileStats stat = fs->stat(path);
    return stat.exists && stat.is_dir;
}

Settings::Settings(StringView path) : m_impl(new SettingsImpl)
{
    setSettingsPath(String(path));

    if(!fileExists(getSettingsPath()))
        sCritical() << "Settings path not defined? This is unpossible!";
    ACE_Ini_ImpExp config_importer(m_impl->config);
    config_importer.import_config(getSettingsPath().c_str());
}

Settings::~Settings()
{
    delete m_impl;
}

void Settings::beginGroup(StringView group)
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_impl->mutex);
    m_impl->group_stack.push_back(String(group));
    ACE_TString section_name(group.data(), group.length());
    ACE_Configuration_Section_Key new_section;
    if (m_impl->config.open_section(m_impl->current_section, section_name.c_str(), true, new_section) == 0)
    {
        m_impl->current_section = new_section;
    }
}

void Settings::endGroup()
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_impl->mutex);
    if (!m_impl->group_stack.empty())
    {
        m_impl->group_stack.pop_back();
        if (m_impl->group_stack.empty())
        {
            m_impl->current_section = m_impl->config.root_section();
        }
        else
        {
            ACE_Configuration_Section_Key parent_section = m_impl->config.root_section();
            for (const auto& group : m_impl->group_stack)
            {
                ACE_TString section_name(group.c_str());
                ACE_Configuration_Section_Key new_section;
                if (m_impl->config.open_section(parent_section, section_name.c_str(), 0, new_section) == 0)
                {
                    parent_section = new_section;
                }
            }
            m_impl->current_section = parent_section;
        }
    }
}

Vector<String> Settings::childGroups() const
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_impl->mutex);
    Vector<String> groups;
    ACE_TString name;
    for (int i = 0; m_impl->config.enumerate_sections(m_impl->current_section, i, name) == 0; ++i)
    {
        groups.push_back(String(name.c_str()));
    }
    return groups;
}

Vector<String> Settings::allKeys() const
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_impl->mutex);
    Vector<String> keys;
    ACE_Configuration::VALUETYPE type;
    ACE_TString name;
    for (int i = 0; m_impl->config.enumerate_values(m_impl->current_section, i, name, type) == 0; ++i)
    {
        keys.push_back(String(name.c_str()));
    }
    return keys;
}

bool Settings::contains(StringView key)
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_impl->mutex);
    ACE_TString ace_key(key.data(), key.length());
    ACE_Configuration::VALUETYPE type;
    return m_impl->config.find_value(m_impl->current_section, ace_key.c_str(), type) == 0;
}

StringView Settings::value(StringView key)
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_impl->mutex);
    ACE_TString ace_key(key.data(), key.length());
    ACE_TString value;
    if (m_impl->config.get_string_value(m_impl->current_section, ace_key.c_str(), value) == 0)
    {
        return StringView(value.c_str(), value.length());
    }
    return StringView();
}

template<typename T>
T Settings::value(StringView key, const T &default_value, bool *ok)
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_impl->mutex);
    ACE_TString ace_key(key.data(), key.length());
    ACE_TString value;
    ACE_Configuration::VALUETYPE vt;
    int found=m_impl->config.find_value(m_impl->current_section, ace_key.c_str(),vt);
    if (m_impl->config.get_string_value(m_impl->current_section, ace_key.c_str(), value) == 0)
    {
        if (ok) *ok = true;
        return fromString<T>(String(value.c_str()));
    }
    if (ok) *ok = false;
    return default_value;
}

// Explicit instantiations for common types
template int Settings::value<int>(StringView key, const int &default_value, bool *ok);
template float Settings::value<float>(StringView key, const float &default_value, bool *ok);
template double Settings::value<double>(StringView key, const double &default_value, bool *ok);
template bool Settings::value<bool>(StringView key, const bool &default_value, bool *ok);
template String Settings::value<String>(StringView key, const String &default_value, bool *ok);

void Settings::setSettingsPath(const String &path)
{
    if(path.empty())
        sCritical() << "Settings path not defined? This is unpossible!";
    if(!PathUtils::is_rel_path(path))
    {
        s_settings_path = path;
    }
    else
    {
        s_settings_path = getSEGSDir() + '/' + path;
    }

    if(!fileExists(s_settings_path))
        createSettingsFile(s_settings_path);

    sCDebug(logSettings) << "Settings Path" << s_settings_path;
}

String Settings::getSettingsPath()
{
    if(s_settings_path.empty())
        setSettingsPath("settings.cfg"); // set default path to "settings.cfg"

    return s_settings_path;
}

void Settings::discoverSEGSDir()
{
    // Get the current SEGS directory. This library is shared
    // by dbtool and others, so make sure we're in the correct
    // working directory
    std::filesystem::path curdir(std::filesystem::current_path());

    // get absolute path
    auto absolute_curdir = std::filesystem::absolute(curdir);

    sCDebug(logSettings) << "Current Active Dir" << std::filesystem::absolute(curdir).string().c_str();

    String last_part=String(curdir.filename().string().c_str());
    // if called from utilities, move up one directory
    if(last_part.ends_with("utilities", false))
    {
        curdir = curdir.parent_path();
        sCDebug(logSettings) << "Root Dir" << std::filesystem::absolute(curdir).string().c_str();
    }
    // check if we have segs_server executable in the current directory
    SEGS::IServiceLocator *svc = SEGS::getServiceLocator();
    auto *fs = svc->getFS();
    bool has_segs_server = false;

    auto search_path = String(curdir.string().c_str());

    fs->visitEntries(search_path,[&](StringView path, bool is_dir) -> SEGS::IFilesystem::VisitResult {
        if(!is_dir && path.contains("segs_server"))
        {
            has_segs_server=true;
            return SEGS::IFilesystem::VisitResult::VisitStop;
        }
        return SEGS::IFilesystem::VisitResult::VisitNext;
    });

    if(!has_segs_server)
        sWarning() << "Cannot find SEGS Server at" << std::filesystem::absolute(curdir).string().c_str();

    s_segs_dir = std::filesystem::absolute(curdir).string().c_str();
}

String Settings::getSEGSDir()
{
    // if m_segs_dir is not empty, we've set it, return that instead
    if(s_segs_dir.empty())
        discoverSEGSDir();

    return s_segs_dir;
}

String Settings::getSettingsTplPath()
{
    String curdir(getSEGSDir()); // Get the SEGS working directory
    if(!fileExists(curdir + "/" + s_default_settings_path))
        sWarning() << "Cannot find" << s_default_settings_path;

    return curdir + "/" + s_default_settings_path;
}

String Settings::getTemplateDirPath()
{
    String curr_dir(getSEGSDir()); // Get the SEGS working directory
    if(!dirExists(curr_dir + "/" + s_default_tpl_dir))
        sWarning() << "Cannot find directory" << s_default_tpl_dir;

    return curr_dir + "/" + s_default_tpl_dir;
}

void Settings::createSettingsFile(const String &new_file_path)
{
    sCDebug(logSettings) << "Creating Settings file" << new_file_path;
    String tpl_file(Settings::getSettingsTplPath());
    String new_file(new_file_path);

    SEGS::IFilesystem *fs = SEGS::getServiceLocator()->getFS();
    SEGS::IFile *tpl_file_ptr = fs->open(tpl_file, SEGS::IFile::OpenMode::ReadOnly);

    if(!tpl_file_ptr)
    {
        sWarning() << "Unable to read" << tpl_file << "Check folder permissions.";
        return;
    }
    auto content=tpl_file_ptr->readAll();
    delete tpl_file_ptr;
    SEGS::IFile *new_file_ptr = fs->open(new_file, SEGS::IFile::OpenMode::WriteOnly);
    // QSettings setValue() methods delete all file comments, it's better to
    // simply copy the template over to our destination directory.
    // Unfortunately QFile::copy() has some sort of bug and doesn't work
    // so instead let's open the new file, and copy the contents from template
    if(!new_file_ptr || !new_file_ptr->write(content.data(),content.size()))
    {
        sWarning() << "Unable to create" << new_file << "Check folder permissions.";
        return;
    }
    delete new_file_ptr;
}



void settingsDump()
{
    Settings config(Settings::getSettingsPath());
    settingsDump(&config);
}

void settingsDump(Settings *s)
{
    String output = "Settings File Dump\n";
    for(const String &group : s->childGroups())
    {
        String groupString = "===== "+group+" =====\n";
        s->beginGroup(group);

        for(const String &key : s->allKeys())
        {
            auto value = s->value(key);
            groupString.append_sprintf("  %s\t\t %.*s\n",key.c_str(), value.size(),value.data());
        }

        s->endGroup();
        groupString.append("\n");

        output.append(groupString);
    }
    sDebug() << output;
}

//! @}
