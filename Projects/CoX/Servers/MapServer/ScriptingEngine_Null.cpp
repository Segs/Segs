#include "ScriptingEngine.h"
ScriptingEngine::ScriptingEngine() {}
ScriptingEngine::~ScriptingEngine() {}
void ScriptingEngine::registerTypes() {}
int ScriptingEngine::runScript(const QString &script_contents, const char *script_name) {return -1;}
int ScriptingEngine::runScript(MapClientSession * client, const QString &script_contents, const char *script_name) {return -1;}
int ScriptingEngine::loadAndRunFile(const QString &path) { return -1; }
std::string ScriptingEngine::callFunc(const char *name, int arg1) { return ""; }
std::string ScriptingEngine::callFuncWithClientContext(MapClientSession *client, const char *name, int arg1) { return "";}
