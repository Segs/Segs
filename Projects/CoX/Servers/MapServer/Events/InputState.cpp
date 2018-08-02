/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServerEvents Projects/CoX/Servers/MapServer/Events
 * @{
 */

#include "InputState.h"

#include "NetStructures/Character.h"
#include "NetStructures/Entity.h"
#include "GameData/CoHMath.h"
#include "Logging.h"

#include <glm/gtc/constants.hpp>
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
    LAST_BINARY_VALUE=5,
    LAST_QUANTIZED_VALUE=7,
};

void InputState::serializeto(BitStream &) const
{
    assert(!"Not implemented");
}

InputStateStorage &InputStateStorage::operator =(const InputStateStorage &other)
{
    m_csc_deltabits             = other.m_csc_deltabits;
    m_send_deltas               = other.m_send_deltas;
    m_control_bits              = other.m_control_bits;
    m_send_id                   = other.m_send_id;
    m_time_diff1                = other.m_time_diff1;
    m_time_diff2                = other.m_time_diff2;
    m_key_released              = other.m_key_released;
    m_received_server_update_id = other.m_received_server_update_id;
    m_no_collision              = other.m_no_collision;
    m_controls_disabled         = other.m_controls_disabled;

    for(int i=0; i<3; ++i)
    {
        if(other.pos_delta_valid[i])
            pos_delta[i] = other.pos_delta[i];
    }
    bool update_needed=false;
    for(int i=0; i<3; ++i)
    {
        if(other.pyr_valid[i])
            m_camera_pyr[i] = other.m_camera_pyr[i];

        if(other.m_orientation_pyr[i])
        {
            qCDebug(logOrientation) << other.m_orientation_pyr[i];
            m_orientation_pyr[i] = other.m_orientation_pyr[i];
            update_needed = true;
        }
    }
    if(update_needed)
        m_direction = fromCoHYpr(m_orientation_pyr);

    qCDebug(logOrientation) << m_direction.w << m_direction.x << m_direction.y << m_direction.z;
    return *this;
}

void InputStateStorage::processDirectionControl(int dir,int /*prev_time*/,int press_release)
{
    if(press_release)
    {
        qCDebug(logInput, "pressed: %d", dir);
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
                qCDebug(logInput, "%s  : %d - ", control_name[control_id], ms_since_prev);
                m_data.processDirectionControl(control_id,ms_since_prev,bs.GetBits(1));
                break;
            case PITCH: // camera pitch (Insert/Delete keybinds)
            {
                v = AngleDequantize(bs.GetBits(11),11); // pitch
                m_data.pyr_valid[0] = true;
                m_data.m_camera_pyr[0] = v;
                qCDebug(logOrientation, "Pitch (%f): %f", m_data.m_orientation_pyr[0], m_data.m_camera_pyr.x);
                break;
            }
            case YAW: // camera yaw (Q or E keybinds)
            {
                v = AngleDequantize(bs.GetBits(11),11); // yaw
                m_data.pyr_valid[1] = true;
                m_data.m_camera_pyr[1] = v;
                qCDebug(logOrientation, "Yaw (%f): %f", m_data.m_orientation_pyr[1], m_data.m_camera_pyr.y);
                break;
            }
            case 8:
            {
                m_data.m_controls_disabled = bs.GetBits(1);
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
                    m_data.m_input_vel_scale=bs.GetBits(8);
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
                m_data.m_no_collision = bs.GetBits(1);
                break;
            }
            default:
                assert(!"Unknown control_id");
        }

    } while(bs.GetBits(1));
}

void InputState::extended_input(BitStream &bs)
{
    m_data.m_key_released = bs.GetBits(1);
    if(m_data.m_key_released) // list of partial_2 follows
    {
        m_data.m_csc_deltabits=bs.GetBits(5) + 1; // number of bits in max_time_diff_ms
        m_data.m_send_id = bs.GetBits(16);
        m_data.current_state_P = nullptr;
        qCDebug(logInput, "CSC_DELTA[%x-%x-%x] : ", m_data.m_csc_deltabits, m_data.m_send_id, m_data.current_state_P);
        partial_2(bs);

    }
    m_data.m_control_bits = 0;
    for(int idx=0; idx<6; ++idx)
        m_data.m_control_bits |= (bs.GetBits(1))<<idx;

    if(m_data.m_control_bits)
        qCDebug(logInput, "E input %x : ",m_data.m_control_bits);

    if(bs.GetBits(1)) //if ( abs(s_prevTime - ms_time) < 1000 )
    {
        m_data.m_orientation_pyr[0] = AngleDequantize(bs.GetBits(11),11);
        m_data.m_orientation_pyr[1] = AngleDequantize(bs.GetBits(11),11);
        qCDebug(logOrientation, "%f : %f",m_data.m_orientation_pyr[0],m_data.m_orientation_pyr[1]);
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
    void serializefrom_delta(BitStream &bs,const ControlState &/*prev*/)
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
        qCDebug(logInput, "CSC: %d,%d, [%f,%f]", client_timenow, time_res, timestep,time_rel1C);
        qCDebug(logInput, "(%lld %lld)", m_perf_cntr_diff, m_perf_freq_diff);
    }
};

void InputState::serializefrom(BitStream &bs)
{
    m_data.m_send_deltas=false;

    if(bs.GetBits(1))
    {
        m_data.m_input_received = true;
        extended_input(bs);
    }
    else
        m_data.m_input_received = false;

    m_has_target = bs.GetBits(1);
    m_target_idx = bs.GetPackedBits(14); // targeted entity server_index

    if(m_has_target)
        qCDebug(logTarget, "Has Target? %d | TargetIdx: %d", m_has_target, m_target_idx);

    int ctrl_idx=0;
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
    if(bs.GetReadableBits()>0)
    {
        m_user_commands.ResetOffsets();
        bs.ByteAlign(true,false);
        m_user_commands.StoreBitArray(bs.read_ptr(),bs.GetReadableBits());
        // all remaining bits were moved to m_user_commands.
        bs.SetReadPos(bs.GetWritePos());
    }
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
                    qCDebug(logInput, "Quant angle res:%f", *tgt_angle); //dequantized angle
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

//! @}
