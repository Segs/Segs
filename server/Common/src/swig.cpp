/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: swig.cpp 251 2006-08-31 21:49:27Z malign $
 */

// Variables to hold our configuration values
char *ADMIN_EMAIL;        // Holds the email address of the server administrator
char *SERVER_NAME;        // Holds the server name of our segs server
char *AUTH_HOST;          // Holds the authentication hostname or IP address to listen on
char *DATABASE_HOST;      // Holds the database hostname or IP that holds account and character data
char *DATABASE_NAME;      // Holds the name of the database where account and character data reside
char *DATABASE_USER;      // Holds the username to connect to the database server as
char *DATABASE_PASSWORD;  // Holds the password to use in conjuction with DATABASE_USER
