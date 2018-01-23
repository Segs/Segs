PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE `table_versions` (
    `id` INTEGER PRIMARY KEY AUTOINCREMENT,
    `table_name` VARCHAR NOT NULL,
    `version` INTEGER NOT NULL DEFAULT 0,
    `last_update` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP
);

INSERT INTO table_versions VALUES(1,'db_version',0,'2018-01-06 16:27:01');
INSERT INTO table_versions VALUES(2,'table_versions',0,'2017-11-11 08:57:42');
INSERT INTO table_versions VALUES(3,'accounts',0,'2017-11-11 08:57:43');
INSERT INTO table_versions VALUES(4,'characters',1,'2018-01-06 16:16:27');
INSERT INTO table_versions VALUES(5,'costume',0,'2017-11-11 08:57:43');
INSERT INTO table_versions VALUES(6,'progress',0,'2017-11-11 08:57:43');

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
	`char_level`	INTEGER NOT NULL DEFAULT 0,
	`archetype`	TEXT NOT NULL,
	`origin`	TEXT NOT NULL,
	`description` VARCHAR(1024) NOT NULL,
	`battlecry` VARCHAR(30) NOT NULL,
	`bodytype`	INTEGER NOT NULL DEFAULT 4,
	`current_map`	INTEGER NOT NULL,
	`last_costume_id`	integer DEFAULT 0,
	`last_online` DATETIME DEFAULT CURRENT_TIMESTAMP,
	`hitpoints` INTEGER DEFAULT 0,
	`endurance` INTEGER DEFAULT 0,
	`inf` INTEGER DEFAULT 0,
	`xp` INTEGER DEFAULT 0,
	`xpdebt` INTEGER DEFAULT 0,
	`xppatrol` INTEGER DEFAULT 0,
	`alignment` VARCHAR(30) NOT NULL DEFAULT `hero`,
	`posx` INTEGER NOT NULL DEFAULT 0,
	`posy` INTEGER NOT NULL DEFAULT 0,
	`posz` INTEGER NOT NULL DEFAULT 0,
	`orientp` INTEGER NOT NULL DEFAULT 0,
	`orienty` INTEGER NOT NULL DEFAULT 0,
	`orientr` INTEGER NOT NULL DEFAULT 0,
	`title` VARCHAR(20),
	`badgetitle` VARCHAR(20),
	`specialtitle` VARCHAR(20),
	`supergroup_id` INTEGER DEFAULT 0,
        `options` BLOB,
        `gui` BLOB,
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
COMMIT;

