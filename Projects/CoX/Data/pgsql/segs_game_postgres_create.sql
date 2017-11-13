CREATE TABLE "table_versions" (
	"id" serial NOT NULL,
	"table_name" varchar NOT NULL UNIQUE,
	"version" integer NOT NULL DEFAULT '0',
	"last_update" DATETIME NOT NULL DEFAULT 'CURRENT_TIMESTAMP',
	CONSTRAINT table_versions_pk PRIMARY KEY ("id")
) WITH (
  OIDS=FALSE
);


INSERT INTO "table_versions" (table_name)
VALUES ('table_versions'),
VALUES ('accounts'),
VALUES ('costumes'),
VALUES ('characters'),
VALUES ('options'),
VALUES ('gui'),
VALUES ('supergroups'),
VALUES ('progress');


CREATE TABLE "accounts" (
	"id" serial NOT NULL,
	"account_id" integer NOT NULL,
	"max_slots" integer NOT NULL DEFAULT '8',
	CONSTRAINT accounts_pk PRIMARY KEY ("id")
) WITH (
  OIDS=FALSE
);


INSERT INTO "accounts" (account_id) VALUES (1);


CREATE TABLE "costumes" (
	"id" serial NOT NULL,
	"character_id" integer NOT NULL,
	"costume_index" integer NOT NULL,
	"skin_color" integer NOT NULL,
	"parts" bytea NOT NULL,
	CONSTRAINT costumes_pk PRIMARY KEY ("id")
) WITH (
  OIDS=FALSE
);



CREATE TABLE "characters" (
	"id" serial NOT NULL,
	"char_level" integer NOT NULL DEFAULT '1',
	"slot_index" integer NOT NULL DEFAULT '0',
	"account_id" integer NOT NULL DEFAULT '0',
	"char_name" varchar(20) NOT NULL,
	"archetype" varchar(32) NOT NULL,
	"origin" varchar(32) NOT NULL,
	"description" varchar(1024) NOT NULL,
	"battlecry" varchar(32) NOT NULL,
	"current_map" integer NOT NULL,
	"body_type" integer NOT NULL,
	"last_costume_id" integer NOT NULL DEFAULT '0',
	"last_online" DATETIME NOT NULL DEFAULT 'CURRENT_TIMESTAMP',
	"hitpoints" integer NOT NULL DEFAULT '0',
	"endurance" integer NOT NULL DEFAULT '0',
	"inf" integer NOT NULL DEFAULT '0',
	"xp" integer NOT NULL DEFAULT '0',
	"xpdebt" integer NOT NULL DEFAULT '0',
	"xppatrol" integer NOT NULL DEFAULT '0',
	"alignment" varchar NOT NULL DEFAULT 'hero',
	"posx" integer NOT NULL,
	"posy" integer NOT NULL,
	"posz" integer NOT NULL,
	"orientp" integer NOT NULL DEFAULT '0',
	"orienty" integer NOT NULL,
	"orientr" integer NOT NULL DEFAULT '0',
	"title" varchar NOT NULL DEFAULT '32',
	"badgetitle" varchar NOT NULL DEFAULT '32',
	"specialtitle" varchar NOT NULL DEFAULT '32',
	"supergroup_id" integer NOT NULL,
	CONSTRAINT characters_pk PRIMARY KEY ("id")
) WITH (
  OIDS=FALSE
);



CREATE TABLE "options" (
	"id" serial NOT NULL,
	"character_id" integer NOT NULL,
	"option" varchar(32) NOT NULL,
	"value" integer NOT NULL,
	CONSTRAINT options_pk PRIMARY KEY ("id")
) WITH (
  OIDS=FALSE
);



CREATE TABLE "gui" (
	"id" serial NOT NULL,
	"character_id" integer NOT NULL,
	"window" integer NOT NULL,
	"x" integer NOT NULL,
	"y" integer NOT NULL,
	"sx" integer NOT NULL,
	"sy" integer NOT NULL,
	"scale" integer NOT NULL DEFAULT '1',
	"color1" integer NOT NULL,
	"color2" integer NOT NULL,
	"frozen" integer NOT NULL DEFAULT '0',
	"state" integer NOT NULL DEFAULT '4',
	"resizable" integer NOT NULL DEFAULT '0',
	"flags" integer NOT NULL DEFAULT '0',
	CONSTRAINT gui_pk PRIMARY KEY ("id")
) WITH (
  OIDS=FALSE
);



CREATE TABLE "supergroups" (
	"id" serial NOT NULL,
	"supergroup_id" serial NOT NULL,
	"motto" varchar(128) NOT NULL,
	"rank_names" bytea NOT NULL,
	"rank_permissions" bytea NOT NULL,
	"motd" varchar(1024) NOT NULL,
	"emblem" bytea NOT NULL,
	"colors" bytea NOT NULL,
	"members" bytea NOT NULL,
	CONSTRAINT supergroups_pk PRIMARY KEY ("id")
) WITH (
  OIDS=FALSE
);



CREATE TABLE "progress" (
	"id" serial NOT NULL,
	"character_id" integer NOT NULL,
	"badges" bytea NOT NULL,
	"clues" bytea NOT NULL,
	"contacts" bytea NOT NULL,
	"souvenirs" bytea NOT NULL,
	CONSTRAINT progress_pk PRIMARY KEY ("id")
) WITH (
  OIDS=FALSE
);





ALTER TABLE "costumes" ADD CONSTRAINT "costumes_fk0" FOREIGN KEY ("character_id") REFERENCES "characters"("id");

ALTER TABLE "characters" ADD CONSTRAINT "characters_fk0" FOREIGN KEY ("account_id") REFERENCES "accounts"("id");
ALTER TABLE "characters" ADD CONSTRAINT "characters_fk1" FOREIGN KEY ("supergroup_id") REFERENCES "supergroups"("id");

ALTER TABLE "options" ADD CONSTRAINT "options_fk0" FOREIGN KEY ("character_id") REFERENCES "characters"("id");

ALTER TABLE "gui" ADD CONSTRAINT "gui_fk0" FOREIGN KEY ("character_id") REFERENCES "characters"("id");


ALTER TABLE "progress" ADD CONSTRAINT "progress_fk0" FOREIGN KEY ("character_id") REFERENCES "characters"("id");

