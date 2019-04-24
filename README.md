35C3 Nokia phone challenge
==========================

This repository contains all the source code, infrastructure code and exploits for the 35C3 challenges called `newphonewhodis` and `identitytheft`

This includes:

* A nokia like UI application which resembles a phone with SMS sending and receiving capabilities
* A baseband implementation based on osmocom-bb capable of sending and receiving SMS
* A bare minimum SIM card which communicates with the baseband over a socket or a trustzone interface
* Build infrastructure to build a Yocto Linux containing all these components
* Docker files to run a virtphy based GSM basestation
* Docker files to build everything
* All the files that were distributed and deployed during the CTF if you'd like to play the original challenge
* Reference exploits for the challenges

All of this stuff is highly experimental as it is a CTF challenge that I cobbled together in just a few weeks. There is absolutely NO SUPPORT from my side. I just published all of this so that people can learn new stuff and have fun with the code.

Running the development environment like during the CTF
-------------------------------------------------------

NOTE: If you just want to play the challenge, do this! The rest is just if you are interested how all of this is built.

If you want to play around with the environment which was distributed during the CTF, check out the `delivery/development` subfolder. It contains a README with more instructions.

Building the Yocto Linux
------------------------

Get all the git submodules:

    git submodule update --init --recursive

Build the build container:

    cd yocto_buildenv
    ./build.sh

Run the build container:

    cd yocto_buildenv
    ./build.sh
    ./run.h

In the container:

    cd /project/linux
    source poky/oe-init-build-env ./build
    bitbake nokia-image

or if you want to build the prod image with real flags, without a compiler etc.:

    bitbake nokia-image-prod

For the prod image to work properly, you need to edit the `optee-os_git.bb` recipe in `linux/meta-nokia/recipes-security` and include a patch which changes the default crypto keys which are used in a qemu environment. I did this so that the teams aren't able to use the development qemu to decrypt the SIM file from the production environment. I was too lazy to do it properly and only apply the patch when needed.
If you don't apply this, the SIM trustlet won't be able to decrypt the installed SIM files and you won't have a SIM card.

Running the Yocto Image in qemu
-------------------------------

To run the built image in qemu, you need a patch which removes some semohosting stuff which you could abuse to take over the host but is needed for the initial bootloader to work as it loads the trustzone kernel etc. The patch also adds support for the Nokia display (yes, it is the same controller that was used in real Nokia phones back in the day :) )

    cd qemu
    ./build.sh
    ./run.sh

You need to figure out by yourself how to make this thing reach a virtphy basestation.

Running the baseband, SIM and UI for development purposes
---------------------------------------------------------

If you want to compile the code and run this on your regular PC, you can do so. That it how most of the code was initially developed

Building everything:

    cd nokia_ui/sim && make
    cd ../layer1 && make
    cd ../baseband && make
    cd ../ui && make
    cd ../..

To build the UI, you need `python2` and `PIL` (or `pillow` which is the successor) and `libsdl2`.

Start `layer1`:

    cd nokia_ui/layer1 && ./layer1

Start `sim`:

    cd nokia_ui/sim && ./layer1

Note: The `sim` executable will look in the current directory for a `sim.json`. See the reference file for its contents. There are also test SIM cards in `linux/meta-nokia/sims`.

Start `baseband`:

    cd nokia_ui/baseband && ./baseband

Start `ui`:

    cd nokia_ui/ui && ./nokia_ui

The PIN depends on your SIM card of course. The default `sim.json` uses `1234`

Make sure you have a working Osmocom VirtPHY Basestation running in your network if you want to use the GSM network. You may need to alter the SIM card for the phone to log onto your network.
If you need an additional GSM phone on the virtphy network, you can either set up a real second BTS using an SDR and use a real phone or you can build `osmocom-bb` and use its virtphy layer1. For a ready-made environment, check out `delivery/development` which builds a complete virtphy base station and an `osmocom-bb phone` with all the properly configured SIM cards etc.

