/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
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

static const char *control_name[] = {
    "FORWARD",
    "BACK",
    "LEFT",
    "RIGHT",
    "UP",
    "DOWN",
    "PITCH",
    "YAW",
};

void InputState::serializeto(BitStream &) const
{
    assert(!"Not implemented");
}

InputStateStorage &InputStateStorage::operator =(const InputStateStorage &other)
{
    m_csc_deltabits             = other.m_csc_deltabits;
    m_autorun               = other.m_autorun;
    m_control_bits              = other.m_control_bits;
    m_send_id                   = other.m_send_id;
    m_time_diff1                = other.m_time_diff1;
    m_time_diff2                = other.m_time_diff2;
    has_input_commit_guess      = other.has_input_commit_guess;
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

void InputStateStorage::processDirectionControl(uint8_t dir, int prev_time, int press_release)
{
    float delta = 0.0f;

    if(press_release)
        delta = 1.0f;

    qCDebug(logInput, "Pressed dir: %s \t prev_time: %d \t press_release: %d", control_name[dir], prev_time, press_release);
    switch(dir)
    {
        case 0: pos_delta[2] = delta; break;    //FORWARD
        case 1: pos_delta[2] = -delta; break;   //BACKWARD
        case 2: pos_delta[0] = -delta; break;   //LEFT
        case 3: pos_delta[0] = delta; break;    //RIGHT
        case 4: pos_delta[1] = delta; break;    // UP
        case 5: pos_delta[1] = -delta; break;   // DOWN
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
    uint8_t     control_id;
    uint32_t    ms_since_prev;
    float       v;

    do
    {
        if(bs.GetBits(1))
            control_id = 8;
        else
            control_id = bs.GetBits(4);

        if(bs.GetBits(1))
            ms_since_prev = bs.GetBits(2)+32; // delta from prev event
        else
            ms_since_prev = bs.GetBits(m_data.m_csc_deltabits);

        switch(control_id)
        {
            case FORWARD: case BACKWARD:
            case LEFT: case RIGHT:
            case UP: case DOWN:
            {
                bool keypress_state = bs.GetBits(1); // get keypress state
                m_data.m_control_bits |= keypress_state << control_id; // save control_bits
                m_data.processDirectionControl(control_id, ms_since_prev, keypress_state); // TODO: this should be moved out of partial_2?
                break;
            }
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
                if ( m_data.m_autorun ) // sent_run_physics. maybe autorun? maybe is_running?
                {
                    m_data.m_time_diff1 = bs.GetPackedBits(8);   // value - previous_value
                    m_data.m_time_diff2 = bs.GetPackedBits(8);   // time - previous_time
                }
                else
                {
                    m_data.m_autorun = true;
                    m_data.m_time_diff1 = bs.GetBits(32);       // value
                    m_data.m_time_diff2 = bs.GetPackedBits(10); // value - time
                }
                /*
                qCDebug(logInput, "Controls Disabled: %d \t time_diff1: %d \t time_diff2: %d",
                        m_data.m_input_vel_scale, m_data.m_time_diff1, m_data.m_time_diff2);
                */

                if(bs.GetBits(1)) // if true velocity scale < 255
                {
                    m_data.m_input_vel_scale = bs.GetBits(8);
                    qCDebug(logMovement, "Velocity Scale: %d", m_data.m_input_vel_scale);
                }
                break;
            }
            case 9:
            {
                m_data.m_received_server_update_id = bs.GetBits(8); // value is always 1?
                //qCDebug(logInput, "Server Update ID: %d", m_data.m_received_server_update_id);
                break;
            }
            case 10:
            {
                m_data.m_no_collision = bs.GetBits(1);
                qCDebug(logInput, "Collision: %d", m_data.m_no_collision);
                break;
            }
            default:
                assert(!"Unknown control_id");
        }

    } while(bs.GetBits(1));
}

void InputState::extended_input(BitStream &bs)
{
    bool keypress_state;

    m_data.has_input_commit_guess = bs.GetBits(1);
    if(m_data.has_input_commit_guess) // list of partial_2 follows
    {
        m_data.m_csc_deltabits=bs.GetBits(5) + 1; // number of bits in max_time_diff_ms
        m_data.m_send_id = bs.GetBits(16);
        m_data.current_state_P = nullptr;

        //qCDebug(logInput, "CSC_DELTA[%x-%x-%x] : ", m_data.m_csc_deltabits, m_data.m_send_id, m_data.current_state_P);
        partial_2(bs);
    }

    for(int idx=0; idx<6; ++idx)
    {
        keypress_state = bs.GetBits(1);
        m_data.m_control_bits ^= keypress_state << idx;
        // TODO: do something with these control_bits now
        //m_data.processDirectionControl(idx, 0, keypress_state);
    }

    if(bs.GetBits(1)) //if ( abs(s_prevTime - ms_time) < 1000 )
    {
        m_data.m_orientation_pyr[0] = AngleDequantize(bs.GetBits(11),11);
        m_data.m_orientation_pyr[1] = AngleDequantize(bs.GetBits(11),11);
        qCDebug(logOrientation, "pitch: %f \tyaw: %f", m_data.m_orientation_pyr[0], m_data.m_orientation_pyr[1]);
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
    m_data.m_autorun=false;

    if(bs.GetBits(1))
        extended_input(bs);

    m_has_target = bs.GetBits(1);
    m_target_idx = bs.GetPackedBits(14); // targeted entity server_index
    int ctrl_idx=0;

    qCDebug(logTarget, "Has Target? %d | TargetIdx: %d", m_has_target, m_target_idx);

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
