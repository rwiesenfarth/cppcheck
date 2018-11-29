# **Cppcheck** 

|Linux Build Status|Windows Build Status|Coverity Scan Build Status|
|:--:|:--:|:--:|
|[![Linux Build Status](https://img.shields.io/travis/danmar/cppcheck/master.svg?label=Linux%20build)](https://travis-ci.org/danmar/cppcheck)|[![Windows Build Status](https://img.shields.io/appveyor/ci/danmar/cppcheck/master.svg?label=Windows%20build)](https://ci.appveyor.com/project/danmar/cppcheck/branch/master)|[![Coverity Scan Build Status](https://img.shields.io/coverity/scan/512.svg)](https://scan.coverity.com/projects/512)|

## Donations

If you find Cppcheck useful for you, feel free to make a donation.

[![Donate](http://pledgie.com/campaigns/4127.png)](http://pledgie.com/campaigns/4127)

## About the name

The original name of this program was "C++check", but it was later changed to "Cppcheck".

Despite the name, Cppcheck is designed for both C and C++.

## Manual

A manual is available [online](http://cppcheck.sourceforge.net/manual.pdf).

## Compiling on Windows - the preferred way

If you want to build Cppcheck the same way it was built for distribution on Windows, you should follow these steps to keep the pain points at a minimum.

### Prerequisites

You will need:
* CMake 3.x - mandatory, version 3.9.4 definitely works. [Download link](https://cmake.org/download/)
* Visual Studio 2015 - mandatory, later versions should also work, but require tweaking the WiX installer files. You may get the Community version from Microsoft's [Visual Studio homepage](https://visualstudio.microsoft.com/)
* Qt 5.x - mandatory for the GUI part, available on the [Qt download page](https://www.qt.io/download-qt-installer). Make sure to download a version that matches your Visual Studio version and choose "64bit", if both 32bit and 64bit are available.
* WiX Toolset - mandatory to create the installer, available on the [WiX Download page](http://wixtoolset.org/releases/)
* WiX Toolset Visual Studio Extension - mandatory to create the installer, available as a Visual Studio extension and installed there.

You do not need (but may decide to use it):
* PCRE original version - the pcre.h and pcre.lib files are included in the externals directory as prebuild version. If you decide to build these files on your own, use the Subversion repository at svn://vcs.exim.org/pcre/code/trunk. The files used by Cppcheck can be build with the help of CMake without any additional components (no need for zlib, bz2,...).

### Building the software

The software is build in three stages: configuration using CMake, compiling using Visual Studio and packaging using Visual Studio with the WiX Toolkit.

Preparation:
* Open a "Developer Command Prompt for VS2015"
* Set the environment variable QTDIR to point to your Qt directory (so that qmake.exe is in %QTDIR%\bin)

The configuration stage with CMake consists of these steps:
* From the above command prompt, run cmake-gui
* Set the source directory to the cppcheck directory and the build directory to a subdirectory named "build".
* "Configure", using "Visual Studio 14 2015 Win64" as the generator (note the "Win64"!)
* Enable BUILD / BUILD_GUI, BUILD / BUILD_TESTS and Ungrouped Entries / HAVE_RULES
* "Configure" again
* Check the settings in Ungrouped Entries. These should point to pcre.lib and the Qt5... directories.
* "Configure" again, followed by "Generate"

To actually compile the code, do
* Run devenv from the above command prompt
* Load the Cppcheck.sln solution from the build directory above
* Make sure to set the configuration to "Release"
* Choose "Build Solution"

When the solution is built, the installer package is created with these steps:
* In Visual Studio, open the solution win_installer\cppcheck.sln from the cppcheck directory
* Make sure that configuration is set to Release and platform is set to x64
* Choose "Build Solution"

The final .msi file is generated in win_installer\Build.

If you are using a different version of Visual Studio, you have to adjust the CrtMergeModule settings in win_installer\config.wxi to reflect the correct runtime.

## Compiling - alternative approaches

Any C++11 compiler should work. For compilers with partial C++11 support it may work. If your compiler has the C++11 features that are available in Visual Studio 2013 / GCC 4.6 then it will work.

To build the GUI, you need Qt.

When building the command line tool, [PCRE](http://www.pcre.org/) is optional. It is used if you build with rules.

There are multiple compilation choices:
* cmake - cross platform build tool
* qmake - cross platform build tool
* Windows: Visual Studio (VS 2010 and above)
* Windows: Qt Creator + mingw
* gnu make
* g++ 4.6 (or later)
* clang++

### cmake

If you are familiar with CMake, this might be the best option. On Windows, the generated solution includes both GUI (if Qt is available) and the testrunner, generating the most complete package.
 
On Windows, just run cmake-gui and choose your favorite generator. To build the GUI, you need Qt and must specify the path to Qt's CMake files:

```shell
cmake-gui -DCMAKE_PREFIX_PATH=%QTDIR%/lib/cmake
```

On Linux, it's probably the same procedure, but I did not (yet) test it there.

### qmake

There is a build.bat script in the root directory. Just call it to get the usage message.

You can also use the gui/gui.pro file to build the GUI directly.

```shell
cd gui
qmake
make
```

### Visual Studio

Use the cppcheck.sln file. The file is configured for Visual Studio 2015, but the platform toolset can be changed easily to older or newer versions. The solution contains platform targets for both x86 and x64.

To compile with rules, select "Release-PCRE" or "Debug-PCRE" configuration. pcre.lib (pcre64.lib for x64 builds) and pcre.h are expected to be in /externals then.

### Qt Creator + MinGW

The PCRE dll is needed to build the CLI. It can be downloaded here:
http://software-download.name/pcre-library-windows/

### GNU make

Simple, unoptimized build (no dependencies):

```shell
make
```

The recommended release build is:

```shell
make SRCDIR=build CFGDIR=cfg HAVE_RULES=yes CXXFLAGS="-O2 -DNDEBUG -Wall -Wno-sign-compare -Wno-unused-function"
```

Flags:

1.  `SRCDIR=build`  
    Python is used to optimise cppcheck

2.  `CFGDIR=cfg`  
    Specify folder where .cfg files are found

3.  `HAVE_RULES=yes`  
    Enable rules (PCRE is required if this is used)

4.  `CXXFLAGS="-O2 -DNDEBUG -Wall -Wno-sign-compare -Wno-unused-function"`
    Enables most compiler optimizations, disables cppcheck-internal debugging code and enables basic compiler warnings.

### g++ (for experts)

If you just want to build Cppcheck without dependencies then you can use this command:

```shell
g++ -o cppcheck -std=c++11 -Iexternals/simplecpp -Iexternals/tinyxml -Ilib cli/*.cpp lib/*.cpp externals/simplecpp/simplecpp.cpp externals/tinyxml/*.cpp
```

If you want to use `--rule` and `--rule-file` then dependencies are needed:

```shell
g++ -o cppcheck -std=c++11 -lpcre -DHAVE_RULES -Ilib -Iexternals/simplecpp -Iexternals/tinyxml cli/*.cpp lib/*.cpp externals/simplecpp/simplecpp.cpp externals/tinyxml/*.cpp
```

### MinGW

```shell
mingw32-make LDFLAGS=-lshlwapi
```

### Other Compiler/IDE

1. Create a empty project file / makefile.
2. Add all cpp files in the cppcheck cli and lib folders to the project file / makefile.
3. Add all cpp files in the externals folders to the project file / makefile.
4. Compile.

### Cross compiling Win32 (CLI) version of Cppcheck in Linux

```shell
sudo apt-get install mingw32
make CXX=i586-mingw32msvc-g++ LDFLAGS="-lshlwapi" RDYNAMIC=""
mv cppcheck cppcheck.exe
```

## Webpage

http://cppcheck.sourceforge.net/
