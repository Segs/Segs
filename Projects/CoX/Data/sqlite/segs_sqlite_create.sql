CREATE TABLE `table_versions` (
    `id` INTEGER PRIMARY KEY AUTOINCREMENT,
    `table_name` VARCHAR NOT NULL,
    `version` INTEGER NOT NULL DEFAULT 0,
    `last_update` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP
);

INSERT INTO "table_versions" (table_name)
VALUES ('table_versions'),
VALUES ('accounts'),
VALUES ('bans'),
VALUES ('game_servers');

CREATE TABLE `accounts` (
	`id`	INTEGER PRIMARY KEY AUTOINCREMENT,
	`username`	TEXT UNIQUE,
	`access_level`	INTEGER DEFAULT 1,
	`creation_date`	DATETIME DEFAULT CURRENT_TIMESTAMP),
	`passw`	BLOB
);

INSERT INTO `accounts` (username,passw)
VALUES (`segsadmin`,x`segs123`);

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
