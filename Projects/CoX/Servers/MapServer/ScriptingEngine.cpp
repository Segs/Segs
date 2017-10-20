#include "ScriptingEngine.h"
#include "MapClient.h"

#include "Events/ChatMessage.h"
#include "Common/NetStructures/Entity.h"
#include "Common/NetStructures/Contact.h"

#include <lua/lua.hpp>
#include <sol2/sol.hpp>


int luaopen_package(lua_State *)
{
    assert(false && "functionality removed");
}
int luaopen_io(lua_State *)
{
    assert(false && "functionality removed");
}
int luaopen_os(lua_State *)
{
    assert(false && "functionality removed");
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

void ScriptingEngine::registerTypes()
{
    m_private->m_lua.new_usertype<Contact>( "Contact",
        // 3 constructors
        sol::constructors<Contact()>(),
        // gets or set the value using member variable syntax
        "name", sol::property(&Contact::getName, &Contact::setName),
        "display_name", &Contact::m_display_name
    );
    m_private->m_lua.new_usertype<MapClient>( "MapClient",
        "new", sol::no_constructor, // The client links are not constructible from the script side.
        "admin_chat_message", sendAdminMessage
    );

}
int ScriptingEngine::runScript(MapClient * client, const QString &script_contents, const char *script_name)
{
    m_private->m_lua["client"] = client;
    sol::load_result load_res=m_private->m_lua.load(script_contents.toStdString(),script_name);
    if(!load_res.valid())
    {
        sol::error err = load_res;
        sendAdminMessage(client,err.what());
        return -1;
    }
    sol::protected_function_result script_result = load_res();
    if(!script_result.valid())
    {
        sol::error err = script_result;
        sendAdminMessage(client,err.what());
        return -1;
    }
    return 0;
}
int ScriptingEngine::runScript(const QString &script_contents, const char *script_name)
{
    sol::load_result load_res = m_private->m_lua.load(script_contents.toStdString(), script_name);
    if (!load_res.valid())
    {
        sol::error err = load_res;
        // TODO: report error here.
        return -1;
    }
    sol::protected_function_result script_result = load_res();
    if (!script_result.valid())
    {
        sol::error err = script_result;
        // TODO: report error here.
        return -1;
    }
    return 0;
}
