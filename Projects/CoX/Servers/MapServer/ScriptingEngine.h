#ifndef SCRIPTINGENGINE_H
#define SCRIPTINGENGINE_H
#include <memory>

class ScriptingEngine
{
    struct ScriptingEnginePrivate;
public:
    ScriptingEngine();
    ~ScriptingEngine();
private:
    std::unique_ptr<ScriptingEnginePrivate> m_private;
};

#endif // SCRIPTINGENGINE_H
