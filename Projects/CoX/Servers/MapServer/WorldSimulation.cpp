#include "WorldSimulation.h"
#include "MapInstance.h"



void World::updateEntity(Entity *e, const ACE_Time_Value &dT) {
    if(e->inp_state.pos_delta.length2()) {
        // todo: take into account time between updates
        osg::Matrix za(osg::Matrix::rotate(e->inp_state.direction));

        e->pos += ((za*e->inp_state.pos_delta)*dT.msec())/50.0f;
    }
    if(e->m_is_logging_out) {
        e->m_time_till_logout -= dT.msec();
        if(e->m_time_till_logout<0)
            e->m_time_till_logout=0;
    }
}
