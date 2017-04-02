/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

//#define DEBUG_INPUT
#define _USE_MATH_DEFINES
#include <cmath>
#include "Events/InputState.h"
#include "Entity.h"

static glm::quat QuaternionFromYawPitchRoll(const glm::vec3 &pyr)
{
    float pitch(pyr.x);
    float yaw(pyr.y);
    float roll(pyr.z);

    float rollOver2 = roll * 0.5f;
    float sinRollOver2 = (float)sin((double)rollOver2);
    float cosRollOver2 = (float)cos((double)rollOver2);
    float pitchOver2 = pitch * 0.5f;
    float sinPitchOver2 = (float)sin((double)pitchOver2);
    float cosPitchOver2 = (float)cos((double)pitchOver2);
    float yawOver2 = yaw * 0.5f;
    float sinYawOver2 = (float)sin((double)yawOver2);
    float cosYawOver2 = (float)cos((double)yawOver2);

    // X = PI is giving incorrect result (pitch)

    // Heading = Yaw
    // Attitude = Pitch
    // Bank = Roll

    glm::quat result;
    //result.X = cosYawOver2 * cosPitchOver2 * cosRollOver2 + sinYawOver2 * sinPitchOver2 * sinRollOver2;
    //result.Y = cosYawOver2 * cosPitchOver2 * sinRollOver2 - sinYawOver2 * sinPitchOver2 * cosRollOver2;
    //result.Z = cosYawOver2 * sinPitchOver2 * cosRollOver2 + sinYawOver2 * cosPitchOver2 * sinRollOver2;
    //result.W = sinYawOver2 * cosPitchOver2 * cosRollOver2 - cosYawOver2 * sinPitchOver2 * sinRollOver2;

    result.w = cosYawOver2 * cosPitchOver2 * cosRollOver2 - sinYawOver2 * sinPitchOver2 * sinRollOver2;
    result.x = sinYawOver2 * sinPitchOver2 * cosRollOver2 + cosYawOver2 * cosPitchOver2 * sinRollOver2;
    result.y = sinYawOver2 * cosPitchOver2 * cosRollOver2 + cosYawOver2 * sinPitchOver2 * sinRollOver2;
    result.z = cosYawOver2 * sinPitchOver2 * cosRollOver2 - sinYawOver2 * cosPitchOver2 * sinRollOver2;

    return result;
}
void InputState::serializeto(BitStream &) const
{
    assert(!"Not implemented");
}
InputStateStorage &InputStateStorage::operator =(const InputStateStorage &other)
{
    m_csc_deltabits=other.m_csc_deltabits;
    m_send_deltas=other.m_send_deltas;
    controlBits=other.controlBits;
    someOtherbits=other.someOtherbits;
    m_t1=other.m_t1;
    m_t2=other.m_t2;
    m_A_ang11_probably=other.m_A_ang11_probably;
    m_B_ang11_probably=other.m_B_ang11_probably;
    has_input_commit_guess=other.has_input_commit_guess;

    for(int i=0; i<3; ++i)
        if(other.pos_delta_valid[i])
            pos_delta[i] = other.pos_delta[i];
    bool update_needed=false;
    for(int i=0; i<3; ++i)
        if(other.pyr_valid[i]) {
            camera_pyr[i] = other.camera_pyr[i];
            update_needed = true;
        }
    if(update_needed) {
        direction = glm::angleAxis(camera_pyr[0], glm::vec3(1, 0, 0)) *
                    glm::angleAxis(camera_pyr[1], glm::vec3(0,-1, 0)) *
                    glm::angleAxis(camera_pyr[2], glm::vec3(0, 0, 1))
                ;
        //direction = glm::quat(camera_pyr[0], osg::X_AXIS, camera_pyr[1], -osg::Y_AXIS, 0, osg::Z_AXIS);
        //direction = QuaternionFromYawPitchRoll(camera_pyr);
    }
    return *this;
}

void InputStateStorage::processDirectionControl(int dir,int prev_time,int press_release)
{
    if(press_release)
    {
        fprintf(stderr,"pressed\n");
        switch(dir)
        {
            case 0: pos_delta[2] = 1.0f; break;
            case 1: pos_delta[2] = -1.0f; break;
            case 2: pos_delta[0] = 1.0f; break;
            case 3: pos_delta[0] = -1.0f; break;
            case 4: pos_delta[1] = 1.0f; break;
            case 5: pos_delta[1] = -1.0f; break;
        }
    }
    else {
        switch(dir)
        {
            case 0: pos_delta[2] =0.0f; break;
            case 1: pos_delta[2] =0.0f; break;
            case 2: pos_delta[0] =0.0f; break;
            case 3: pos_delta[0] =0.0f; break;
            case 4: pos_delta[1] =0.0f; break;
            case 5: pos_delta[1] =0.0f; break;
        }
    }
    switch(dir)
    {
        case 0:
        case 1: pos_delta_valid[2] = true; break;
        case 2:
        case 3: pos_delta_valid[0] = true; break;
        case 4:
        case 5: pos_delta_valid[1] = true; break;
    }
}
void InputState::partial_2(BitStream &bs)
{
    uint8_t control_id;
    //uint16_t v6;
    uint16_t time_since_prev;
    int v;
    static const char *control_name[] = {"FORWARD",
                                         "BACK",
                                         "LEFT",
                                         "RIGHT",
                                         "UP",
                                         "DOWN"};
    do
    {
        if(bs.GetBits(1))
            control_id = 8;
        else
            control_id = bs.GetBits(4);

        if(bs.GetBits(1))
            time_since_prev=bs.GetBits(2)+32; // delta from prev event
        else
            time_since_prev=bs.GetBits(m_data.m_csc_deltabits);
        switch(control_id)
        {
            case 0: case 1:
            case 2: case 3:
            case 4: case 5:
#ifdef DEBUG_INPUT
                fprintf(stderr,"%s  : %d - ",control_name[control_id],time_since_prev);
#endif
                m_data.processDirectionControl(control_id,time_since_prev,bs.GetBits(1));
                break;
            case 6:
            case 7:
            {
                v = bs.GetBits(11);
                // v = (x+pi)*(2048/2pi)
                // x = (v*(pi/1024))-pi
                float recovered = (float(v)/2048.0f)*(2*M_PI) - M_PI;
                m_data.pyr_valid[control_id==7] = true;
                if(control_id==6) //TODO: use camera_pyr.v[] here ?
                    m_data.camera_pyr[0] = recovered;
                else
                    m_data.camera_pyr[1] = recovered;
                fprintf(stderr,"Pyr %f : %f \n",m_data.camera_pyr.x,m_data.camera_pyr.y);
                break;
            }
            case 8:
                v = bs.GetBits(1);
#ifdef DEBUG_INPUT
                fprintf(stderr," C8[%d] ",v);
#endif
                if ( m_data.m_send_deltas )
                {
                    m_data.m_t1=bs.GetPackedBits(8);   // value - previous_value
                    m_data.m_t2=bs.GetPackedBits(8);   // time - previous_time
                }
                else
                {
                    m_data.m_send_deltas = true;
                    m_data.m_t1=bs.GetBits(32);       // value
                    m_data.m_t2=bs.GetPackedBits(10); // value - time
                }
#ifdef DEBUG_INPUT
                fprintf(stderr,"t1:t2 [%d,%d] ",m_data.m_t1,m_data.m_t2);
#endif
                if(bs.GetBits(1))
                {
                    m_data.field_20=bs.GetBits(8);
#ifdef DEBUG_INPUT
                fprintf(stderr,"v [%d] ",v);
#endif
                }
                break;
            case 9:
            {
                //a2->timerel_18
                //fprintf(stderr,"CtrlId %d  : %d - ",control_id,time_since_prev);
                uint8_t s=bs.GetBits(8);
#ifdef DEBUG_INPUT
                fprintf(stderr,"C9:%d ",s);
#endif
            }
                break;
            case 10: {
                uint8_t s=bs.GetBits(1);

#ifdef DEBUG_INPUT
                fprintf(stderr,"C10 : %d - ",time_since_prev);
                fprintf(stderr,"%d\n",s); //a2->timerel_18 & 1
#endif
            }
                break;
            default:
                assert(!"Unknown control_id");
        }

    } while(bs.GetBits(1));
}

void InputState::extended_input(BitStream &bs)
{
    m_data.has_input_commit_guess = bs.GetBits(1);
    if(m_data.has_input_commit_guess) // list of partial_2 follows
    {
        m_data.m_csc_deltabits=bs.GetBits(5) + 1; // number of bits in max_time_diff_ms
        m_data.someOtherbits = bs.GetBits(16);//ControlStateChange::field_8 or OptRel::field_19A8
        m_data.current_state_P = 0;
#ifdef DEBUG_INPUT
        fprintf(stderr,"CSC_DELTA[%x-%x] : ",m_data.m_csc_deltabits,m_data.someOtherbits);
#endif
        partial_2(bs);

    }
    m_data.controlBits = 0;
    for(int idx=0; idx<6; ++idx)
        m_data.controlBits |= (bs.GetBits(1))<<idx;
#ifdef DEBUG_INPUT
    if(m_data.controlBits)
        fprintf(stderr,"E input %x : ",m_data.controlBits);
#endif
    if(bs.GetBits(1))//if ( abs(s_prevTime - ms_time) < 1000 )
    {
        m_data.m_A_ang11_probably = bs.GetBits(11);//pak->SendBits(11, control_state.field_1C[0]);
        m_data.m_B_ang11_probably = bs.GetBits(11);//pak->SendBits(11, control_state.field_1C[1]);
#ifdef DEBUG_INPUT
        fprintf(stderr,"%f : %f",m_data.m_A_ang11_probably/2048.0,m_data.m_A_ang11_probably/2048.0);
#endif
    }
}
struct ControlState
{
    int client_timenow;
    int time_res;
    float timestep;
    float time_rel1C;
    uint64_t m_perf_cntr_diff;
    uint64_t m_perf_freq_diff;
    // recover actual ControlState from network data and previous entry
    void serializefrom_delta(BitStream &bs,const ControlState &prev)
    {
        client_timenow   = bs.GetPackedBits(1); // field_0 diff next-current
        time_res = bs.GetPackedBits(1); // time to next state ?
        timestep = bs.GetFloat(); // next state's timestep

        time_rel1C = timestep;
        if(bs.GetBits(1)) //timestep!=time_rel1C
            time_rel1C = bs.GetFloat();

        m_perf_cntr_diff = bs.Get64Bits(); //current_state->ticks - prev_state->ticks
        if(bs.GetBits(1))
        {
            // perf freq changed between current and prev
            m_perf_freq_diff = bs.Get64Bits();
        }
    }
    void serializefrom_base(BitStream &bs)
    {
        client_timenow   = bs.GetBits(32); // result of time(NULL)
        time_res = bs.GetBits(32); // result of timeGetTime()
        timestep = bs.GetFloat(); //client global TIMESTEP - per frame time

        time_rel1C = timestep;
        if(bs.GetBits(1)) //timestep!=time_rel1C
            time_rel1C = bs.GetFloat(); // simulation timestep ?

        m_perf_cntr_diff = bs.Get64Bits(); //next_state->ticks - current_state->ticks
        m_perf_cntr_diff = bs.Get64Bits(); //v7->perf_cntr1
    }
    void dump()
    {
#ifdef DEBUG_INPUT
        fprintf(stderr,"CSC: %d,%d, [%f,%f]",field0,time_res,timestep,time_rel1C);
        fprintf(stderr, "(%lld %lld)",m_perf_cntr_diff,m_perf_freq_diff);
#endif
    }
};
void InputState::serializefrom(BitStream &bs)
{
    m_data.m_send_deltas=false;
#ifdef DEBUG_INPUT
    fprintf(stderr,"\nI:");
#endif
    if(bs.GetBits(1))
        extended_input(bs);

    bool has_targeted_entity = bs.GetBits(1);
    int tgt_idx=bs.GetPackedBits(14); // targeted entity server index
    int ctrl_idx=0;
#ifdef DEBUG_INPUT
    fprintf(stderr,"T:[%d]",has_targeted_entity);
    if(has_targeted_entity)
        fprintf(stderr,"TI:[%d]",tgt_idx);
#endif
    ControlState prev_fld;
    while(bs.GetBits(1)) // receive control state array entries ?
    {
        ControlState fld;
        if(ctrl_idx)
        {
            fld.serializefrom_delta(bs,prev_fld);
        }
        else // initial values
        {
            fld.serializefrom_base(bs);
        }
        fld.dump();
        prev_fld = fld;
        ctrl_idx++;
    }
    recv_client_opts(bs); // g_pak contents will follow
#ifdef DEBUG_INPUT
    fprintf(stderr,"\n");
#endif
}
//TODO: use generic ReadableStructures here ?
void InputState::recv_client_opts(BitStream &bs)
{
    ClientOptions opts;
    ClientOption *entry;
    int opt_idx=0;
    int some_idx = bs.GetPackedBits(1);
    entry=opts.get(opt_idx)-1;
    glm::vec3 vec;
    while(some_idx!=0)
    {
        for(size_t i=0; i<entry->m_args.size(); i++)
        {
            ClientOption::Arg &arg=entry->m_args[i];
            switch ( arg.type )
            {
                case ClientOption::t_int:
                {
                    *((int32_t *)arg.tgt) = bs.GetPackedBits(1);
                    break;
                }
                case ClientOption::t_float:
                {
                    *((float *)arg.tgt)=bs.GetFloat();
                    break;
                }
                case ClientOption::t_quant_angle:
                {
                    printf("Quant:%d\n",bs.GetBits(14)); //quantized angle
                    break;
                }
                case ClientOption::t_string:
                case 4:
                {
                    QString v;
                    bs.GetString(v);
                    break;
                }
                case ClientOption::t_vec3:
                {
                    for (int j = 0; j < 3; ++j )
                    {
                        vec[j] = bs.GetFloat();
                    }
                    break;
                }
                default:
                    continue;
            }
        }
        some_idx = bs.GetPackedBits(1)-1;
        opt_idx++;
        entry=opts.get(opt_idx);
    }
}
