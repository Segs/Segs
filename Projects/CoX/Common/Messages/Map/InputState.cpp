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

#include "GameData/Character.h"
#include "GameData/Entity.h"
#include "GameData/Movement.h"
#include "GameData/CoHMath.h"
#include "Logging.h"

#include <glm/gtc/constants.hpp>
#include <cmath>
#include <chrono>

using namespace SEGSEvents;

void RecvInputState::receiveControlState(BitStream &bs) // formerly partial_2
{
    uint8_t     control_id = 0;
    uint32_t    ms_since_prev = 0;
    float       angle = 0.0f;

    do
    {
        if(bs.GetBits(1))
            control_id = 8;
        else
            control_id = bs.GetBits(4);

        if(bs.GetBits(1))
            ms_since_prev = bs.GetBits(2)+32; // delta from prev event
        else
            ms_since_prev = bs.GetBits(m_next_state.m_csc_deltabits);

        if(control_id < 8)
                    m_next_state.m_input_received = true;

        m_next_state.m_ms_since_prev = ms_since_prev;

        switch(control_id)
        {
            case FORWARD: case BACKWARD:
            case LEFT: case RIGHT:
            case UP: case DOWN:
            {
                bool keypress_state = bs.GetBits(1); // get keypress state
                auto now_ms = std::chrono::steady_clock::now();
                m_next_state.m_svr_keypress_time[control_id] = now_ms - m_next_state.m_keypress_start[control_id];

                m_next_state.m_control_bits[control_id] = keypress_state; // save control_bits state
                processDirectionControl(&m_next_state, control_id, ms_since_prev, keypress_state);

                qCDebug(logInput, "key released %d", control_id);
                qCDebug(logInput, "svr vs client keypress time: %f %f : %f", 
                        m_next_state.m_svr_keypress_time[control_id].count(),
                        m_next_state.m_keypress_time[control_id], m_next_state.m_ms_since_prev);
                break;
            }
            case PITCH: // camera pitch (Insert/Delete keybinds)
            {
                angle = AngleDequantize(bs.GetBits(11),11); // pitch
                m_next_state.m_pyr_valid[0] = true;
                m_next_state.m_camera_pyr[0] = angle;
                qCDebug(logInput, "Pitch (%f): %f", m_next_state.m_orientation_pyr[0], m_next_state.m_camera_pyr.x);
                break;
            }
            case YAW: // camera yaw (Q or E keybinds)
            {
                angle = AngleDequantize(bs.GetBits(11),11); // yaw
                m_next_state.m_pyr_valid[1] = true;
                m_next_state.m_camera_pyr[1] = angle;
                qCDebug(logInput, "Yaw (%f): %f", m_next_state.m_orientation_pyr[1], m_next_state.m_camera_pyr.y);
                break;
            }
            case 8:
            {
                m_next_state.m_controls_disabled = bs.GetBits(1);
                if( m_next_state.m_full_timeupdate ) // sent_run_physics. maybe autorun? maybe is_running?
                {
                    m_next_state.m_time_diff1 = bs.GetPackedBits(8);   // value - previous_value
                    m_next_state.m_time_diff2 = bs.GetPackedBits(8);   // time - previous_time
                }
                else
                {
                    m_next_state.m_full_timeupdate = true;
                    m_next_state.m_time_diff1 = bs.GetBits(32);       // value
                    m_next_state.m_time_diff2 = bs.GetPackedBits(10); // value - time
                }

                /*
                qCDebug(logMovement, "Controls Disabled: %d  time_diff1: %d \t time_diff2: %d",
                        m_next_state.m_controls_disabled, m_next_state.m_time_diff1, m_next_state.m_time_diff2);
                */

                if(bs.GetBits(1)) // if true velocity scale < 255
                {
                    m_next_state.m_velocity_scale = bs.GetBits(8);
                    qCDebug(logInput, "Velocity Scale: %d", m_next_state.m_velocity_scale);
                }
                else
                    m_next_state.m_velocity_scale = 255;

                break;
            }
            case 9:
            {
                m_next_state.m_every_4_ticks = bs.GetBits(8); // value goes to 0 every 4 ticks. Some kind of send_partial flag

                if(m_next_state.m_every_4_ticks != 1)
                    qCDebug(logInput, "This goes to 0 every 4 ticks: %d", m_next_state.m_every_4_ticks);

                break;
            }
            case 10:
            {
                m_next_state.m_no_collision = bs.GetBits(1);
                qCDebug(logInput, "Collision: %d", m_next_state.m_no_collision);
                break;
            }
            default:
                assert(!"Unknown control_id");
        }

    } while(bs.GetBits(1));

    //qCDebug(logInput, "recv control_id 9 %f", m_next_state.m_every_4_ticks);
}

void RecvInputState::extended_input(BitStream &bs)
{
    bool keypress_state;

    m_next_state.m_full_input_packet = bs.GetBits(1);
    if(m_next_state.m_full_input_packet) // list of partial_2 follows
    {
        m_next_state.m_csc_deltabits = bs.GetBits(5) + 1; // number of bits in max_time_diff_ms
        m_next_state.m_send_id = bs.GetBits(16);

        //qCDebug(logInput, "CSC_DELTA[%x-%x-%x] : ", m_current.m_csc_deltabits, m_current.m_send_id, m_current.current_state_P);
        receiveControlState(bs); // formerly partial_2
    }

    // Key Pressed/Held
    for(int idx=0; idx<6; ++idx)
    {
        keypress_state = bs.GetBits(1);
        m_next_state.m_control_bits[idx] = keypress_state;
        if(keypress_state==true)
        {
            m_next_state.m_keypress_start[idx] = std::chrono::steady_clock::now();
            processDirectionControl(&m_next_state, idx, 0, keypress_state);
            qCDebug(logInput, "keypress down %d", idx);
        }
    }

    if(m_next_state.m_control_bits != 0)
            m_next_state.m_input_received = true;

    if(bs.GetBits(1)) //if( abs(s_prevTime - ms_time) < 1000 )
    {
        m_next_state.m_orientation_pyr[0] = AngleDequantize(bs.GetBits(11),11);
        m_next_state.m_orientation_pyr[1] = AngleDequantize(bs.GetBits(11),11);
        qCDebug(logOrientation, "extended pitch: %f \tyaw: %f", m_next_state.m_orientation_pyr[0], m_next_state.m_orientation_pyr[1]);
    }
}

void RecvInputState::serializefrom(BitStream &bs)
{
    m_next_state.m_full_timeupdate = false; // possibly some kind of full_update flag that is used elsewhere also

    if(bs.GetBits(1))
        extended_input(bs);

    m_next_state.m_has_target = bs.GetBits(1);
    m_next_state.m_target_idx = bs.GetPackedBits(14); // targeted entity server_index

    qCDebug(logTarget, "Has Target? %d | TargetIdx: %d", m_next_state.m_has_target, m_next_state.m_target_idx);

    TimeState prev_fld;
    int ctrl_idx = 0;
    while(bs.GetBits(1)) // receive control state array entries ?
    {
        TimeState fld;
        if(ctrl_idx)
        {
            fld.serializefrom_delta(bs, prev_fld);
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

void RecvInputState::serializeto(BitStream &) const
{
    assert(!"Not implemented");
}

//TODO: use generic ReadableStructures here ?
void RecvInputState::recv_client_opts(BitStream &bs)
{
    ClientOptions opts;
    ClientOption *entry;
    glm::vec3 vec;
    int cmd_idx;

    while((cmd_idx = bs.GetPackedBits(1))!=0)
    {
        entry=opts.get(cmd_idx-1);
        if(!entry)
        {
            qWarning() << "recv_client_opts missing opt for cmd index" << cmd_idx-1;
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
