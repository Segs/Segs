#include "ScriptingEngine.h"

#ifdef SCRIPTING_ENABLED
#include <lua/lua.hpp>
#include <sol2/sol.hpp>
#endif


int luaopen_package(lua_State *L)
{
    assert(!"functionality removed");
}
int luaopen_io(lua_State *L)
{
    assert(!"functionality removed");
}
int luaopen_os(lua_State *L)
{
    assert(!"functionality removed");
}

struct ScriptingEngine::ScriptingEnginePrivate
{
    ScriptingEnginePrivate()
    {
        m_lua.open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::table, sol::lib::string, sol::lib::math,
                             sol::lib::utf8, sol::lib::debug);

    }
    sol::state m_lua;
};

ScriptingEngine::ScriptingEngine() : m_private(new ScriptingEnginePrivate)
{

}

ScriptingEngine::~ScriptingEngine()
{

}
