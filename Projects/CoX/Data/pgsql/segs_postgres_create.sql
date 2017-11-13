CREATE TABLE "table_versions" (
	"id" serial NOT NULL,
	"table_name" varchar NOT NULL UNIQUE,
	"version" integer NOT NULL,
	"last_update" DATETIME NOT NULL DEFAULT 'CURRENT_TIMESTAMP',
	CONSTRAINT table_versions_pk PRIMARY KEY ("id")
) WITH (
  OIDS=FALSE
);



INSERT INTO "table_versions" (table_name)
VALUES ('table_versions'),
VALUES ('accounts'),
VALUES ('bans'),
VALUES ('game_servers');



CREATE TABLE "accounts" (
	"id" serial NOT NULL,
	"username" TEXT NOT NULL UNIQUE,
	"access_level" integer NOT NULL DEFAULT '1',
	"creation_date" DATE NOT NULL DEFAULT 'datetime('now','localtime')',
	"passw" bytea NOT NULL,
	CONSTRAINT accounts_pk PRIMARY KEY ("id")
) WITH (
  OIDS=FALSE
);



INSERT INTO `accounts` (username,passw)
VALUES (`segsadmin`,x`segs123`);



CREATE TABLE "bans" (
	"id" serial NOT NULL,
	"account_id" integer NOT NULL,
	"offending_ip" varchar(39) NOT NULL,
	"started" DATETIME NOT NULL DEFAULT 'CURRENT_TIMESTAMP',
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





ALTER TABLE "bans" ADD CONSTRAINT "bans_fk0" FOREIGN KEY ("account_id") REFERENCES "accounts"("id");


