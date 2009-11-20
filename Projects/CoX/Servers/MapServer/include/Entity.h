#pragma once
#include "CommonNetStructures.h"
#include "Powers.h"
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
	bool costume_sends_nonquantized_floats;
	union
	{
		struct{
			float m_height;
			float m_physique;
		} split;
		float m_floats[30];
	};


protected:
		u8		m_costume_type;
		int		costume_type_idx_P;
		int		costume_sub_idx_P;
		void	GetCostume(BitStream &src);
		void	SendCommon(BitStream &bs) const;
public:
				~MapCostume(){}
				MapCostume()
				{
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
		bool m_selector1,m_pchar_things,might_have_rare          ,m_hasname  ,m_hasgroup_name,m_classname_override;
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
						MobEntity();
virtual					~MobEntity(){}
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
							PlayerEntity();
virtual						~PlayerEntity(){}
virtual	void				sendCostumes(BitStream &bs) const;
		void				serializefrom_newchar(BitStream &src);


};
