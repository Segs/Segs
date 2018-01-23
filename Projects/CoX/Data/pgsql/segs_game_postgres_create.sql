CREATE TABLE "table_versions" (
	"id" serial NOT NULL,
	"table_name" varchar NOT NULL UNIQUE,
	"version" integer NOT NULL DEFAULT '0',
	"last_update" DATE NOT NULL DEFAULT CURRENT_TIMESTAMP,
	CONSTRAINT table_versions_pk PRIMARY KEY ("id")
) WITH (
  OIDS=FALSE
);

INSERT INTO table_versions VALUES(1,'db_version',1,'2018-01-23 10:27:01');
INSERT INTO table_versions VALUES(2,'table_versions',0,'2017-11-11 08:57:42');
INSERT INTO table_versions VALUES(3,'accounts',0,'2017-11-11 08:57:43');
INSERT INTO table_versions VALUES(4,'characters',2,'2018-01-23 10:16:27');
INSERT INTO table_versions VALUES(5,'costume',0,'2017-11-11 08:57:43');
INSERT INTO table_versions VALUES(6,'progress',0,'2017-11-11 08:57:43');
INSERT INTO table_versions VALUES(7,'supergroups',0,'2018-01-23 10:16:43');


CREATE TABLE "accounts" (
	"id" serial NOT NULL,
	"account_id" integer NOT NULL,
	"max_slots" integer NOT NULL DEFAULT '8',
	CONSTRAINT accounts_pk PRIMARY KEY ("id")
) WITH (
  OIDS=FALSE
);


INSERT INTO accounts VALUES(1,1,8);


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
	"chardata" bytea NOT NULL,
	"body_type" integer NOT NULL,
	"last_online" DATE NOT NULL DEFAULT CURRENT_TIMESTAMP,
	"hitpoints" integer NOT NULL DEFAULT '0',
	"endurance" integer NOT NULL DEFAULT '0',
	"posx" integer NOT NULL,
	"posy" integer NOT NULL,
	"posz" integer NOT NULL,
	"orientp" integer NOT NULL DEFAULT '0',
	"orienty" integer NOT NULL,
	"orientr" integer NOT NULL DEFAULT '0',
	"options" bytea NOT NULL,
	"supergroup_id" integer NOT NULL DEFAULT '0',
	CONSTRAINT characters_pk PRIMARY KEY ("id")
) WITH (
  OIDS=FALSE
);



CREATE TABLE "supergroups" (
	"id" serial NOT NULL,
	"supergroup_id" serial NOT NULL,
	"sg_name" varchar(30) NOT NULL,
	"sg_motto" varchar(128) NOT NULL,
	"sg_motd" varchar(1024) NOT NULL,
	"sg_rank_names" bytea NOT NULL,
	"sg_rank_perms" bytea NOT NULL,
	"sg_emblem" bytea NOT NULL,
	"sg_colors" bytea NOT NULL,
	"sg_members" bytea NOT NULL,
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

ALTER TABLE "progress" ADD CONSTRAINT "progress_fk0" FOREIGN KEY ("character_id") REFERENCES "characters"("id");

