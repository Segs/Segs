/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: Client.h 253 2006-08-31 22:00:14Z malign $
 */

// Inclusion guards
#pragma once
#ifndef CHARACTER_H
#define CHARACTER_H

//* Another small step toward a real Character server.

#pragma once
#include "types.h"
#include "CommonNetStructures.h"
#include <string>
#include "BitStream.h"
#define MAX_CHARACTER_SLOTS 8

class CharacterCostume : public Costume
{
	u8  m_slot_index;
	u64 m_character_id; //! Character to whom this costume belongs
public:
	u8 getSlotIndex() const { return m_slot_index; }
	void setSlotIndex(u8 val) { m_slot_index = val; }
	u64 getCharacterId() const { return m_character_id; }
	void setCharacterId(u64 val) { m_character_id = val; }

	void serializeto(BitStream &tgt) const
	{
		tgt.StorePackedBits(1,(u32)m_parts.size());
		for(size_t i=0; i<m_parts.size(); i++)
		{
			//tgt.StoreBits(32,m_parts[i].m_type);
			tgt.StoreString(m_parts[i].name_0);
			tgt.StoreString(m_parts[i].name_1);
			tgt.StoreString(m_parts[i].name_2);
			tgt.StoreString(m_parts[i].name_3);
			tgt.StorePackedBits(32,m_parts[i].m_colors[0]);
			tgt.StorePackedBits(32,m_parts[i].m_colors[1]);
		}
	}
	void serializefrom(BitStream &src)
	{
		u32 num_parts;
		num_parts =  src.GetPackedBits(1);
		for(size_t i=0; i<num_parts; i++)
		{

		}
	}
	//////////////////////////////////////////////////////////////////////////
	// Database related
	//////////////////////////////////////////////////////////////////////////
	u64		m_id;
	u32		m_face_ints_vec[7];
	size_t	m_face_used_ints; // this value is forced to by <= sizeof(m_face_ints_vec)  by Database's getColIntArray return value; 
	float	m_floats_vec[8];
	size_t	m_used_floats;
};

class Character : public NetStructure
{
public:
						Character()
						{
							m_costume=0;
						}
//////////////////////////////////////////////////////////////////////////
// Getters and setters
		u32				getLevel() const { return m_level; }
		void			setLevel(u32 val) { m_level = val; }
const	std::string &	getName() const { return m_name; }
		void			setName(const std::string &val); 
const	std::string &	getArchetype() const { return m_archetype; }
		void			setArchetype(std::string val) { m_archetype = val; }
const	std::string &	getOrigin() const { return m_origin; }
		void			setOrigin(const std::string &val) { m_origin = val; }
		u32				getBodyType() const { return m_bodyType; }
		void			setBodyType(u32 val) { m_bodyType = val; }
		u32				getFace_bits() const { return m_face_bits; }
		void			setFace_bits(u32 val) { m_face_bits = val; }
const	std::string &	getMapName() const { return m_mapName; }
		void			setMapName(const std::string &val) { m_mapName = val; }
		u8				getIndex() const { return m_index; }
		void			setIndex(u8 val) { m_index = val; }
		u64				getAccountId() const { return m_owner_account_id; }
		void			setAccountId(u64 val) { m_owner_account_id = val; }
		u64				getLastCostumeId() const { return m_last_costume_id; }
		void			setLastCostumeId(u64 val) { m_last_costume_id = val; }
//
//////////////////////////////////////////////////////////////////////////
		void			reset();
		bool			isEmpty();
		bool			serializeFromDB(u64 user_id,u32 slot_index){return false;};
		void			serializefrom(BitStream &buffer);
		void			serializeto(BitStream &buffer) const;

		bool m_villain;
		f32 m_unkn1;
		f32 m_unkn2;
		u32 m_unkn3;
		u32 m_unkn4;
		CharacterCostume *m_costume;
protected:
		u64 m_owner_account_id;
		u64 m_last_costume_id;
		u8	m_index;
		u32 m_level;
		u32 m_bodyType;
		u32 m_face_bits;
		std::string m_name;
		std::string m_archetype;
		std::string m_origin;
		std::string m_mapName;
		typedef enum _CharBodyType
		{
			TYPE_MALE,
			TYPE_FEMALE,
			TYPE_UNUSED1,
			TYPE_UNUSED2,
			TYPE_HUGE,
			TYPE_NOARMS
		} CharBodyType, *pCharBodyType;

	/*
	static const char *charBodyTypeStrings[] = {
	"Male",
	"Female",
	"Unused1",
	"Unused2",
	"Huge",
	"No arms"
	};
	*/

};

#endif // CHARACTER_H
