PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE `table_versions` (
    `id` INTEGER PRIMARY KEY AUTOINCREMENT,
    `table_name` VARCHAR NOT NULL,
    `version` INTEGER NOT NULL DEFAULT 0,
    `last_update` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP
);

INSERT INTO table_versions VALUES(1,'db_version',8,'2018-05-03 17:52:33');
INSERT INTO table_versions VALUES(2,'table_versions',0,'2017-11-11 08:57:42');
INSERT INTO table_versions VALUES(3,'accounts',1,'2017-05-03 12:56:03');
INSERT INTO table_versions VALUES(4,'characters',8,'2018-05-04 14:58:27');
INSERT INTO table_versions VALUES(5,'costume',0,'2017-11-11 08:57:43');
INSERT INTO table_versions VALUES(7,'supergroups',1,'2018-05-03 12:56:43');
INSERT INTO table_versions VALUES(8,'emails',0,'2018-09-23 08:00:00');

CREATE TABLE `accounts` (
    `id` INTEGER PRIMARY KEY,
    `max_slots` INTEGER NOT NULL DEFAULT 8
);

CREATE TABLE `characters` (
    `id`	INTEGER PRIMARY KEY AUTOINCREMENT,
    `account_id`	INTEGER NOT NULL,
    `slot_index`	INTEGER NOT NULL DEFAULT 0,
    `char_name`	TEXT NOT NULL,
    `chardata`	BLOB,
    `entitydata`	BLOB,
    `bodytype`	INTEGER NOT NULL DEFAULT 4,
    `height` real NOT NULL DEFAULT 0.0,
    `physique` real NOT NULL DEFAULT 0.0,
    `supergroup_id`	INTEGER NOT NULL DEFAULT 0,
    `player_data` BLOB,
    FOREIGN KEY(`account_id`) REFERENCES accounts ( id ) ON DELETE CASCADE
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

CREATE TABLE `supergroups` (
    `id`	INTEGER PRIMARY KEY AUTOINCREMENT,
    `sg_name`	TEXT NOT NULL,
    `sg_motto`	TEXT,
    `sg_motd`	TEXT,
    `sg_rank_names`	BLOB,
    `sg_rank_perms`	BLOB,
    `sg_emblem`	BLOB,
    `sg_colors`	BLOB,
    `sg_members`	BLOB
);

CREATE TABLE 'emails'(
	`id`	INTEGER PRIMARY KEY AUTOINCREMENT,
	`sender_id`	INTEGER NOT NULL,
	`recipient_id` INTEGER NOT NULL,
	`email_data` BLOB,
	FOREIGN KEY(`sender_id`) REFERENCES characters ( id ) ON DELETE CASCADE,
	FOREIGN KEY(`recipient_id`) REFERENCES characters ( id ) ON DELETE CASCADE
);
COMMIT;

