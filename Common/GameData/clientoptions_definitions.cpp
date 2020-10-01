#include "clientoptions_definitions.h"

#define CLIENT_OPT(type,var)\
    ClientOption {#var,{{type,&var}} }

void ClientOptions::init()
{
    m_opts = {
        CLIENT_OPT(ClientOption::t_int,control_debug),
        CLIENT_OPT(ClientOption::t_int,no_strafe),
        CLIENT_OPT(ClientOption::t_int,alwaysmobile),
        CLIENT_OPT(ClientOption::t_int,repredict),
        CLIENT_OPT(ClientOption::t_int,neterrorcorrection),
        CLIENT_OPT(ClientOption::t_float,speed_scale),
        CLIENT_OPT(ClientOption::t_int,svr_lag),
        CLIENT_OPT(ClientOption::t_int,svr_lag_vary),
        CLIENT_OPT(ClientOption::t_int,svr_pl),
        CLIENT_OPT(ClientOption::t_int,svr_oo_packets),
        CLIENT_OPT(ClientOption::t_int,client_pos_id),
        CLIENT_OPT(ClientOption::t_int,atest0),
        CLIENT_OPT(ClientOption::t_int,atest1),
        CLIENT_OPT(ClientOption::t_int,atest2),
        CLIENT_OPT(ClientOption::t_int,atest3),
        CLIENT_OPT(ClientOption::t_int,atest4),
        CLIENT_OPT(ClientOption::t_int,atest5),
        CLIENT_OPT(ClientOption::t_int,atest6),
        CLIENT_OPT(ClientOption::t_int,atest7),
        CLIENT_OPT(ClientOption::t_int,atest8),
        CLIENT_OPT(ClientOption::t_int,atest9),
        CLIENT_OPT(ClientOption::t_int,predict),
        CLIENT_OPT(ClientOption::t_int,notimeout),
        CLIENT_OPT(ClientOption::t_int,selected_ent_server_index),
    };
}
#undef ADD_OPT
