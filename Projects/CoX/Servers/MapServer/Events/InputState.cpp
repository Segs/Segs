/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2017 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

//#define DEBUG_INPUT
//#define DEBUG_INPUT2
#define _USE_MATH_DEFINES
#include "Events/InputState.h"
#include "Entity.h"
#include "GameData/CoHMath.h"
#include <QDebug>
#include <cmath>
enum BinaryControl
{
    FORWARD=0,
    BACKWARD=1,
    LEFT=2,
    RIGHT=3,
    UP=4,
    DOWN=5,
    PITCH=6,
    YAW=7,
};
void InputState::serializeto(BitStream &) const
{
    assert(!"Not implemented");
}
InputStateStorage &InputStateStorage::operator =(const InputStateStorage &other)
{
    m_csc_deltabits=other.m_csc_deltabits;
    m_send_deltas=other.m_send_deltas;
    controlBits=other.controlBits;
    send_id=other.send_id;
    m_time_diff1=other.m_time_diff1;
    m_time_diff2=other.m_time_diff2;
    has_input_commit_guess=other.has_input_commit_guess;
    m_received_server_update_id = other.m_received_server_update_id;
    m_no_coll = other.m_no_coll;

    for(int i=0; i<3; ++i)
    {
        if(other.pos_delta_valid[i])
            pos_delta[i] = other.pos_delta[i];
            
        if(other.m_orientation_pyr[i])
            m_orientation_pyr[i] = other.m_orientation_pyr[i];
    }
    bool update_needed=false;
    for(int i=0; i<3; ++i)
    {
        if(other.pyr_valid[i])
        {
            camera_pyr[i] = other.camera_pyr[i];
            update_needed = true;
        }
    }
    if(update_needed)
    {
        direction = fromCoHYpr(camera_pyr);
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
            case 0: pos_delta[2] = 1.0f; break; //FORWARD
            case 1: pos_delta[2] = -1.0f; break; //BACKWARD
            case 2: pos_delta[0] = -1.0f; break; //LEFT
            case 3: pos_delta[0] = 1.0f; break; //RIGHT
            case 4: pos_delta[1] = 1.0f; break; // UP
            case 5: pos_delta[1] = -1.0f; break; // DOWN
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
    uint16_t ms_since_prev;
    float v;
    static const char *control_name[] = {"FORWARD",
                                         "BACK",
                                         "LEFT",
                                         "RIGHT",
                                         "UP",
                                         "DOWN",
                                         "PITCH",
                                         "YAW",};
    do
    {
        if(bs.GetBits(1))
            control_id = 8;
        else
            control_id = bs.GetBits(4);

        if(bs.GetBits(1))
            ms_since_prev=bs.GetBits(2)+32; // delta from prev event
        else
            ms_since_prev=bs.GetBits(m_data.m_csc_deltabits);
        switch(control_id)
        {
            case FORWARD: case BACKWARD:
            case LEFT: case RIGHT:
            case UP: case DOWN:
#ifdef DEBUG_INPUT
                fprintf(stderr,"%s  : %d - ", control_name[control_id], ms_since_prev);
#endif
                m_data.processDirectionControl(control_id,ms_since_prev,bs.GetBits(1));
                break;
            case PITCH: // camera pitch (Insert/Delete keybinds)
            {
                v = AngleDequantize(bs.GetBits(11),11); // pitch
                m_data.pyr_valid[control_id==6] = true;
                m_data.camera_pyr[0] = v;
#ifdef DEBUG_INPUT2
                fprintf(stderr,"Pitch (%f): %f \n", m_data.m_orientation_pyr[0], m_data.camera_pyr.x);
#endif
                break;
            }
            case YAW: // camera yaw (Q or E keybinds)
            {
                v = AngleDequantize(bs.GetBits(11),11); // yaw
                m_data.pyr_valid[control_id==7] = true;
                m_data.camera_pyr[1] = v;
#ifdef DEBUG_INPUT2
                fprintf(stderr,"Yaw (%f): %f \n", m_data.m_orientation_pyr[1], m_data.camera_pyr.y);
#endif
                break;
            }
            case 8:
            {
                bool controls_disabled = bs.GetBits(1);
                if ( m_data.m_send_deltas )
                {
                    m_data.m_time_diff1=bs.GetPackedBits(8);   // value - previous_value
                    m_data.m_time_diff2=bs.GetPackedBits(8);   // time - previous_time
                }
                else
                {
                    m_data.m_send_deltas = true;
                    m_data.m_time_diff1=bs.GetBits(32);       // value
                    m_data.m_time_diff2=bs.GetPackedBits(10); // value - time
                }
                if(bs.GetBits(1))
                {
                    m_data.input_vel_scale=bs.GetBits(8);
                }
                break;
            }
            case 9:
            {
                m_data.m_received_server_update_id = bs.GetBits(8);
                break;
            }
            case 10:
            {
                m_data.m_no_coll = bs.GetBits(1);
                break;
            }
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
        m_data.send_id = bs.GetBits(16);
        m_data.current_state_P = 0;
#ifdef DEBUG_INPUT
        fprintf(stderr,"CSC_DELTA[%x-%x-%x] : ", m_data.m_csc_deltabits, m_data.send_id, m_data.current_state_P);
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
        m_data.m_orientation_pyr[0] = AngleDequantize(bs.GetBits(11),11); //pak->SendBits(11, control_state.field_1C[0]);
        m_data.m_orientation_pyr[1] = AngleDequantize(bs.GetBits(11),11); //pak->SendBits(11, control_state.field_1C[1]);
#ifdef DEBUG_INPUT
        fprintf(stderr,"%f : %f",m_data.m_orientation_pyr[0],m_data.m_orientation_pyr[1]);;
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
        m_perf_freq_diff = bs.Get64Bits(); //v7->perf_cntr1
    }
    void dump()
    {
#ifdef DEBUG_INPUT
        fprintf(stderr,"CSC: %d,%d, [%f,%f]",client_timenow,time_res,timestep,time_rel1C);
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
    bs.ByteAlign(true,false);
    if(bs.GetReadableBits()>0) {
        m_user_commands.Reset();
        m_user_commands.appendBitStream(bs);
        // all remaining bits were moved to m_user_commands.
        bs.SetReadPos(bs.GetWritePos());
    }
#ifdef DEBUG_INPUT
    fprintf(stderr,"\n");
#endif
}
//TODO: use generic ReadableStructures here ?
void InputState::recv_client_opts(BitStream &bs)
{
    ClientOptions opts;
    ClientOption *entry;
    glm::vec3 vec;
    int cmd_idx;
    while((cmd_idx = bs.GetPackedBits(1))!=0)
    {
        entry=opts.get(cmd_idx-1);
        if (!entry) 
        {
            qWarning() << "recv_client_opts missing opt for cmd index"<<cmd_idx-1;
            continue;
        }
        for(ClientOption::Arg &arg : entry->m_args)
        {
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
                    float * tgt_angle = (float *)arg.tgt;
                    *tgt_angle = AngleDequantize(bs.GetBits(14),14);
                    printf("Quant angle res:%f\n",*tgt_angle); //dequantized angle
                    break;
                }
                case ClientOption::t_string:
                case ClientOption::t_sentence:
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
    }
}
