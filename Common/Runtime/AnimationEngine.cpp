#include "AnimationEngine.h"
using namespace SEGS;

AnimationEngine &AnimationEngine::get()
{
    static AnimationEngine s_instance;
    return s_instance;
}
