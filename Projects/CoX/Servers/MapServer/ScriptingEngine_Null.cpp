#include "ScriptingEngine.h"
ScriptingEngine::ScriptingEngine() {}
ScriptingEngine::~ScriptingEngine() {}
void ScriptingEngine::registerTypes() {}
int ScriptingEngine::runScript(const QString &script_contents, const char *script_name) {return -1;}
int ScriptingEngine::runScript(MapClient * client, const QString &script_contents, const char *script_name) {return -1;}
