/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: SEGSMap.cpp $
 */

#include "SEGSMap.h"
#if 0
class XFILE
{
public:
	static XFILE Open(char *path,char *mode);
	int fread(char *tgt,size_t size,size_t count);
	int fseek(size_t ,size_t);
	int ftell();
};
struct PiggHandle_Sub1C
{
	size_t sub_5D1950()
	{
		// shared mem requirement
		return num_entries + 8*table_sizes + 28;
	}
	int pool_flag;
	int table_sizes;
	int num_entries;
	char *contents;
	int *table_1;
	int *table_2;
	int b[2];
};
struct PiggInternalHeader
{
	int flag;
	int name_id;
	int a[10];
};
struct PiggHeader
{
	int pigg_magic;
	short a;
	short pigg_version;
	short header_size;
	short used_header_bytes;
	int num_entries;
};
struct PiggHandle
{
	PiggHeader			header;
	int					used_header_bytes;
	int					num_headers;
	PiggInternalHeader *file_headers;
	PiggHandle_Sub1C	string_pool;
	PiggHandle_Sub1C	header_pool;
	XFILE *				file;
	char *				pigg_filename;
};
void sub_5D1670(PiggHandle_Sub1C *a1)
{
	memset((char *)a1, 0, 0x1Cu);
}
int __cdecl read_file_data_pigg(XFILE *fp, PiggHandle_Sub1C *buffer)
{
	int v3; // [sp+4Ch] [bp-4h]@1

	v3 = 0;
	if ( buffer->contents )
	{
		free(buffer->contents);
	}
	buffer->contents = 0;
	v3 += fp->fread((char*)&buffer->pool_flag, 1u, 4u);
	v3 += fp->fread((char*)&buffer->table_sizes, 1u, 4u);
	v3 += fp->fread((char*)&buffer->num_entries, 1u, 4u);
	buffer->contents = (char *)malloc(buffer->num_entries);
	v3 += fp->fread(buffer->contents, 1u, buffer->num_entries);
	return v3;
}

void __cdecl initPiggHandle(PiggHandle *pgghdr)
{
	memset(pgghdr, 0, 0x54u);
	sub_5D1670(&pgghdr->sub1);
	pgghdr->sub1.datapool_flag = 0x6789;
	sub_5D1670(&pgghdr->sub2);
	pgghdr->sub2.datapool_flag = 0x9ABC;
	pgghdr->field_4 = 2;
	pgghdr->pigg_version = 2;
	pgghdr->header_size = 16;
	pgghdr->used_header_bytes = 48;
	pgghdr->pigg_magic = 0x123;
}
extern int report_LoadingProblem(char *a1, int a2, char *a3, int a4);
int __cdecl split_into_tables(PiggHandle_Sub1C *a1)
{
	char *v3; // [sp+50h] [bp-8h]@5
	int v4; // [sp+4Ch] [bp-Ch]@10

	if ( !a1->table_1 )
		a1->table_1 = malloc(4 * a1->field_4);
	if ( !a1->table_2 )
		a1->table_2 = malloc(4 * a1->field_4);
	v3 = (int *)a1->contents;
	for(int idx=0; idx<a1->field_4; ++idx)
	{
		if ( (char *)v3 >= &a1->contents[a1->num_entries] || (char *)v3 < a1->contents )
			return -1;
		a1->table_2[idx] = *(v3++);
		a1->table_1[idx] = v3;
		(char *)v3+=a1->table_2[idx];
	}
	return 0;
}

int __cdecl sub_5D1970(XFILE *fh, PiggHandle_Sub1C *a2, char *a3)
{
	int v4; // eax@4
	__int32 offset; // [sp+5Ch] [bp-4h]@1
	int phandle; // [sp+54h] [bp-Ch]@1

	offset = 0;
	phandle = 0;
	if ( a2->contents )
			free(a2->contents);
	a2->contents = 0;
	return sub_5D1870(fh,a2);
}
signed int sub_5D1870(XFILE *a1, PiggHandle_Sub1C *a2)
{
	int offset;
	offset += read_file_data_pigg(a1, a2);
	if ( a2->table_1 )
		free(a2->table_1);
	a2->table_1 = 0;
	if ( a2->table_2 )
		free(a2->table_2);
	a2->table_2 = 0;
	if ( split_into_tables(a2) )
		return -1;
	return offset;
}

int __cdecl piglib_5CC7C0(PiggHandle *handle, char *filename)
{
	_DWORD v3; // [sp+64h] [bp-344h]@1
	int v4; // [sp+60h] [bp-348h]@1
	int a4; // [sp+5Ch] [bp-34Ch]@1
	signed int v6; // [sp+58h] [bp-350h]@1
	int v7; // [sp+Ch] [bp-39Ch]@4
	_BYTE str114[260]; // [sp+294h] [bp-114h]@11
	struct stat stat_strct[36]; // [sp+270h] [bp-138h]@19
	int num_headers; // [sp+3A4h] [bp-4h]@34
	int current_offset; // [sp+39Ch] [bp-Ch]@38
	__int32 offset; // [sp+50h] [bp-358h]@40
	int idx; // [sp+3A0h] [bp-8h]@40
	int v14; // [sp+398h] [bp-10h]@54
	__int32 v15; // [sp+54h] [bp-354h]@59
	char buf; // [sp+68h] [bp-340h]@59

	v3 = 0;
	v4 = 0;
	a4 = -1;
	v6 = 0;
	assert(!handle->file && "Cannot reuse table_sizes handle without calling PigFileDestroy() first!");
	strcpy(str114, filename);
	if ( !isFileReadablePPP(str114) )
		sprintf(str114, "%s.pig", filename);
	if ( !isFileReadablePPP(str114) )
		sprintf(str114, "%s.pigg", filename);
	if ( handle->pigg_filename )
	{
		free(handle->pigg_filename);
		handle->pigg_filename = 0;
	}
	handle->pigg_filename = strdup(str114);
	handle->file = XFILE::Open(str114, "rb~");
	handle->file->fread(&handle->header, 1u, 0x10u);
	assert(sizeof(handle->header)==16 && "Changed size of PigHeader but didn't update reading code to read the extra values");
	if ( handle->header.pigg_magic != 0x123 )
		return report_LoadingProblem(filename, 2, "Bad flag on file - not table_sizes Pig file", a4);
	if ( handle->header.pigg_version > 2 )
		return report_LoadingProblem(filename, 3, "Pig file too new of table_sizes version ", handle->pigg_version);
	if ( handle->header.header_size != 16 )
		handle->file->fseek(, handle->header.header_size - 16, 1);
	num_headers = handle->header.num_entries;
	assert(!handle->file_headers);
	handle->file_headers = (PiggInternalHeader *)malloc(sizeof(PiggInternalHeader) * num_headers);
	current_offset = 0;
	if ( handle->used_header_bytes == 48 )
	{
		current_offset += handle->file->fread(handle->file_headers, 1u, 48 * num_headers);
	}
	else
	{
		offset = handle->header.used_header_bytes - 48;
		for(idx=0; idx<num_headers; ++idx)
		{
			current_offset += handle->file->fread(&handle->file_headers[idx], 1u, 0x30u);
			handle->file->fseek(offset, 1);
			if ( offset < 0 )
				memset((char *)&handle->file_headers[idx]+handle->header.used_header_bytes,0,-offset);
		}
	}
	if ( current_offset != 48 * num_headers )
		return report_LoadingProblem(filename, 9, "Couldn't read all of the required data from table_sizes Pig file", a4);
	for(idx = 0; idx < (unsigned int)num_headers; idx++)
	{
		if ( handle->file_headers[idx].flag != 0x3456 )
			return report_LoadingProblem(filename, 4, "Bad flag on fileheader #", idx);
		v7 = handle->file_headers[idx].field_8;
		if ( handle->file_headers[idx].field_2C )
			v7 = handle->file_headers[idx].field_2C;
			
		size_t required_size = handle->file_headers[idx].field_10 + v7 - 1;
		if ( required_size > stat_strct->st_size )
			return report_LoadingProblem(filename,10,"Fileheader extends past end of .pigg, probably corrupt .pigg file!  #",idx);
		if ( handle->file_headers[idx].name_id < 0 )
			return report_LoadingProblem(filename, 10, "Bad name id", idx);
	}
	v15 = handle->file->ftell();
	sprintf(&buf, "PIG_SP_%s", str114);
	if ( sub_5D1970(handle->file, &handle->_stringpool, &buf) == -1 )
		return report_LoadingProblem(filename, 5, "string Datapool error", a4);
	sprintf(&buf, "PIG_DP_%s", str114);
	if ( sub_5D1970(handle->file, &handle->_headerpool, &buf) == -1 )
		return report_LoadingProblem(filename, 6, "header Datapool error", a4);
	if ( handle->_stringpool.table_sizes == handle->header.num_entries )
	{
		if ( handle->_stringpool.datapool_flag != 0x6789 )
			return report_LoadingProblem(filename, 7, "Bad flag on String Pool", a4);
		if ( handle->_headerpool.datapool_flag != 0x9ABC )
			return report_LoadingProblem(filename, 8, "Bad flag on Header Data Pool", a4);
	}
	handle->file->fseek(v15, 0);
	if ( sub_5D1870(handle->file, &handle->_stringpool) == -1 )
		return report_LoadingProblem(filename, 5, "string Datapool error", a4);
	if ( sub_5D1870(handle->file, &handle->_headerpool) -= -1 )
		return report_LoadingProblem(filename, 6, "header Datapool error", a4);
	if ( handle->_stringpool.datapool_flag != 0x6789 )
		return report_LoadingProblem(filename, 7, "Bad flag on String Pool", a4);
	if ( handle->_headerpool.datapool_flag != 0x9ABC )
		return report_LoadingProblem(filename, 8, "Bad flag on Header Data Pool", a4);
	else
		
		
	return 0;
}
#endif
SEGSMap::SEGSMap( const string &name ) :m_name(name)
{

}
#if 0
int __cdecl sub_5BE550(ParserRelatedStruct *str_to_parse, CharAttr_Nested1 *valid_tokens, int a3, signed int (__cdecl *tok_func)(ParserRelatedStruct *))
{
	char *v5; // eax@8
	char *v6; // eax@8
	char *v7; // eax@15
	CharAttr_Nested1 *v8; // edx@17
	char *v9; // eax@31
	CharAttr_Nested1 *v10; // edx@48
	int v11; // [sp+60h] [bp-4h]@1
	signed int v12; // [sp+50h] [bp-14h]@1
	int v13; // [sp+58h] [bp-Ch]@1
	LPVOID *v14; // [sp+4Ch] [bp-18h]@6
	char *v15; // [sp+5Ch] [bp-8h]@15
	signed int v16; // [sp+54h] [bp-10h]@31

	v11 = 1;
	v12 = 0;
	for(v13=0; valid_tokens[v13].name && *valid_tokens[v13].name || valid_tokens[v13].type; ++v13)
	{
		if ( valid_tokens[v13].type != 3 )
			continue;
		v14 = (LPVOID *)(valid_tokens[v13].offset + a3);
		if ( *v14 )
			ParserAllocString(*v14);
		*v14 = (LPVOID)ParserStrDup(GetSourceName(str_to_parse));
		v6 = _strupr((char *)*v14);
		fix_path_slashes(v6);
	}
	for(v13=0; valid_tokens[v13].name && *valid_tokens[v13].name || valid_tokens[v13].type; ++v13)
	{
		if (( valid_tokens[v13].type & 0x200 )==0)
			continue;
		v7 = next_Token_P(str_to_parse, 0, 1);
		v15 = v7;
		if ( v7 )
		{
			if ( *v7 != 0xa )
			{
				v8 = &valid_tokens[v13];
				if ( sub_5BD7F0(str_to_parse, v8->name, v8->type, v8->offset, v8->param, v8->ptr_sub, a3, tok_func) )
					v12 = 1;
			}
		}
	}
	for(v13=0; valid_tokens[v13].name && *valid_tokens[v13].name || valid_tokens[v13].type; ++v13)
	{
		if ( valid_tokens[v13].type != 2 )
			continue;
		if ( *valid_tokens[v13].name == '\n' )
		{
			parsing_err = 0;
			if ( parsing_err )
				v11 = 0;
			v12 = 1;
		}
	}
	while ( !v12 )
	{
		parsing_err = 0;
		v16 = 0;
		v9 = next_Token_P(str_to_parse, 1, 1);
		v15 = v9;
		if ( !v9 )
			break;
		for(v13=0; valid_tokens[v13].name && *valid_tokens[v13].name || valid_tokens[v13].type; ++v13)
		{
			if ( !__ascii_stricmp(v15, valid_tokens[v13].name) )
			{
				v16 = 1;
				break;
			}
		}
		if ( parsing_err )
			v11 = 0;
		if ( v16 )
		{
			sub_5BBE10(str_to_parse, 1, 1);
			v10 = &valid_tokens[v13];
			if ( sub_5BD7F0(str_to_parse, v10->name, v10->type, v10->offset, v10->param, v10->ptr_sub, a3, tok_func) )
				v12 = 1;
			if ( !v12 )
				sub_5BE910(str_to_parse);
		}
		else
		{
			if ( tok_func )
			{
				if ( !((int (__cdecl *)(ParserRelatedStruct *, char *, int))tok_func)(str_to_parse, v15, a3) )
					v12 = 1;
			}
			else
			{
				v12 = 1;
			}
		}
		if ( parsing_err )
			v11 = 0;
	}
	if ( !v11 )
		parsing_err = 1;
	return v11;
}
#endif