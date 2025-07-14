# LEGO Island, SPARC

This is a port of the LEGO Island decompilation project to SPARC architecture, currently targeting Solaris 9 on SPARC V8.

## Current Status
NOCD.SI works.

All the basic necessities for running the game are implemented, including:
- Crosscompilation toolchain:
  - sysroot can be extracted from a Solaris 9 SPARC installation using provided script
  - GCC 4.8.1 built from source, one of the first versions to fully support C++11, one of the last versions to support SPARC V8
  - libstdc++ as Solaris 9 doesnt provide it by default - even if it did, it would be too old
  - a few custom headers, mainly OS compat, math and atomic functions (yep, no atomics on SPARC V8)
  - test programs to verify compilers and libs
  - packaging everything into a tarball
- SDL3
  - not officially supported on Solaris but easy enough to get working
  - no opengl for now
  - backported atomic operations to SPARC V8
  - SDL_GetError() segfaults, no idea why, just dont call it lol
- ISLE
  - backports for atomic ops
  - Solaris-specific patches
  - C++11 instead of C++17
  - endianess fixes (developed PTSD, thanks M$ and IBM for making RIFFs so little-endian dependent)
  - disabled audio, no idea if it works, dont really care for now
  - disabled joystick/gamepad (...)
  - disabled config app as I don't want to play around with compiling Qt6 for SPARC

## Preparation

1. Get Solaris 9 on a VM or a real machine. 

For VM the recommended approach is this https://github.com/andarazoroflove/sparc/releases prebuilt qcow2 image. Run with:

```bash
qemu-system-sparc -M SS-5 -m 256 -cdrom ./SOL_9_SPARC_DVD.iso -drive file=sparc9.qcow2,if=scsi,bus=0,unit=0 -net nic -net user,hostfwd=tcp::2222-:22 -vga tcx -g 1024x768x24
```

This will run VM like a SparcStation 5, with 256MB RAM, Solaris 9 DVD as a CD-ROM, and a prebuilt qcow2 image as the hard drive. The VM will forward port 2222 on the host to port 22 on the VM for SSH. Display is set to tcx with 24 bit color, as SDL no longer supports the default 8-bit color depth (learned the hard way). Username is "root" without a password.

After boot, change 
PermitEmptyPasswords to yes
PermitRootLogin to yes
in /etc/ssh/sshd_config in order to use ssh
...or just create a user with a password, idc.

2. Libraries

Thank god that OpenCSW still exists. Install instructions from here
https://cdn.opencsw.org/manual/for-administrators/setup-old-versions.html#old-solaris
doesnt seem to have proper paths, so here is a quick guide:
 /usr/sfw/bin/wget http://mirror.opencsw.org/opencsw/pkgutil.pkg
 /usr/sbin/pkgadd -d pkgutil.pkg all

This repo assumes PATH being set with /opt/csw/bin.

To install the required libraries, run the following commands:

```bash
pkgutil -y -i libx11 libxext
```

that should be enough, if you get lib errors just install the missing libs with pkgutil.

## Building

Just run `docker build .` and look for `/workspace/isle-portable-solaris9-sparc.tar` in the image.

## Running

To run the game, copy the tarball to your Solaris 9 SPARC machine and extract it:

```bash
tar -xvf isle-portable-solaris9-sparc.tar
```

As there are a few dynamic libs, use:
```bash
./run_isle.sh
```
to run the game. This script sets the necessary environment variables and runs the game executable.

Also, make sure you have all the required assets, following this structure:

```
isle
└── LEGO
    ├── data
    │   ├── ACT1INF.DTA
    │   ├── ACT2INF.DTA
    │   ├── ACT3INF.DTA
    │   ├── BLDDINF.DTA
    │   ├── BLDHINF.DTA
    │   ├── BLDJINF.DTA
    │   ├── BLDRINF.DTA
    │   ├── GMAININF.DTA
    │   ├── HOSPINF.DTA
    │   ├── ICUBEINF.DTA
    │   ├── IELEVINF.DTA
    │   ├── IISLEINF.DTA
    │   ├── IMAININF.DTA
    │   ├── IREGINF.DTA
    │   ├── OBSTINF.DTA
    │   ├── PMAININF.DTA
    │   ├── RACCINF.DTA
    │   ├── RACJINF.DTA
    │   ├── testinf.dta
    │   └── WORLD.WDB
    └── Scripts
        ├── Act2
        │   └── ACT2MAIN.SI
        ├── Act3
        │   └── ACT3.SI
        ├── Build
        │   ├── COPTER.SI
        │   ├── DUNECAR.SI
        │   ├── JETSKI.SI
        │   └── RACECAR.SI
        ├── CREDITS.SI
        ├── Garage
        │   └── GARAGE.SI
        ├── Hospital
        │   └── HOSPITAL.SI
        ├── Infocntr
        │   ├── ELEVBOTT.SI
        │   ├── HISTBOOK.SI
        │   ├── INFODOOR.SI
        │   ├── INFOMAIN.SI
        │   ├── INFOSCOR.SI
        │   └── REGBOOK.SI
        ├── INTRO.SI
        ├── Isle
        │   ├── ISLE.SI
        │   ├── JUKEBOX.SI
        │   └── JUKEBOXW.SI
        ├── NOCD.SI
        ├── Police
        │   └── POLICE.SI
        ├── Race
        │   ├── CARRACER.SI
        │   ├── CARRACE.SI
        │   ├── JETRACER.SI
        │   └── JETRACE.SI
        └── SNDANIM.SI
```

You might need to set the proper path in config file - check debug output for its path.