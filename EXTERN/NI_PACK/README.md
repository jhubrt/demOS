# GUP sources for Demo Coders and GUP Embedding in Your Software

## What's this, doc?

These are the bare-bones memory-to-memory GUP decompression sources, which can be embedded in your software to *decompress* any file data which has been compressed by the `gup` tool.

Each source file represents one of the depack modes, corresponding to the same *pack mode* used with `gup` at the time you packed the data.

Next to the sources, a simple sample application (depack_t.c) has been provided which showcases all the depack modes together in a single application. Demo coders and others will generally only require the use of *one* depack mode, so you may then want to ditch most of the sources.

The sample application lists the files encoded in the given ARJ archive file and lists additional information, such as CRC and depack time. This was done so the demo application was at least doing something remotely useful. ;-)

## Building the sources on platform X

We have provided project / make / build files for the following platforms:

- `atari_st.prj` is the Atari ST PureC project file which builds the `ni_packer` demo application, combined with optimized MC68K assembly sources located in the `atari_st/` directory.
- `Makefile` is a very simple GCC `make` file for everyone to enjoy.
- `windows/msvc2022/demo.sln` is for all of you Microsoft Visual Studio adepts: load the `.sln` solution into Visual Studio (Community Edition or better ;-) ) and Build All: you will then see *two* demo applications, which are functionally identical.


### The demo applications' source code explained

#### demo 1 (ni_packer.vcxproj for MSVC2022, etc.)

`depack_t.c` (+ the rest): compiles each source file individually and links them all together to form the demo application. This is a rough&ready way of building the sources.


#### demo 2 (ni_packer_demo.vcxproj for MSVC2022, etc.)

`demo_app.c`: this way of embedding shwocases the original intent: the desired depack source code files are `#include`d in your application. This is one way demo coders include a specific depacker source and we show here this is doable, even when you choose to use more than one depack algortihm.


## I've built this stuff and I want to see it run!

Okay, just a quick run-down of a sample session given the current sample code (your final embedding will of course use `gup` cdump mode or other smart ways to reduce header costs, etc.etc. ;-) )

First, we create a basic ARJ/GUP archive using the provided `gup.exe` compressor:

```
echo "pack the stuff for demo ARJ file production"
cd demo-sources
../gup.exe a -r -m7 ../demo.arj *.c
cd ..
```

> **Note**: the result of the command above is also included already in this distro as `demo.arj` just in case you wonder and wish to debug your workflow.

Now, *assuming* you have built ni_packer in Release / x64 mode in MSVC2022, you can **list & test** the ARJ archive file produced and you may then expect output similar to the stuff shown below:

```
$ echo "run ni_packer without arguments to test your build quickly:"
$ demo-sources/windows/msvc2022/bin/Release-Unicode-64bit-x64/ni_packer.exe
Usage: Q:\nyh\gup\demo-sources\windows\msvc2022\bin\Release-Unicode-64bit-x64\ni_packer.exe <file to be tested>

$ echo "list and test demo.arj:"
$ demo-sources/windows/msvc2022/bin/Release-Unicode-64bit-x64/ni_packer.exe demo.arj
File name:               original        packed mode CRC32
arj_crc.c                    1150           521   7  1BAF6B46    0.000 s CRC OK
arj_m4.c                     3184           989   7  078F42A5    0.000 s CRC OK
arj_m7.c                    14472          3215   7  203BBD5D    0.000 s CRC OK
demo_app.c                    391           202   7  3E81AE81    0.000 s CRC OK
depack_t.c                   6590          1956   7  E3AF7754    0.000 s CRC OK
ni_n0.c                      1475           588   7  0C03215A    0.000 s CRC OK
ni_n1.c                      3399          1081   7  35C794E5    0.000 s CRC OK
ni_n2.c                      2248           724   7  1533C2A6    0.000 s CRC OK
unstore.c                     161           123   7  07B832D2    0.000 s CRC OK

totaal                      33070          9399                  0.000 s CRC OK
```

