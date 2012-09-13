/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */
#include "types.h"
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
    int v;
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
        if(control_id!=8)
            fprintf(stderr,"CtrlId %d  : %d\n",control_id,time_since_prev);
        switch(control_id)
        {
            case 0: case 1: case 2:
            case 3: case 4: case 5:
                // field_38 bits , control_id is the number of the bit
                v = bs.GetBits(1);
//                fprintf(stderr,"P2_%d : %d\n",control_id,v);
                break;
            case 6:
            case 7:
                v = bs.GetBits(11);
//                fprintf(stderr,"P2_timerel_18 : %d\n",v);
                break;
            case 8:
                v = bs.GetBits(1);
//                fprintf(stderr,"P2_8_1 : %d\n",v);
                if ( m_send_deltas )
                {
//                    bs.GetPackedBits(8);  //a2->timerel_18 - X
//                    bs.GetPackedBits(8);  //a2->timerel_1C - Y;
                    v=bs.GetPackedBits(8);
//                    fprintf(stderr,"P2_8_32_8 : %d\n",v);
                    v=bs.GetPackedBits(8);
//                    fprintf(stderr,"P2_8_10_8 : %d\n",v);
                }
                else
                {
                    m_send_deltas = true;
                    v=bs.GetBits(32);
//                    fprintf(stderr,"P2_8_32 : %d\n",v); //pak->SendBits(32, a2->timerel_18);
                    v=bs.GetPackedBits(10);
//                    fprintf(stderr,"P2_8_10 : %d\n",v);
                    ; //pak->SendPackedBits(10, a2->timerel_18 - a2->timerel_1C);
                }
                if(bs.GetBits(1))
                {
                    v=bs.GetBits(8);
//                    fprintf(stderr,"P2_8_opt : %d\n",v);
                }
                break;
            case 9:
                 //a2->timerel_18
                fprintf(stderr,"P2_9 : %d\n",bs.GetBits(8));
                break;
            case 10:
                fprintf(stderr,"P2_10 : %d\n",bs.GetBits(1)); //a2->timerel_18 & 1
                break;
            default:
                assert(!"Unknown control_id");
        }

    } while(bs.GetBits(1));
}

void InputState::extended_input(BitStream &bs)
{
    if(bs.GetBits(1)) // list of partial_2 follows
    {
        m_csc_deltabits=bs.GetBits(5); // number of bits in max_time_diff_ms
        /*uint16_t a3_bits=*/bs.GetBits(16);//ControlStateChange::field_8 or OptRel::field_19A8
        current_state_P = 0;
        partial_2(bs);

    }
    controlBits = 0;
    for(int idx=0; idx<6; ++idx)
        controlBits |= (bs.GetBits(1))<<idx;
    fprintf(stderr,"E input %x : ",controlBits);
    if(bs.GetBits(1))//if ( abs(s_prevTime - ms_time) < 1000 )
    {
        int v;
        v = bs.GetBits(11);//pak->SendBits(11, control_state.field_1C[0]);
        fprintf(stderr,"%x : ",v);
        v =bs.GetBits(11);//pak->SendBits(11, control_state.field_1C[1]);
        fprintf(stderr,"%x : ",v);
    }
    fprintf(stderr,"\n");
}
struct ControlState
{
    int field0;
    int time_res;
    float timestep;
    float time_rel1C;
    uint64_t m_perf_cntr_diff;
    uint64_t m_perf_freq_diff;
    // recover actual ControlState from network data and previous entry
    void serializefrom_delta(BitStream &bs,const ControlState &prev)
    {
        field0   = bs.GetPackedBits(1); // field_0 diff next-current
        time_res = bs.GetPackedBits(1); // time to next state ?
        timestep = bs.GetFloat(); // next state's timestep

        time_rel1C = timestep;
        if(bs.GetBits(1)) //timestep!=time_rel1C
            time_rel1C = bs.GetFloat();

        m_perf_cntr_diff = bs.Get64Bits(); //next_state->ticks - current_state->ticks
        if(bs.GetBits(1))
        {
            // perf freq changed between current and next
            m_perf_freq_diff = bs.Get64Bits();
        }
    }
    void serializefrom_base(BitStream &bs)
    {
        field0   = bs.GetBits(32); //field_0
        time_res = bs.GetBits(32); // get_time_resl
        timestep = bs.GetFloat(); //v7->timestep

        time_rel1C = timestep;
        if(bs.GetBits(1)) //timestep!=time_rel1C
            time_rel1C = bs.GetFloat();

        m_perf_cntr_diff = bs.Get64Bits(); //next_state->ticks - current_state->ticks
        m_perf_cntr_diff = bs.Get64Bits(); //v7->perf_cntr1
    }
    void dump()
    {
        fprintf(stderr,"%d,%d, [%f,%f]\n",field0,time_res,timestep,time_rel1C);
    }
};
void InputState::serializefrom(BitStream &bs)
{
    m_send_deltas=false;
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("Input state received\n")));
    if(bs.GetBits(1))
        extended_input(bs);

    bool has_targeted_entity = bs.GetBits(1);
    int tgt_idx=bs.GetPackedBits(14); // targeted entity server index
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
}
//TODO: use generic ReadableStructures here ?
void InputState::recv_client_opts(BitStream &bs)
{
    ClientOptions opts;
    ClientOption *entry;
    int opt_idx=0;
    int some_idx = bs.GetPackedBits(1);
    entry=opts.get(opt_idx);
    while(some_idx!=0)
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
                    printf("Quant:%d\n",bs.GetBits(14)); //quantized angle
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
        some_idx = bs.GetPackedBits(1);
        opt_idx++;
        entry=opts.get(opt_idx);
    }
}

