/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

#pragma once

class MapRef : public NetStructure
{
public:
	int m_idx;
	bool reduced_transform;
	Matrix4x3 m_matrix;
	TransformStruct m_transforms;
	string m_name;
	MapRef(int idx,Matrix4x3 &mat,string name) : m_idx(idx),m_matrix(mat),m_name(name){reduced_transform=false;}
	MapRef(int idx,string name,Vector3 &pos,Vector3 &rot) : m_idx(idx),m_name(name)
	{
		reduced_transform=true;
		m_transforms=TransformStruct(pos,rot,Vector3(),true,true,false);
	}
	MapRef(): m_idx(0),m_name(""){}
	void dump(void)
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    idx %d\n"),m_idx));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    name %s\n"),m_name.c_str()));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    |%f,%f,%f|\n"),m_matrix.row1.v));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    |%f,%f,%f|\n"),m_matrix.row2.v));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    |%f,%f,%f|\n"),m_matrix.row3.v));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    |%f,%f,%f|\n"),m_matrix.row4.v));
	}
	void serializefrom(BitStream &src)
	{
		m_idx = src.GetPackedBits(1);
		if(m_idx<0) return;
		src.GetString(m_name);
		getTransformMatrix(src,m_matrix);
	}
	void serializeto(BitStream &tgt) const
	{
		tgt.StorePackedBits(16,m_idx); //def_id
		//tgt.StoreString(m_name);
		if(reduced_transform)
		{
			storeTransformMatrix(tgt,m_transforms);
		}
		else
		{
			storeTransformMatrix(tgt,m_matrix);
		}

	}

};
