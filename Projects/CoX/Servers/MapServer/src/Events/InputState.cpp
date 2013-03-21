/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */
#define _USE_MATH_DEFINES
#include <cmath>
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

        if(bs.GetBits(1)) //
            time_since_prev=bs.GetBits(2)+32;
        else
            time_since_prev=bs.GetBits(m_csc_deltabits);
        switch(control_id)
        {
            case 0: case 1: case 2:
            case 3: case 4: case 5:
                // field_38 bits , control_id is the number of the bit
                fprintf(stderr,"%s  : %d - ",control_name[control_id],time_since_prev);
                v = bs.GetBits(1); // press release
                fprintf(stderr,"%d\n",v);
                break;
            case 6:
            case 7:
            {
                v = bs.GetBits(11);
                // v = (x+pi)*(2048/2pi)
                // x = (v*(pi/1024))-pi
                float recovered = (float(v)/2048.0f)*(2*M_PI) - M_PI;
                fprintf(stderr,"Pyr %f : %f \n",camera_pyr.x,camera_pyr.y);
                if(control_id==6) //TODO: use camera_pyr.v[] here ?
                    camera_pyr.x = recovered;
                else
                    camera_pyr.y = recovered;
                break;
            }
            case 8:
                v = bs.GetBits(1);
//                fprintf(stderr,"\tCTRL_8[%d] ",v);
                if ( m_send_deltas )
                {
                    m_t1=bs.GetPackedBits(8);
                    m_t2=bs.GetPackedBits(8);
                }
                else
                {
                    m_send_deltas = true;
                    m_t1=bs.GetBits(32);
                    m_t2=bs.GetPackedBits(10);
                }
//                fprintf(stderr,"[%d, ",t1);
//                fprintf(stderr,",%d] ",t2);
                if(bs.GetBits(1))
                {
                    v=bs.GetBits(8);
//                    fprintf(stderr,"CTRL_8C[%d] ",v);
//                    fprintf(stderr,"P2_8_opt : %d\n",v);
                }
                break;
            case 9:
                 //a2->timerel_18
                //fprintf(stderr,"CtrlId %d  : %d - ",control_id,time_since_prev);
                fprintf(stderr,"%d\n",bs.GetBits(8));
                break;
            case 10:
                fprintf(stderr,"CtrlId %d  : %d - ",control_id,time_since_prev);
                fprintf(stderr,"%d\n",bs.GetBits(1)); //a2->timerel_18 & 1
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
        m_csc_deltabits=bs.GetBits(5) + 1; // number of bits in max_time_diff_ms
        someOtherbits = bs.GetBits(16);//ControlStateChange::field_8 or OptRel::field_19A8
        current_state_P = 0;
#ifdef DEBUG_INPUT
        fprintf(stderr,"CSC_DELTA[%x-%x] : ",m_csc_deltabits,someOtherbits);
#endif
        partial_2(bs);

    }
    controlBits = 0;
    for(int idx=0; idx<6; ++idx)
        controlBits |= (bs.GetBits(1))<<idx;
#ifdef DEBUG_INPUT
    fprintf(stderr,"E input %x : ",controlBits);
#endif
    if(bs.GetBits(1))//if ( abs(s_prevTime - ms_time) < 1000 )
    {
        m_A_ang11_propably = bs.GetBits(11);//pak->SendBits(11, control_state.field_1C[0]);
        m_B_ang11_propably = bs.GetBits(11);//pak->SendBits(11, control_state.field_1C[1]);
#ifdef DEBUG_INPUT
        fprintf(stderr,"%x : %x",v1,v2);
#endif
    }
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
#ifdef DEBUG_INPUT
        fprintf(stderr,"CSC: %d,%d, [%f,%f]",field0,time_res,timestep,time_rel1C);
        fprintf(stderr, "(%lld %lld)",m_perf_cntr_diff,m_perf_freq_diff);
#endif
    }
};
void InputState::serializefrom(BitStream &bs)
{
    m_send_deltas=false;
#ifdef DEBUG_INPUT
    fprintf(stderr,"I:");
#endif
    if(bs.GetBits(1))
        extended_input(bs);

    bool has_targeted_entity = bs.GetBits(1);
    int tgt_idx=bs.GetPackedBits(14); // targeted entity server index
    int ctrl_idx=0;
#ifdef DEBUG_INPUT
    fprintf(stderr,"T:[%d]",has_targeted_entity);
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
    entry=opts.get(opt_idx);
    Vector3 vec;
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
                    std::string v;
                    bs.GetString(v);
                    break;
                }
                case ClientOption::t_vec3:
                {
                    for (int j = 0; j < 3; ++j )
                    {
                        vec.v[j] = bs.GetFloat();
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

