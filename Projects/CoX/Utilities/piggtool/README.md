SEGS PIGGTOOL v0.1
======

A tool used to extract archived files from the City of Heroes `.pigg` files.


ADVANCED GUIDE
------
Please run `SEGSAdmin` tool for setup and server configuration. The piggtool and this guide are for ADVANCED USERS only and require intimate knowledge of command line tools.


COMMAND LINE ARGUMENTS
------

**1. File to process and extraction folder**

To extract a pigg file, please run piggtool with the `-x` command line option:

```
piggtool -x misc.pigg
```


**2. A List of all command line options and arguments**

- `-l`: List files in pigg archive
- `-x`: Extract all files from pigg archive
- `pigg_file`: File to process
- `target_directory`: Optional directory to put extracted files in