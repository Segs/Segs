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
    ACE_ASSERT(!"Not implemented");
}

void InputState::partial_2(BitStream &bs)
{
    u8 control_id;
    //u16 v6;
    do
    {
        if(bs.GetBits(1))
            control_id=8;
        else
            control_id=bs.GetBits(4);
        if(bs.GetBits(1)) // time related , (control timestamp ?)
            /*v6=*/bs.GetBits(2);
        else
            /*v6=*/bs.GetBits(m_csc_deltabits);
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
    bool has_csc_mumble_listP=bs.GetBits(1);
    if(has_csc_mumble_listP)
    {
        m_csc_deltabits=bs.GetBits(5);
        /*u16 a3_bits=*/bs.GetBits(16);//ControlStateChange::field_8 or OptRel::field_19A8
        partial_2(bs);
    }
    for(int idx=0; idx<6; ++idx)
        bs.GetBits(1); //pak->SendBits(1, ((1 << idx) & (control_state.field_38)) != 0);
    if(bs.GetBits(1))//if ( abs(Z - v21) < 1000 )
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
            bs.GetPackedBits(1); //v7->field_0 - control_state.field_1A8[idx].field_0
            bs.GetPackedBits(1); // v7->get_time_resl - control_state.field_1A8[idx].field_4
            bs.GetFloat(); //v7->timestep
            if(bs.GetBits(1)) // if timestep!=field_1C
            {
                bs.GetFloat(); //v7->field_1C
            }
            m_perf_cntr = bs.Get64Bits(); //v7->perf_cntr1 - *((_QWORD *)v6 + 1)
            if(bs.GetBits(1)) // perf freq changed
            {
                m_perf_cntr2 = bs.Get64Bits(); //v7->perf_freq - *((_QWORD *)v6 + 2)
            }
        }
        else
        {
            bs.GetBits(32); //v7->field_0
            bs.GetBits(32); // v7->get_time_resl
            bs.GetFloat(); //v7->timestep
            if(bs.GetBits(1)) // if timestep==field_1C
            {
                bs.GetFloat(); //v7->field_1C
            }
            m_perf_cntr = bs.Get64Bits(); //v7->perf_cntr1
            m_perf_cntr2 = bs.Get64Bits(); //v7->perf_freq
        }
    }
    recv_client_opts(bs); // g_pak contents will follow
}
void InputState::recv_client_opts(BitStream &bs)
{
    int opt_idx=bs.GetPackedBits(1);
    while(opt_idx>0)
    {
        ACE_ASSERT(!"Don't know what options have what args!!");
        int num_options=0;
        for(int i=0; i<num_options; i++)
        {
            int option_arg_type=0;
            switch ( option_arg_type )
            {
            case 1:
            {
                int v=bs.GetPackedBits(1);
                break;
            }
            case 3:
            {
                float v=bs.GetFloat();
                break;
            }
            case 5:
            {
                bs.GetBits(14);
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
    }
}

