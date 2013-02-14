/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

%module swig
%{
extern char *ADMIN_EMAIL;       // Holds the email address of the server administrator
extern char *SERVER_NAME;       // Holds the server name of our segs server
extern char *AUTH_HOST;         // Holds the authentication hostname or IP address to listen on
extern char *DATABASE_HOST;     // Holds the database hostname or IP that holds account and character data
extern char *DATABASE_NAME;     // Holds the name of the database where account and character data reside
extern char *DATABASE_USER;     // Holds the username to connect to the database server as
extern char *DATABASE_PASSWORD; // Holds the password to use in conjuction with DATABASE_USER
%}

extern char *ADMIN_EMAIL;       // Holds the email address of the server administrator
extern char *SERVER_NAME;       // Holds the server name of our segs server
extern char *AUTH_HOST;         // Holds the authentication hostname or IP address to listen on
extern char *DATABASE_HOST;     // Holds the database hostname or IP that holds account and character data
extern char *DATABASE_NAME;     // Holds the name of the database where account and character data reside
extern char *DATABASE_USER;     // Holds the username to connect to the database server as
extern char *DATABASE_PASSWORD; // Holds the password to use in conjuction with DATABASE_USER
