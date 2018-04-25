#include "ScriptingEngine.h"
#include "MapClientSession.h"
#include "MapSceneGraph.h"

#include "Events/ChatMessage.h"
#include "Events/StandardDialogCmd.h"
#include "Common/NetStructures/Entity.h"
#include "Common/NetStructures/Contact.h"
#define SOL_CHECK_ARGUMENTS
#include <lua/lua.hpp>
#include <sol2/sol.hpp>

#include <QtCore/QDebug>

int luaopen_package(lua_State *)
{
    assert(false && "functionality removed");
    return 0;
}
int luaopen_io(lua_State *)
{
    assert(false && "functionality removed");
    return 0;
}
int luaopen_os(lua_State *)
{
    assert(false && "functionality removed");
    return 0;
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
    m_private->m_lua.new_usertype<glm::vec3>( "vec3",
        sol::constructors<glm::vec3(), glm::vec3(float,float,float)>(),
        "x", &glm::vec3::x,
        "y", &glm::vec3::y,
        "z", &glm::vec3::z
    );
    m_private->m_lua.new_usertype<Contact>( "Contact",
        // 3 constructors
        sol::constructors<Contact()>(),
        // gets or set the value using member variable syntax
        "name", sol::property(&Contact::getName, &Contact::setName),
        "display_name", &Contact::m_display_name
    );
    m_private->m_lua.new_usertype<MapClientSession>( "MapClientSession",
        "new", sol::no_constructor, // The client links are not constructible from the script side.
        "admin_chat_message", sendChatMessage,
        "simple_dialog", [](MapClientSession *cl,const char *dlgtext) {
            auto n = new StandardDialogCmd(dlgtext);
            cl->addCommandToSendNextUpdate(std::unique_ptr<StandardDialogCmd>(n));
        }
    );
    m_private->m_lua.new_usertype<MapSceneGraph>( "MapSceneGraph",
        "new", sol::no_constructor, // The client links are not constructible from the script side.
        "set_default_spawn_point", &MapSceneGraph::set_default_spawn_point
    );
    m_private->m_lua.script("function ErrorHandler(msg) return \"Lua call error:\"..msg end");

}
int ScriptingEngine::loadAndRunFile(const QString &filename)
{
    sol::load_result load_res=m_private->m_lua.load_file(filename.toStdString());
    if(!load_res.valid())
    {
        sol::error err = load_res;
        qCritical() << err.what();
        return -1;
    }
    sol::protected_function_result script_result = load_res();
    if(!script_result.valid())
    {
        sol::error err = script_result;
        qCritical() << err.what();
        return -1;
    }
    return 0;
}

std::string ScriptingEngine::callFuncWithClientContext(MapClientSession *client, const char *name, int arg1)
{
    m_private->m_lua["client"] = client;
    return callFunc(name,arg1);
}

std::string ScriptingEngine::callFunc(const char *name, int arg1)
{
    sol::protected_function funcwrap = m_private->m_lua[name];
    funcwrap.error_handler = m_private->m_lua["ErrorHandler"];
    if(!funcwrap.valid())
    {
        qCritical() << "Failed to retrieve script func:"<<name;
        return "";
    }
    auto result = funcwrap(arg1);
    if(!result.valid())
    {
        sol::error err = result;
        qCritical() << "Failed to run script func:"<<name<<err.what();
        return "";
    }
    return result.get<std::string>();
}
int ScriptingEngine::runScript(MapClientSession * client, const QString &script_contents, const char *script_name)
{
    m_private->m_lua["client"] = client;
    sol::load_result load_res=m_private->m_lua.load(script_contents.toStdString(),script_name);
    if(!load_res.valid())
    {
        sol::error err = load_res;
        sendInfoMessage(MessageChannel::ADMIN,err.what(),client);
        return -1;
    }
    sol::protected_function_result script_result = load_res();
    if(!script_result.valid())
    {
        sol::error err = script_result;
        sendInfoMessage(MessageChannel::ADMIN,err.what(),client);
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

