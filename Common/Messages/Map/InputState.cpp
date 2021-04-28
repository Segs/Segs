/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
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
#include "Components/Logging.h"

#include <glm/gtc/constants.hpp>
#include <cmath>
#include <chrono>

using namespace SEGSEvents;

void RecvInputState::receiveControlStateChanges(BitStream &bs) // formerly partial_2
{
    uint8_t csc_deltabits = bs.GetBits(5) + 1; // number of bits in max_time_diff_ms
    m_input_state_change.m_first_control_state_change_id =  bs.GetBits(16);

    bool control_id_8_received_at_least_once = false;

    do
    {
        uint8_t control_id = 0;
        if(bs.GetBits(1))
            control_id = 8;
        else
            control_id = bs.GetBits(4);

        uint32_t time_since_prev_ms;
        if(bs.GetBits(1))
            time_since_prev_ms = bs.GetBits(2)+32; // delta from prev event
        else
            time_since_prev_ms = bs.GetBits(csc_deltabits);

        ControlStateChange csc;
        csc.control_id = control_id;
        csc.time_since_prev_ms = time_since_prev_ms;

        switch(control_id)
        {
            case FORWARD: case BACKWARD:
            case LEFT: case RIGHT:
            case UP: case DOWN:
            {
                csc.data.key_state = bs.GetBits(1); // get keypress state
                break;
            }
            case PITCH: // camera pitch (Insert/Delete keybinds)
            {
                csc.data.angle = AngleDequantize(bs.GetBits(11),11);
                break;
            }
            case YAW: // camera yaw (Q or E keybinds)
            {
                csc.data.angle = AngleDequantize(bs.GetBits(11),11); // yaw
                break;
            }
            case 8:
            {
                csc.data.control_id_8.controls_disabled = bs.GetBits(1);
                if( control_id_8_received_at_least_once ) // delta from previous
                {
                    csc.data.control_id_8.time_diff_1 = bs.GetPackedBits(8);   // value - previous_value
                    csc.data.control_id_8.time_diff_2 = bs.GetPackedBits(8);   // time - previous_time
                }
                else
                {
                    control_id_8_received_at_least_once = true;
                    csc.data.control_id_8.time_diff_1 = bs.GetBits(32);       // value
                    csc.data.control_id_8.time_diff_2 = bs.GetPackedBits(10); // value - time
                }

                if(bs.GetBits(1)) // if true velocity scale < 255
                    csc.data.control_id_8.velocity_scale = bs.GetBits(8);
                else
                    csc.data.control_id_8.velocity_scale = 255;

                break;
            }
            case 9:
            {
                csc.data.every_4_ticks = bs.GetBits(8); // value goes to 0 every 4 ticks. Some kind of send_partial flag
                break;
            }
            case 10:
            {
                csc.data.no_collision = bs.GetBits(1);
                break;
            }
            default:
                assert(!"Unknown control_id");
        }

        m_input_state_change.m_control_state_changes.push_back(csc);

    } while(bs.GetBits(1));
}

void RecvInputState::extended_input(BitStream &bs)
{
    bool contains_control_state_changes = bs.GetBits(1);
    if(contains_control_state_changes) // list of partial_2 follows
    {
        receiveControlStateChanges(bs); // formerly partial_2
    }

    // Key Pressed/Held
    m_input_state_change.m_has_keys = true;
    for(int idx=0; idx<6; ++idx)
    {
        m_input_state_change.m_keys[idx] = bs.GetBits(1);
    }

    if(bs.GetBits(1))
    {
        m_input_state_change.m_has_pitch_and_yaw = true;
        m_input_state_change.m_pitch = AngleDequantize(bs.GetBits(11),11);
        m_input_state_change.m_yaw = AngleDequantize(bs.GetBits(11),11);
    }
}

void RecvInputState::serializefrom(BitStream &bs)
{
    if(bs.GetBits(1))
        extended_input(bs);

    m_input_state_change.m_has_target = bs.GetBits(1);
    m_input_state_change.m_target_idx = bs.GetPackedBits(14); // targeted entity server_index

    qCDebug(logTarget, "Has Target? %d | TargetIdx: %d", m_input_state_change.m_has_target, m_input_state_change.m_target_idx);

    while(bs.GetBits(1)) // receive control state array entries ?
    {
        TimeState time_state;
        if(m_input_state_change.m_time_state.size())
        {
            time_state.serializefrom_delta(bs, m_input_state_change.m_time_state.back());
        }
        else // initial values
        {
            time_state.serializefrom_base(bs);
        }
        m_input_state_change.m_time_state.push_back(time_state);
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
