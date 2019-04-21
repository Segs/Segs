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

void RecvInputState::receiveControlStateChanges(BitStream &bs) // formerly partial_2
{
    uint8_t csc_deltabits = bs.GetBits(5) + 1; // number of bits in max_time_diff_ms
    uint16_t send_id = bs.GetBits(16);
    //qCDebug(logInput, "CSC_DELTA[%x-%x-%x] : ", m_current.m_csc_deltabits, m_current.m_send_id, m_current.current_state_P);

    uint16_t control_state_change_id = send_id;
    ControlStateChangesForTick changes_for_tick;
    changes_for_tick.first_change_id = control_state_change_id;

    uint8_t control_id = 0;
    do
    {
        if(bs.GetBits(1))
            control_id = 8;
        else
            control_id = bs.GetBits(4);

        uint32_t ms_since_prev;
        if(bs.GetBits(1))
            ms_since_prev = bs.GetBits(2)+32; // delta from prev event
        else
            ms_since_prev = bs.GetBits(csc_deltabits);

        changes_for_tick.tick_length_ms += ms_since_prev;

        switch(control_id)
        {
            case FORWARD: case BACKWARD:
            case LEFT: case RIGHT:
            case UP: case DOWN:
            {
                int32_t keypress_state = bs.GetBits(1); // get keypress state

                qCDebug(logInput, "key %d = %d", control_id, keypress_state);

                ControlStateChangesForTick::KeyChange key_change;
                key_change.key = control_id;
                key_change.state = keypress_state;
                key_change.offset_from_tick_start_ms = changes_for_tick.tick_length_ms;
                changes_for_tick.key_changes.push_back(key_change);// todo(jbr) could we just have an array of 6? see if it's possible for down and up to both be present in 1 tick

                break;
            }
            case PITCH: // camera pitch (Insert/Delete keybinds)
            {
                changes_for_tick.pitch_changed = true;
                changes_for_tick.pitch = AngleDequantize(bs.GetBits(11),11);
                qCDebug(logInput, "Pitch %f", changes_for_tick.pitch);
                break;
            }
            case YAW: // camera yaw (Q or E keybinds)
            {
                changes_for_tick.yaw_changed = true;
                changes_for_tick.yaw = AngleDequantize(bs.GetBits(11),11); // yaw
                qCDebug(logInput, "Yaw %f", changes_for_tick.yaw);
                break;
            }
            case 8:
            {
                changes_for_tick.controls_disabled = bs.GetBits(1);
                if( m_next_state.m_control_state_changes.size() ) // delta from previous
                {
                    changes_for_tick.time_diff_1 = bs.GetPackedBits(8);   // value - previous_value
                    changes_for_tick.time_diff_2 = bs.GetPackedBits(8);   // time - previous_time
                }
                else
                {
                    changes_for_tick.time_diff_1 = bs.GetBits(32);       // value
                    changes_for_tick.time_diff_2 = bs.GetPackedBits(10); // value - time
                }

                if(bs.GetBits(1)) // if true velocity scale < 255
                {
                    changes_for_tick.velocity_scale = bs.GetBits(8);
                }
                else
                {
                    changes_for_tick.velocity_scale = 255;
                }

                // end of tick, finish collecting control changes and add to array
                changes_for_tick.last_change_id = control_state_change_id;
                m_next_state.m_control_state_changes.push_back(changes_for_tick);

                // clear accumulated control changes ready for the next tick
                changes_for_tick = ControlStateChangesForTick();
                changes_for_tick.first_change_id = control_state_change_id + 1;

                break;
            }
            case 9:
            {
                changes_for_tick.every_4_ticks_changed = true;
                changes_for_tick.every_4_ticks = bs.GetBits(8); // value goes to 0 every 4 ticks. Some kind of send_partial flag

                // todo(jbr) maybe only log all of this stuff in the movement code which uses it?
                qCDebug(logInput, "This goes to 0 every 4 ticks: %d", changes_for_tick.every_4_ticks);

                break;
            }
            case 10:
            {
                changes_for_tick.no_collision_changed = true;
                changes_for_tick.no_collision = bs.GetBits(1);
                qCDebug(logInput, "Collision: %d", changes_for_tick.no_collision);
                break;
            }
            default:
                assert(!"Unknown control_id");
        }

        ++control_state_change_id;

    } while(bs.GetBits(1));

    qCDebug(logInput, "End ControlStateChange");

    Q_ASSERT(control_id == 8); // I make some assumptions that these can't contain partial tick control state changes

    //qCDebug(logInput, "recv control_id 9 %f", m_next_state.m_every_4_ticks);
}

void RecvInputState::extended_input(BitStream &bs)
{
    bool contains_control_state_changes = bs.GetBits(1);
    if(contains_control_state_changes) // list of partial_2 follows
    {
        receiveControlStateChanges(bs); // formerly partial_2
    }

    // Key Pressed/Held
    m_next_state.m_has_keys = true;
    for(int idx=0; idx<6; ++idx)
    {
        bool keypress_state = bs.GetBits(1);
        m_next_state.m_keys[idx] = keypress_state;
        if (keypress_state)
        {
            qCDebug(logInput, "keypress down %d", idx);
        }
    }
    // todo(jbr) when processing these, just assert these match with what we've got

    if(bs.GetBits(1))
    {
        // todo(jbr) when do these get sent? every second?
        m_next_state.m_has_pitch_and_yaw = true;
        m_next_state.m_pitch = AngleDequantize(bs.GetBits(11),11);
        m_next_state.m_yaw = AngleDequantize(bs.GetBits(11),11);
        qCDebug(logInput, "extended pitch: %f \tyaw: %f", m_next_state.m_pitch, m_next_state.m_yaw);
    }
}

void RecvInputState::serializefrom(BitStream &bs)
{
    if(bs.GetBits(1))
        extended_input(bs);

    m_next_state.m_has_target = bs.GetBits(1);
    m_next_state.m_target_idx = bs.GetPackedBits(14); // targeted entity server_index

    qCDebug(logTarget, "Has Target? %d | TargetIdx: %d", m_next_state.m_has_target, m_next_state.m_target_idx);

    while(bs.GetBits(1)) // receive control state array entries ?
    {
        TimeState time_state;
        if(m_next_state.m_time_state.size())
        {
            time_state.serializefrom_delta(bs, m_next_state.m_time_state.last());
        }
        else // initial values
        {
            time_state.serializefrom_base(bs);
        }
        time_state.dump();
        m_next_state.m_time_state.push_back(time_state);
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
