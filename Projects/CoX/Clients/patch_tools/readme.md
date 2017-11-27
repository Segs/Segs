Patchint the client executable:

1. Copy the CityOfHeroes.exe to this directory.
2. Run patchit.cmd
3. Enjoy your patched CityOfX.exe

The bspatch is part of the bsdiff utility available here: https://www.pokorra.de/coding/bsdiff.html


After patching the CityOfX.exe will need libcohdll.dll to run.

Generating the CityOfX.lib ( an import library needed by libcohdll.dll to know what exports are available ) 
=========
under MSVC>=2015:
=========
1. From start menu run X86 Native Tools commandline
2. change the directory to the one containing CityOfX.def
3. run lib /def:CityOfX.def
=========
under mingw
=========
1. run mingw shell
2. change the directory to the one containing CityOfX.def
3. run /usr/i686-w64-mingw32/bin/dlltool -d CityOfX.def -l libCityOfX.a



