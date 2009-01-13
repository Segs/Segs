/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: Database.cpp 301 2006-12-26 15:50:44Z nemerle $
 */

// This module contains code to access our database where accounts and characters are stored
#include <sstream>
#include <ace/Log_Msg.h>
#include <ace/OS.h>
// segs Includes
#include "Database.h"
#include <string.h>
// Our PostgreSQL connection information
Database::Database()
{
	pConnection=0;
};
void Database::setConnectionConfiguration(const char *host,const char *db,const char *user,const char *passw)
{
	std::stringstream res;
	res <<"host="<<host<<" dbname="<<db<<" user="<<user<<" password="<<passw;
	connect_string=res.str();

}
int Database::OpenConnection(void) // Opens connection to the database server
{
	pConnection = PQconnectdb(connect_string.c_str());     // Connect with our string defined above

	if (PQstatus(pConnection) != CONNECTION_OK)   // If the connection did not go well, let us know
	{
		ACE_ERROR((LM_ERROR, ACE_TEXT ("(%P|%t) Database: connection to %s failed. %s.\n"), PQhost(pConnection), PQerrorMessage(pConnection)));
		exit(1); // Exit SEGS to make the user corrects the problem
	}
	else
	{
		ACE_DEBUG ((LM_INFO,"Connected to %s database server version %i.\n",PQdb(pConnection), PQserverVersion(pConnection)));
	}
	return 0;
}

int Database::CloseConnection(void) // Closes connection to the database server
{
	PQfinish(pConnection);    // Close our connection to the PostgreSQL db server
	ACE_DEBUG((LM_INFO,"Connection to PostgreSQL database server closed.\n"));
	return 0;
}

bool Database::execQuery(const string &q,DbResults &res)
{
	if(res.m_result)
		PQclear(res.m_result);
	res.m_result = PQexec(pConnection,q.c_str());   // Send our query to the PostgreSQL db server to process
	if(!res.m_result)
		return false;
	res.m_msg = PQresultErrorMessage(res.m_result);

	if(res.m_msg&&res.m_msg[0])
		return false;
	return true;
}
bool Database::execQuery(const string &q)// for insert/update/delete queries, no 'real' result is returned
{
	PGresult *m_result = PQexec(pConnection,q.c_str());   // Send our query to the PostgreSQL db server to process
	if(!m_result)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Database: query %s failed. ERR_FATAL.\n"), q.c_str()),false);	
	char *err = PQresultErrorMessage(m_result);
	if(err&&err[0])
	{
		PQclear(m_result);
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Database: query %s failed. %s.\n"), q.c_str(),err),false);	
	}
	PQclear(m_result);
	return true;
}
DbResults::DbResults()
{
	m_result=0;
	m_msg = 0;
}
DbResults::~DbResults()
{
	PQclear(m_result);
}
DbResultRow DbResults::getRow(size_t row)
{
	if(row>=num_rows())
		return DbResultRow(); // uninitialized row is returned here
	return DbResultRow(m_result,row);
}



const char *DbResultRow::getColString(const char *column_name)
{
	if(!m_result)
		return NULL;
	int field_no = PQfnumber(m_result,column_name);
	if(field_no<0)
		return NULL;
	return PQgetvalue(m_result,m_row,field_no);
}
s16 DbResultRow::getColInt16(const char *column_name)
{
	return (s16)getColInt32(column_name);
}
bool DbResultRow::getColBool(const char *column_name)
{
	const char *res = getColString(column_name);
	if(!res)
	{
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Database: unknown column:%s.\n"), column_name),false);	
	}
	return toupper(res[0])=='T';
}
s32 DbResultRow::getColInt32(const char *column_name)
{
	const char *res = getColString(column_name);
	if(!res)
	{
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Database: unknown column:%s.\n"), column_name),-1);	
	}
	return ACE_OS::atoi(res);
}
s64 DbResultRow::getColInt64(const char *column_name)
{
	const char *res = getColString(column_name);
	if(!res)
	{
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Database: unknown column:%s.\n"), column_name),-1);	
	}
	return strtol(res,0,0);
}
float DbResultRow::getColFloat(const char *column_name)
{
	const char *res = getColString(column_name);
	if(!res)
	{
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Database: unknown column:%s.\n"), column_name),0); // 
	}
	
	return (float)atof(res);
}
size_t DbResultRow::getColIntArray(const char *column_name,u32 *arr,size_t arr_size)
{
	size_t idx=0;
	const char *res = getColString(column_name);
	if(!res)
	{
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Database: unknown column:%s.\n"), column_name),0);
	}
	while(res && idx<arr_size)
	{
		res++;
		arr[idx++]=strtol(res,0,0);
		res=strchr(res,',');
	}
	return idx;
}
size_t DbResultRow::getColFloatArray(const char *column_name,float *arr,size_t arr_size)
{
	size_t idx=0;
	const char *res = getColString(column_name);
	if(!res)
	{
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Database: unknown column:%s.\n"), column_name),0);
	}
	while(res && idx<arr_size)
	{
		res++;
		arr[idx++]=atof(res);
		res=strchr(res,',');
	}
	return idx;
}
