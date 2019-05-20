PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE `table_versions` (
    `id` INTEGER PRIMARY KEY AUTOINCREMENT,
    `table_name` VARCHAR NOT NULL,
    `version` INTEGER NOT NULL DEFAULT 0,
    `last_update` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP
);

INSERT INTO table_versions VALUES(1,'db_version',10,'2019-04-28 22:56:43');
INSERT INTO table_versions VALUES(2,'table_versions',0,'2017-11-11 08:57:42');
INSERT INTO table_versions VALUES(3,'accounts',1,'2017-05-03 12:56:03');
INSERT INTO table_versions VALUES(4,'characters',12,'2019-04-28 22:56:43');
INSERT INTO table_versions VALUES(7,'supergroups',2,'2018-10-22 22:56:43');
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
    `costume_data` BLOB,
    `chardata`	BLOB,
    `entitydata`	BLOB,
    `player_data` BLOB,
    `supergroup_id`	INTEGER NOT NULL DEFAULT 0,
    FOREIGN KEY(`account_id`) REFERENCES accounts ( id ) ON DELETE CASCADE
);

CREATE TABLE `supergroups` (
    `id`	INTEGER PRIMARY KEY AUTOINCREMENT,
    `sg_name`	TEXT NOT NULL,
    `sg_data`	BLOB,
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

