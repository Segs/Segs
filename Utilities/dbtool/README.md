SEGS DBTOOL v0.4
======

A tool used to manage the account and character SQL databases. It is also used for the creation of new user accounts.


ADVANCED GUIDE
------
Please run `SEGSAdmin` tool for setup and server configuration. The dbtool and this guide are for ADVANCED USERS only and require intimate knowledge of command line tools.


COMMAND LINE ARGUMENTS
------

**1. Creating or Resettting Databases**

To create new databases run dbtoool with the command below:

```
dbtool create
```

If the `segs` and `segs_game` database files already exist in the SEGS output directory, you will need to add the `-force` or `-f` argument to the command in order to overwrite the existing databases:

```
dbtool create -f
```


**2. Creating New Users**

To create new users run dbtoool with the `adduser` command. It requires three command line options, `-l` for login username, `-p` for password, and `-a` for access level. An example below:

```
dbtool adduser -l username -p password -a 9
```


**3. A List of all command line options**

- `-f` or `-force`: Overwrite existing database files. THIS CANNOT BE UNDONE.
- `-l` or `-login`: Provide login for added account
- `-p` or `-pass`: Provide password for added account
- `-a` or `-access_level`: Provide access_level [0-9] for account
- `-c` or `-config`: Use provided settings file, default = settings.cfg