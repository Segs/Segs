PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE `table_versions` (
    `id` INTEGER PRIMARY KEY AUTOINCREMENT,
    `table_name` VARCHAR NOT NULL,
    `version` INTEGER NOT NULL DEFAULT 0,
    `last_update` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP
);

INSERT INTO table_versions VALUES(1,'db_version',1,'2018-01-23 10:27:01');
INSERT INTO table_versions VALUES(2,'table_versions',0,'2017-11-11 08:57:42');
INSERT INTO table_versions VALUES(3,'accounts',0,'2017-11-11 08:57:43');
INSERT INTO table_versions VALUES(4,'characters',2,'2018-01-23 10:16:27');
INSERT INTO table_versions VALUES(5,'costume',0,'2017-11-11 08:57:43');
INSERT INTO table_versions VALUES(6,'progress',0,'2017-11-11 08:57:43');
INSERT INTO table_versions VALUES(7,'supergroups',0,'2018-01-23 10:16:43');

CREATE TABLE `accounts` (
	`id`	INTEGER PRIMARY KEY AUTOINCREMENT,
	`account_id`	INTEGER UNIQUE,
	`max_slots`	INTEGER NOT NULL DEFAULT 8
);

INSERT INTO accounts VALUES(1,1,8);

CREATE TABLE `characters` (
	`id`	INTEGER PRIMARY KEY AUTOINCREMENT,
	`account_id`	INTEGER NOT NULL,
	`slot_index`	INTEGER NOT NULL DEFAULT 0,
	`char_name`	TEXT NOT NULL,
	`chardata`	BLOB,
	`bodytype`	INTEGER NOT NULL DEFAULT 4,
	`last_online`	DATETIME DEFAULT CURRENT_TIMESTAMP,
	`hitpoints`	INTEGER DEFAULT 0,
	`endurance`	INTEGER DEFAULT 0,
	`posx`	INTEGER NOT NULL DEFAULT 0,
	`posy`	INTEGER NOT NULL DEFAULT 0,
	`posz`	INTEGER NOT NULL DEFAULT 0,
	`orientp`	INTEGER NOT NULL DEFAULT 0,
	`orienty`	INTEGER NOT NULL DEFAULT 0,
	`orientr`	INTEGER NOT NULL DEFAULT 0,
        `options`	BLOB,
	`supergroup_id`	INTEGER NOT NULL DEFAULT 0,
	FOREIGN KEY(`account_id`) REFERENCES accounts ( account_id ) ON DELETE CASCADE
);

CREATE TABLE `costume` (
	`id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	`character_id` INTEGER NOT NULL,
	`costume_index` INTEGER NOT NULL,
	`skin_color` INTEGER NOT NULL,
	`parts` blob,
	FOREIGN KEY(`character_id`) REFERENCES characters ( id ) ON DELETE CASCADE,
	UNIQUE (character_id, costume_index)
);

CREATE TABLE `progress` (
	`id` integer PRIMARY KEY AUTOINCREMENT,
	`character_id` integer,
	`badges` blob,
	`clues` blob,
	`contacts` blob,
	`souvenirs` blob,
	FOREIGN KEY(`character_id`) REFERENCES characters ( id ) ON DELETE CASCADE,
	UNIQUE (character_id)
);

CREATE TABLE `supergroups` (
	`id`	INTEGER PRIMARY KEY AUTOINCREMENT,
	`supergroup_id`	INTEGER UNIQUE,
	`sg_name`	TEXT NOT NULL,
	`sg_motto`	TEXT,
	`sg_motd`	TEXT,
	`sg_rank_names`	BLOB,
	`sg_rank_perms`	BLOB,
	`sg_emblem`	BLOB,
	`sg_colors`	BLOB,
	`sg_members`	BLOB
);
COMMIT;

