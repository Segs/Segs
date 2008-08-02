#pragma once
#include "CommonNetStructures.h"
class PosUpdate
{
public:
	Vector3 posvec;
	Quaternion quat;
	int a;
	int b;
};
class ClientOptions
{
public:
	bool mouse_invert;
	float mouselook_scalefactor;
	float degrees_for_turns;
};

class PowerPool_Info
{
public:
	int id[3];
};
class Power
{
public:
	int entry_type;
	int unkn1,unkn2;
	string sunkn1;
	string sunkn2;
	string sunkn3;
	void serializeto(BitStream &tgt) const;
	void serializefrom(BitStream &src)
	{
		entry_type = src.GetBits(4);
		switch(entry_type)
		{
		case 1:
			unkn1 = src.GetBits(32);
			unkn2 = src.GetBits(32);
			break;
		case 2:
			unkn1 = src.GetPackedBits(3);
			unkn2 = src.GetPackedBits(3);
			break;
		case 6:
		case 12:
			src.GetString(sunkn1);
			src.GetString(sunkn2);
			src.GetString(sunkn3);
			break;
		case 0:
			break;
		default:
			ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Unknown tray entry type %d\n"),entry_type));
		}
	}
	void Dump()
	{
		switch(entry_type)
		{
		case 1: case 2:
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("[(0x%x,0x%x)]"),unkn1,unkn2));
			break;
		case 6: case 12:
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("[(%s,%s,%s)]"),sunkn1.c_str(),sunkn2.c_str(),sunkn3.c_str()));
			break;
		case 0:
			break;
		default:
			ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Unknown tray entry type %d\n"),entry_type));
		}

	}
};
class PowerTray
{
public:
	int unkn0;
	int unkn1;
	Power m_powers[10];
	Power *getPower(size_t idx)
	{
		if(idx<10)
			return &m_powers[idx];
		return NULL;
	}
	int setPowers()
	{
		int res=0;
		for(int i=0; i<10; i++)
		{
			res += (m_powers[i].entry_type!=0);
		}
		return res;
	}
	void serializefrom(BitStream &src)
	{
		for(int power_slot=0; power_slot<10; power_slot++)
			m_powers[power_slot].serializefrom(src);
	}
	void serializeto(BitStream &tgt) const
	{
		for(int power_slot=0; power_slot<10; power_slot++)
			m_powers[power_slot].serializeto(tgt);
	}
	void Dump()
	{
		for(int power_slot=0; power_slot<10; power_slot++)
		{
			m_powers[power_slot].Dump();
		}

	}
};
class PowerTrayGroup
{
	static const int num_trays=2; // was 3, displayed trays
	PowerTray m_trays[9];
	u32 m_power_rel1,m_power_rel2;
	bool m_c;
	int unkn1,unkn2;
public:
	PowerTrayGroup()
	{
		m_power_rel1=m_power_rel2=0;
		m_c=false;
	}
	void serializeto(BitStream &tgt) const
	{
		for(int i=0; i<num_trays; i++)
			tgt.StoreBits(32,m_trays[i].unkn0);
		for(int bar_num=0; bar_num<9; bar_num++)
			m_trays[bar_num].serializeto(tgt);
		bool m_c = false;
		tgt.StoreBits(1,m_c);
		if(m_c)
		{
			tgt.StoreBits(32,m_power_rel1);
			tgt.StoreBits(32,m_power_rel2);
		}
	}
	void serializefrom(BitStream &src)
	{
		unkn1 = src.GetBits(32);
		unkn2 = src.GetBits(32);
		for(int bar_num=0; bar_num<9; bar_num++)
		{
			m_trays[bar_num].serializefrom(src);
		}
		m_c = src.GetBits(1);
		if(m_c)
		{
			m_power_rel1= src.GetBits(32);
			m_power_rel2= src.GetBits(32);
		}
	}
	void dump()
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    unkn1: 0x%08x\n"),unkn1));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    unkn2: 0x%08x\n"),unkn2));
		for(int bar_num=0; bar_num<9; bar_num++)
		{
			if(m_trays[bar_num].setPowers()==0)
				continue;
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    Tray %d ***"),bar_num));
			m_trays[bar_num].Dump();
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("***\n")));
		}
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_c %d\n"),m_c));
		if(m_c)
		{
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_power_rel1 0x%08x\n"),m_power_rel1));
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_power_rel2 0x%08x\n"),m_power_rel2));
		}
	}

};
class Entity;
class Avatar : public NetStructure //! not to be confuzzled with GameServer's Character :)
{
		bool m_full_options;
		ClientOptions m_options;
		bool m_first_person_view_toggle;
		u8 m_player_collisions;
		PowerPool_Info	get_power_info(BitStream &src);
public:
		Entity *m_ent;
		u32 field_5A8;
		u32 field_498;
		std::string char_arr_10_5AC; 
		vector<PowerPool_Info> m_powers;
		PowerTrayGroup m_trays;
		std::string m_class_name;
		std::string m_origin_name;
virtual			~Avatar()
				{
					m_ent = NULL; // we only borrowed this pointer	    
				}
				Avatar(Entity *m_ent);
virtual void	serializeto(BitStream &bs) const;
virtual void	serializefrom(BitStream &){ACE_ASSERT(false);};

		void	send_character(BitStream &bs)const;
		void	sendFullStats(BitStream &bs)const;
		void	sendBuffs(BitStream &bs)const;
		void	sendTray(BitStream &bs)const;
		void	sendTrayMode(BitStream &bs)const;
		void	sendEntStrings(BitStream &bs)const;
		void	sendWindow(BitStream &bs)const;
		void	sendTeamBuffMode(BitStream &bs)const;
		void	sendDockMode(BitStream &bs)const;
		void	sendChatSettings(BitStream &bs)const;
		void	sendTitles(BitStream &bs)const;
		void	sendDescription(BitStream &bs)const;
		void	sendKeybinds(BitStream &bs)const;
		void	sendOptions(BitStream &bs)const;
		void	sendFriendList(BitStream &bs)const;
		void	GetCharBuildInfo(BitStream &src);
		void	DumpPowerPoolInfo(const PowerPool_Info &pool_info);
		void	DumpBuildInfo();

};
class MapCostume : public Costume
{
	vector<string> stringcache;
	u32 stringcachecount_bitlength;
	vector<u32> colorcache;
	u32 colorcachecount_bitlength;
	bool costume_sends_nonquantized_floats;
	union
	{
		struct{
			float m_height;
			float m_physique;
		} split;
		float m_floats[30];
	};

	u8		m_costume_type;
	int		costume_type_idx_P;
	int		costume_sub_idx_P;

protected:
		void	GetCostume(BitStream &src);
		void	GetCostumeString(BitStream &src,string &tgt);
		u32		GetCostumeColor(BitStream &src);
		void	GetCostumeString_Cached(BitStream &src,string &tgt);
		u32		GetCostumeColor_Cached(BitStream &src);
		void	SendCostumeString_Cached(BitStream &tgt,const string &src) const;
		void	SendCostumeColor_Cached(BitStream &tgt,u32 color) const;
		void	SendCommon(BitStream &bs) const;
public:
				~MapCostume(){}
				MapCostume()
				{
					stringcachecount_bitlength=0;
					colorcachecount_bitlength=0;
				}
		void	clear_cache();
		void	serializefrom(BitStream &);
		void	serializeto(BitStream &bs) const;
		void	dump();
};
class Entity : public NetStructure
{
public:
		enum
		{
			ENT_PLAYER=2,
			ENT_CRITTER=4,
		};
		int field_64;
		int field_60;
		int field_68;
		int field_78;
		int m_num_titles;
		int m_num_fx;
		bool m_has_titles;
		vector<u8> m_fx1;
		vector<u32> m_fx2;
		vector<u8> m_fx3;
		u8		m_costume_type;
		int		m_state_mode;
		bool	m_state_mode_send;
		bool	m_odd_send;
		bool	m_SG_info;
		bool	m_seq_update;
		bool	m_is_villian;
		bool	m_contact;
		int m_seq_upd_num1;
		int m_seq_upd_num2;
		PosUpdate m_pos_updates[64];
		size_t m_update_idx;
		string m_battle_cry;
		string m_character_description;
		u32 field_E30;
		bool var_B4;
		
		Avatar m_char;
		MapCostume *m_costume;

		bool entReceiveAlwaysCon;
		bool entReceiveSeeThroughWalls;
		int pkt_id_QrotUpdateVal[3];
		Quaternion qrot;
		Vector3 pos;
		u32 prev_pos[3];
		bool m_selector1,m_pchar_things,var_C          ,m_hasname  ,m_hasgroup_name,m_classname_override;
		bool m_create   ,m_hasRagdoll  ,m_create_player,m_rare_bits;
		int current_client_packet_id;
		string m_name, m_group_name, m_override_name;
		u8 m_origin_idx,m_class_idx;
		u8 m_type;
		u32 m_idx;
		u32 var_8C;
		u32 var_90;
		bool player_type;
		bool m_player_villain;
		u32 var_1190;
		bool var_129C;

						Entity();
virtual					~Entity(){}
		void			dump();
		u32				getIdx() const {return m_idx;}
virtual void			serializeto(BitStream &bs)const;
		void			sendStateMode(BitStream &bs) const;
		void			sendOnOddSend(BitStream &bs,bool is_odd=true) const;
		void			sendSeqMoveUpdate(BitStream &bs) const;
		void			sendSeqTriggeredMoves(BitStream &bs) const;
		void			sendNetFx(BitStream &bs) const;
virtual	void			sendCostumes(BitStream &bs) const=0;
		void			sendXLuency(BitStream &bs,float xluency) const;
		void			sendTitles(BitStream &bs) const;
		void			sendRagDoll(BitStream &bs) const;
static	void			sendAllyID(BitStream &bs);
static	void			sendPvP(BitStream &bs);
		void			sendEntCollision(BitStream &bs) const;
		void			sendNoDrawOnClient(BitStream &bs)const; 
		void			sendContactOrPnpc(BitStream &bs)const; 
		void			sendPetName(BitStream &bs)const; 
		void			sendAFK(BitStream &bs)const; 
		void			sendOtherSupergroupInfo(BitStream &bs)const; 
		void			sendLogoutUpdate(BitStream &bs)const; 

		void			storePosition(BitStream &bs) const;	
		void			storeOrientation(BitStream &bs) const;
		void			storeUnknownBinTree(BitStream &bs) const;
		void			storePosUpdate(BitStream &bs) const;	
		int				getOrientation(BitStream &bs);
virtual void			serializefrom(BitStream &){};
		bool			update_rot(int axis) const; // returns true if given axis needs updating;

		void			serializefrom_newchar(BitStream &src);
		void			InsertUpdate(PosUpdate pup);
		void			sendCharacterStats(BitStream &bs) const;
		void			sendTargetUpdate(BitStream &bs) const;
		void			sendWhichSideOfTheForce(BitStream &bs) const;
		void			sendBuffs(BitStream &bs) const;
};
class MobEntity : public Entity
{
		std::string		m_costume_seq;
public:

virtual	void			sendCostumes(BitStream &bs) const;


};
class PlayerEntity : public MobEntity
{
		vector<Costume *>	m_costumes;
		Costume *			m_sg_costume;
		bool				m_current_costume_set;
		u32					m_current_costume_idx;
		u32					m_num_costumes;
		bool				m_multiple_costumes; // has more then 1 costume
		bool				m_supergroup_costume; // player has a sg costume
		bool				m_using_sg_costume; // player uses sg costume currently
public:

virtual	void				sendCostumes(BitStream &bs) const;


};
