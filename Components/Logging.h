/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2024 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Common/Containers/Vector.h"
#include "Common/Utils/string_utils.h"
#include <Common/Containers/StringView.h>
#include <cassert>


enum class LogLevel : uint8_t {
    Debug,
    Info,
    Warning,
    Critical
};

class DebugOutput
{
public:
    DebugOutput() = default;
    DebugOutput(const char *category) : m_category(category) {}
    DebugOutput(const char *file, int line, const char *func, LogLevel level=LogLevel::Debug,const char *category=nullptr)
        : m_file(file), m_line(line), m_func(func), m_category(category),m_level(level)
    {
    }
    DebugOutput(const DebugOutput &) = delete;
    DebugOutput(DebugOutput &&other) noexcept
        : m_file(other.m_file), m_line(other.m_line), m_func(other.m_func),
          m_category(other.m_category), m_level(other.m_level), m_buffer(std::move(other.m_buffer))
    {
        other.m_file = nullptr;
        other.m_line = 0;
        other.m_func = nullptr;
        other.m_category = nullptr;
    }

    ~DebugOutput();
    DebugOutput &operator=(const DebugOutput &) = delete;
    DebugOutput &operator=(DebugOutput &&other) noexcept
    {
        if (this != &other) {
            m_file = other.m_file;
            m_line = other.m_line;
            m_func = other.m_func;
            m_category = other.m_category;
            m_buffer = std::move(other.m_buffer);
            m_level = other.m_level;

            other.m_file = nullptr;
            other.m_line = 0;
            other.m_func = nullptr;
            other.m_category = nullptr;
        }
        return *this;
    }

    void addToBuffer(StringView str) { m_buffer.append(str); }
    void addToBuffer(int i) { m_buffer.append(eastl::to_string(i)); }

    inline DebugOutput &operator<<(StringView t)
    {
        addToBuffer(t);
        addToBuffer(" ");
        return *this;
    }

    inline DebugOutput &operator<<(char t)
    {
        addToBuffer(StringView(&t,1));
        addToBuffer(" ");
        return *this;
    }

    inline DebugOutput &operator<<(uint32_t t)
    {
        addToBuffer(eastl::to_string(t));
        addToBuffer(" ");
        return *this;
    }

    inline DebugOutput &operator<<(int32_t t)
    {
        addToBuffer(eastl::to_string(t));
        addToBuffer(" ");
        return *this;
    }

    inline DebugOutput &operator<<(int64_t t)
    {
        addToBuffer(eastl::to_string(t));
        addToBuffer(" ");
        return *this;
    }

    inline DebugOutput &operator<<(uint64_t t)
    {
        addToBuffer(eastl::to_string(t));
        addToBuffer(" ");
        return *this;
    }

    inline DebugOutput &operator<<(float t)
    {
        addToBuffer(eastl::to_string(t));
        addToBuffer(" ");
        return *this;
    }

private:
    const char *m_file=nullptr;
    int m_line=0;
    const char *m_func=nullptr;
    const char *m_category=nullptr;
    LogLevel m_level;
    String m_buffer;
};

template<typename T>
void addToDebugOuput(DebugOutput &deb,const T &val);

inline void addToDebugOuput(DebugOutput &deb,StringView val) {
    deb.addToBuffer(val);
}

template<typename T>
inline DebugOutput &operator<<(DebugOutput &debug, const Vector<T> &values)
{
    for(const T& v : values) {
        debug << v;
        debug.addToBuffer(",");
    }
    debug.addToBuffer(" ");
    return debug;
}

struct LogChannelSwitches {
    uint8_t m_debug:1=0;
    uint8_t m_info:1=0;
    uint8_t m_warning:1=0;
    uint8_t m_critical:1=0;
    void setEnabled(LogLevel level, bool v) {
        switch(level) {
        case LogLevel::Debug: m_debug=v; break;
        case LogLevel::Info: m_info=v; break;
        case LogLevel::Warning: m_warning=v; break;
        case LogLevel::Critical: m_critical=v; break;
        }
    }
    [[nodiscard]] bool isDebugEnabled() const {
        return m_debug==1;
    }
    void toggleSwitches() {
        m_debug=!m_debug;
        m_info=!m_info;
        m_warning=!m_warning;
        m_critical=!m_critical;
    }
};

struct LoggingCategory
{
    static Vector<LoggingCategory *> m_registered_categories;

    const char *m_name;
    bool m_enabled=false;
    LogChannelSwitches switches;

    template <typename T>
    DebugOutput &&operator<<(const T &t)
    {
        return DebugOutput(m_name) << t;
    }
    [[nodiscard]] bool enabled() const { return m_enabled; }
    void setEnabled(bool v) { m_enabled=v; }
    void setEnabled(LogLevel level, bool v) {
        switches.setEnabled(level,v);
    }
    [[nodiscard]] const char *name() const { return m_name; }
    [[nodiscard]] bool isDebugEnabled() const {
        return switches.isDebugEnabled() && m_enabled;
    }
    void toggleLogging() {
        switches.toggleSwitches();
    }
    LoggingCategory(const char *name) : m_name(name) {
        auto sp = Span<LoggingCategory *>(m_registered_categories);
        assert(sp.end()==eastl::find_if(sp,[&](const LoggingCategory *lc)->bool { return 0==strcmp(lc->name(),name);}));
        m_registered_categories.push_back(this);
    }
    static void setFilterRules(StringView rules);
};

class LogChannels {
public:
    static DebugOutput debug(const char *file, int line, const char *func, const char *category=nullptr);
    static DebugOutput info(const char *file, int line, const char *func, const char *category=nullptr);
    static DebugOutput warning(const char *file, int line, const char *func, const char *category=nullptr);
    static DebugOutput critical(const char *file, int line, const char *func, const char *category=nullptr);
};

// Declare common logging categories
#define SEGS_DECLARE_LOGGING_CATEGORY(name) \
    extern LoggingCategory &name();

SEGS_DECLARE_LOGGING_CATEGORY(logLogging)
SEGS_DECLARE_LOGGING_CATEGORY(logKeybinds)
SEGS_DECLARE_LOGGING_CATEGORY(logSettings)
SEGS_DECLARE_LOGGING_CATEGORY(logGUI)
SEGS_DECLARE_LOGGING_CATEGORY(logTeams)
SEGS_DECLARE_LOGGING_CATEGORY(logDB)
SEGS_DECLARE_LOGGING_CATEGORY(logInput)
SEGS_DECLARE_LOGGING_CATEGORY(logPosition)
SEGS_DECLARE_LOGGING_CATEGORY(logOrientation)
SEGS_DECLARE_LOGGING_CATEGORY(logMovement)
SEGS_DECLARE_LOGGING_CATEGORY(logChat)
SEGS_DECLARE_LOGGING_CATEGORY(logInfoMsg)
SEGS_DECLARE_LOGGING_CATEGORY(logEmotes)
SEGS_DECLARE_LOGGING_CATEGORY(logTarget)
SEGS_DECLARE_LOGGING_CATEGORY(logCharSel)
SEGS_DECLARE_LOGGING_CATEGORY(logPlayerSpawn)
SEGS_DECLARE_LOGGING_CATEGORY(logNpcSpawn)
SEGS_DECLARE_LOGGING_CATEGORY(logMapEvents)
SEGS_DECLARE_LOGGING_CATEGORY(logMapXfers)
SEGS_DECLARE_LOGGING_CATEGORY(logSlashCommand)
SEGS_DECLARE_LOGGING_CATEGORY(logDescription)
SEGS_DECLARE_LOGGING_CATEGORY(logFriends)
SEGS_DECLARE_LOGGING_CATEGORY(logMiniMap)
SEGS_DECLARE_LOGGING_CATEGORY(logLFG)
SEGS_DECLARE_LOGGING_CATEGORY(logNPCs)
SEGS_DECLARE_LOGGING_CATEGORY(logAnimations)
SEGS_DECLARE_LOGGING_CATEGORY(logPowers)
SEGS_DECLARE_LOGGING_CATEGORY(logTrades)
SEGS_DECLARE_LOGGING_CATEGORY(logTailor)
SEGS_DECLARE_LOGGING_CATEGORY(logScripts)
SEGS_DECLARE_LOGGING_CATEGORY(logSceneGraph)
SEGS_DECLARE_LOGGING_CATEGORY(logStores)
SEGS_DECLARE_LOGGING_CATEGORY(logTasks)
SEGS_DECLARE_LOGGING_CATEGORY(logRPC)
SEGS_DECLARE_LOGGING_CATEGORY(logAFK)
SEGS_DECLARE_LOGGING_CATEGORY(logConnection)
SEGS_DECLARE_LOGGING_CATEGORY(logMigration)

void    setLoggingFilter();
void    toggleLogging(StringView category);
void    dumpLogging();

#define TIMED_LOG(x,msg) {\
    auto log(sDebug());\
    log << (msg) << "..."; \
    SEGS::ElapsedTimer timer;\
    timer.start();\
    x;\
    log << "done in"<<float(timer.elapsed())/1000.0f<<"s";\
}

#define sCDebug(category) \
    if(category().enabled())\
        LogChannels::debug(__FILE__,__LINE__,__FUNCTION__, category().name())

#define sCFDebug(category,fmt,...) \
    if(category().enabled()) \
        LogChannels::debug(__FILE__,__LINE__,__FUNCTION__, category().name()) << String(String::CtorSprintf(),(fmt), __VA_ARGS__)

#define sCInfo(category) \
    if (category().enabled()) \
        LogChannels::info(__FILE__,__LINE__,__FUNCTION__, category().name())

#define sCWarning(category) \
    if(category().enabled())\
        LogChannels::warning(__FILE__,__LINE__,__FUNCTION__, category().name())

#define sCFWarning(category, fmt, ...) \
    if (category().enabled()) \
        LogChannels::warning(__FILE__,__LINE__,__FUNCTION__, category().name()) << String(String::CtorSprintf(), (fmt), __VA_ARGS__)

#define sCCritical(category) \
    if(category().enabled()) \
        LogChannels::critical(__FILE__,__LINE__,__FUNCTION__, category().name())

#define sCritical() LogChannels::critical(__FILE__,__LINE__,__FUNCTION__)
#define sWarning() LogChannels::warning(__FILE__,__LINE__,__FUNCTION__)
#define sDebug() LogChannels::debug(__FILE__,__LINE__,__FUNCTION__)
#define sInfo() LogChannels::info(__FILE__,__LINE__,__FUNCTION__)

extern void toggleLogging(StringView category);
