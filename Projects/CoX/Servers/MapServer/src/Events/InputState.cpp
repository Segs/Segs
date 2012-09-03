/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */
#include "Events/InputState.h"
#include "Entity.h"

void InputState::serializeto(BitStream &) const
{
    assert(!"Not implemented");
}

void InputState::partial_2(BitStream &bs)
{
    uint8_t control_id;
    //uint16_t v6;
    uint16_t time_since_prev;
    do
    {
        if(bs.GetBits(1))
            control_id=8;
        else
            control_id=bs.GetBits(4);
        if(bs.GetBits(1)) //
            time_since_prev=bs.GetBits(2)+32;
        else
            time_since_prev=bs.GetBits(m_csc_deltabits);
        switch(control_id)
        {
        case 6:
        case 7:
            bs.GetBits(11);// a2->timerel_18 & 0x7FF
            break;
        case 8:
            bs.GetBits(1);
            if ( m_send_deltas )
            {
                bs.GetPackedBits(8);  //a2->timerel_18 - X
                bs.GetPackedBits(8);  //a2->timerel_1C - Y;
            }
            else
            {
                m_send_deltas = true;
                bs.GetBits(32);       //pak->SendBits(32, a2->timerel_18);
                bs.GetPackedBits(10); //pak->SendPackedBits(10, a2->timerel_18 - a2->timerel_1C);
            }
            if(bs.GetBits(1))
            {
                bs.GetBits(8);
            }
            break;
        case 9:
            bs.GetBits(8);  //a2->timerel_18
            break;
        case 10:
            bs.GetBits(1);  //a2->timerel_18 & 1
            break;
        default:
            bs.GetBits(1);  //a2->timerel_18 & 1, will also set/clr control_state.field_38 control_id'th bit
        }

    } while(bs.GetBits(1));
}

void InputState::partial(BitStream &bs)
{
    if(bs.GetBits(1)==0)
        return;
    // has extended control info.
    bool has_csc_mumble_listP=bs.GetBits(1);
    if(has_csc_mumble_listP)
    {
        m_csc_deltabits=bs.GetBits(5); // number of bits in max_time_diff_ms
        /*uint16_t a3_bits=*/bs.GetBits(16);//ControlStateChange::field_8 or OptRel::field_19A8
        partial_2(bs);
    }
    controlBits = 0;
    for(int idx=0; idx<6; ++idx)
        controlBits |= (bs.GetBits(1))<<idx;
    if(bs.GetBits(1))//if ( abs(s_prevTime - ms_time) < 1000 )
    {
        bs.GetBits(11);//pak->SendBits(11, control_state.field_1C[0]);
        bs.GetBits(11);//pak->SendBits(11, control_state.field_1C[1]);
    }
}
void InputState::serializefrom(BitStream &bs)
{
    m_send_deltas=false;
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("Input state received\n")));

    partial(bs);

    bool has_targeted_entity = bs.GetBits(1);
    int tgt_idx=bs.GetPackedBits(14);
    int ctrl_idx=0;
    while(bs.GetBits(1)) // receive control states ?
    {
        if(ctrl_idx)
        {
            bs.GetPackedBits(1); // field_0 diff next-current
            bs.GetPackedBits(1); // time to next state ?
            bs.GetFloat(); // next state's timestep
            if(bs.GetBits(1))
            {
                //timestep!=field_1C
                bs.GetFloat(); //v7->field_1C
            }
            m_perf_cntr_diff = bs.Get64Bits(); //next_state->ticks - current_state->ticks
            if(bs.GetBits(1))
            {
                // perf freq changed between current and next
                m_perf_freq_diff = bs.Get64Bits();
            }
        }
        else
        {
            bs.GetBits(32); //field_0
            bs.GetBits(32); // get_time_resl
            bs.GetFloat(); //v7->timestep
            if(bs.GetBits(1)) // if timestep==field_1C
            {
                bs.GetFloat(); //v7->field_1C
            }
            m_perf_cntr_diff = bs.Get64Bits(); //v7->perf_cntr1
            m_perf_freq_diff = bs.Get64Bits(); //v7->perf_freq
        }
    }
    recv_client_opts(bs); // g_pak contents will follow
}
void InputState::recv_client_opts(BitStream &bs)
{
    ClientOptions opts;
    ClientOption *entry;
    int opt_idx=bs.GetPackedBits(1);
    entry=opts.get(opt_idx);
    while(entry!=0)
    {
        for(size_t i=0; i<entry->m_args.size(); i++)
        {
            ClientOption::Arg &arg=entry->m_args[i];
            switch ( arg.type )
            {
            case 1:
            {
                *((int32_t *)arg.tgt) = bs.GetPackedBits(1);
                break;
            }
            case 3:
            {
                *((float *)arg.tgt)=bs.GetFloat();
                break;
            }
            case 5:
            {
                bs.GetBits(14); //quantized angle
                break;
            }
            case 2:
            case 4:
            {
                std::string v;
                bs.GetString(v);
                break;
            }
            case 7:
            {
                for (int j = 0; j < 3; ++j )
                {
                    float v=bs.GetFloat();
                }
                break;
            }
            default:
                continue;
            }
        }
        opt_idx=bs.GetPackedBits(1);
        entry=opts.get(opt_idx);
    }
}

