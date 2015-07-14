# PatchMaker
Console Qt5 application that compares two folders and copies changed files into resulting folder. PatchMaker calculates CRC checksum for files for comparison. All new and changed files will be copied to resulting folder. Use -h -help or /? command line option to see help. 

Building
--------
Get Qt5 SDK, open PatchMaker.pro file in Qt Creator and build. 

Note
--------
Windows 8.1 asks for administrator privileges for PatchMaker.exe because the word "patch" is present in EXE file name. To solve this issue one may rename the resulting EXE file or embed a manifest that will make EXE file not requiring administrator privileges.
