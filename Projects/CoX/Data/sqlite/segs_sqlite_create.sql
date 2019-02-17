PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE `table_versions` (
    `id` INTEGER PRIMARY KEY AUTOINCREMENT,
    `table_name` VARCHAR NOT NULL,
    `version` INTEGER NOT NULL DEFAULT 0,
    `last_update` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP
);

INSERT INTO table_versions VALUES(1,'db_version',1,'2019-02-17 03:11:58');
INSERT INTO table_versions VALUES(2,'table_versions',0,'2017-11-11 08:55:54');
INSERT INTO table_versions VALUES(3,'accounts',1,'2018-01-06 11:18:01');
INSERT INTO table_versions VALUES(4,'game_servers',0,'2017-11-11 09:12:37');
INSERT INTO table_versions VALUES(5,'bans',0,'2017-11-11 09:12:37');

CREATE TABLE `accounts` (
	`id`	INTEGER PRIMARY KEY AUTOINCREMENT,
	`username`	TEXT UNIQUE,
	`access_level`	INTEGER DEFAULT 1,
	`creation_date`	DATETIME DEFAULT CURRENT_TIMESTAMP,
        `passw`	BLOB,
        `salt` BLOB
);

CREATE TABLE `bans` (
    `id` INTEGER PRIMARY KEY AUTOINCREMENT,
    `account_id` VARCHAR NOT NULL,
    `offending_ip` INTEGER NOT NULL DEFAULT 0,
    `started` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `reason` VARCHAR NOT NULL,
    FOREIGN KEY(`account_id`) REFERENCES accounts ( id ) ON DELETE CASCADE,
	UNIQUE (account_id)
);

CREATE TABLE `game_servers` (
    `id` INTEGER PRIMARY KEY AUTOINCREMENT,
    `addr` VARCHAR NOT NULL,
    `port` INTEGER NOT NULL DEFAULT 0,
    `name` VARCHAR NOT NULL,
    `token` INTEGER NOT NULL
);
COMMIT;

