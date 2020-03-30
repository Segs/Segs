# SEGS Database Schemas
ATTENTION: View the latest version of this document on our Wiki!
- https://github.com/Segs/Segs/wiki/DB-Schema-and-Proposing-Changes

## SQL and SEGS database files
SEGS utilizes SQL databases for inter-session (long term) storage of data. The server can be configured to work with PostgreSQL, MySQL, or SQLite database drivers, and defaults to SQLite. There are two database files, `segs` for Accounts and account related information, and `segs_game` for Player Characters, Supergroups and other game related information that cannot be read from scripts.
- `segs` - Account information such as username, pass, and last login
- `segs_game` - Per-player information and transient game world data, such as player characters, stats, settings, progress, supergroups, etc. 

As a rule, SEGS attempts to use scripts for configuration and server-side data where possible and realistic. Player data and other "transient" information intended to be kept long term should be stored in the database.
- SCRIPT item: Contacts, Spawns, Missions, Zone configurations
- Database item: Supergroups, Player Character stats, Player Character costumes

## Current Database Schema and Proposing Changes
The current database schema is version 7. SEGS is using [dbdesigner.net](http://dbdesigner.net) to cooperatively work on database schema changes. We have a limited number of available shares for dbdesigner, so proposed changes to schemas should be made by creating a [new GitHub issue](https://github.com/Segs/Segs/issues/new), describing your changes, and submitting those changes in the appropriate database scripts in [`Projects/CoX/Data/`](https://github.com/Segs/Segs/tree/master/Projects/CoX/Data)

### `segs` database
![segs database](http://doxy.segs.dev/images/dbschema/segs_dbschema.png)

### `segs_game` database
![segs_game database](http://doxy.segs.dev/images/dbschema/segs_game_dbschema.png)
