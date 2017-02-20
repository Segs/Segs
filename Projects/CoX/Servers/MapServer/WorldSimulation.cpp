#include "WorldSimulation.h"
#include "MapInstance.h"

#include <glm/gtx/vector_query.hpp>

void World::updateEntity(Entity *e, const ACE_Time_Value &dT) {
    if(glm::length2(e->inp_state.pos_delta)) {
        // todo: take into account time between updates
        glm::mat3 za = static_cast<glm::mat3>(e->inp_state.direction); // quat to mat4x4 conversion

        e->pos += ((za*e->inp_state.pos_delta)*float(dT.msec()))/50.0f;
    }
    if(e->m_is_logging_out) {
        e->m_time_till_logout -= dT.msec();
        if(e->m_time_till_logout<0)
            e->m_time_till_logout=0;
    }
}
