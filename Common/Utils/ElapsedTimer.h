#pragma once

#include <EASTL/chrono.h>

namespace SEGS
{
    class ElapsedTimer
    {
    public:
        ElapsedTimer() { restart(); }
        void start() { m_start = eastl::chrono::steady_clock::now();}
        void restart() { start(); }
        // time elapsed since start/restart in milliseconds
        [[nodiscard]] int64_t elapsed() const {
            return eastl::chrono::duration_cast<eastl::chrono::milliseconds>(eastl::chrono::steady_clock::now() - m_start).count();
        }
        eastl::chrono::milliseconds elapsedAndRestart() {
            auto now = eastl::chrono::steady_clock::now();
            auto elapsed = eastl::chrono::duration_cast<eastl::chrono::milliseconds>(now - m_start);
            m_start = now;
            return elapsed;
        }
    private:
        eastl::chrono::steady_clock::time_point m_start;
    };
}