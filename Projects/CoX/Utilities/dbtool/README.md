SEGS DBTOOL v0.3
======

A tool used to manage the account and character databases.

PREREQUISITES
------

**1. Ensure `dbtool` is located in the correct directory.**

SEGS `dbtool` must be run from the SEGS output folder, where the `default_dbs` directory resides. This ensures that all relative pathing used in the code will execute correctly. The output directory should be located somewhere similar to:

```
PATH/TO/SEGS/INSTALL/build-PROJECT_NAME-Desktop_Qt_YOUR_QT_VERSION_MinGW_32bit-Debug/out
```

If `dbtool` is not run from this directory, you will receive an error.

_**// ADDITIONAL PREREQUISITES GO HERE //**_

INSTRUCTIONS
------

**1. Navigate to the SEGS output directory using a command prompt.**

Open a command prompt terminal. Using the `cd` command, change the working directory to the SEGS output folder, given above under "Prerequisites."

**2. Run `dbtool`.**

Run `dbtool` by typing the following command and pressing ENTER:

```
dbtool
```

Yes, it's that simple. If the `segs` and `segs_game` binary databases already exist in the SEGS output directory, you will need to add the optional `force` argument to the command in order to overwrite the existing databases:

```
dbtool -f
```

If the operation was successful, you should see two `COMPLETED` messages in the command prompt. In addition, the `segs` and `segs_game` binary databases should contain an updated "last modified" date.

_**// ADDITIONAL INFORMATION GOES HERE //**_
