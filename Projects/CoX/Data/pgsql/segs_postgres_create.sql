DROP TABLE IF EXISTS "table_versions";
DROP TABLE IF EXISTS "game_servers";
DROP TABLE IF EXISTS "bans";
DROP TABLE IF EXISTS "accounts";

CREATE TABLE "accounts" (
    "id" serial NOT NULL,
    "username" TEXT NOT NULL UNIQUE,
    "access_level" integer NOT NULL DEFAULT '1',
    "creation_date" DATE NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "passw" bytea NOT NULL,
    "salt" bytea NOT NULL,
    CONSTRAINT accounts_pk PRIMARY KEY ("id")
) WITH (
    OIDS=FALSE
);

CREATE TABLE "bans" (
    "id" serial NOT NULL,
    "account_id" integer NOT NULL,
    "offending_ip" varchar(39) NOT NULL,
    "started" DATE NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "reason" varchar(1024) NOT NULL,
    CONSTRAINT bans_pk PRIMARY KEY ("id")
) WITH (
    OIDS=FALSE
);

CREATE TABLE "game_servers" (
    "id" serial NOT NULL,
    "addr" varchar(39) NOT NULL,
    "port" integer NOT NULL,
    "name" varchar(100) NOT NULL,
    "token" integer NOT NULL,
    CONSTRAINT game_servers_pk PRIMARY KEY ("id")
) WITH (
    OIDS=FALSE
);

CREATE TABLE "table_versions" (
    "id" serial NOT NULL,
    "table_name" varchar NOT NULL UNIQUE,
    "version" integer NOT NULL,
    "last_update" DATE NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT table_versions_pk PRIMARY KEY ("id")
) WITH (
    OIDS=FALSE
);

INSERT INTO table_versions VALUES(1,'db_version',1,'2019-02-17 03:11:58');
INSERT INTO table_versions VALUES(2,'table_versions',0,'2017-11-11 08:55:54');
INSERT INTO table_versions VALUES(3,'accounts',1,'2018-01-06 11:18:01');
INSERT INTO table_versions VALUES(4,'game_servers',0,'2017-11-11 09:12:37');
INSERT INTO table_versions VALUES(5,'bans',0,'2017-11-11 09:12:37');

ALTER TABLE "bans" ADD CONSTRAINT "bans_fk0" FOREIGN KEY ("account_id") REFERENCES "accounts"("id");
