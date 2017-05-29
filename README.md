# snotif
a simple notification service for battery and network notifications. this was created as replacement for notifications *applets* often offer, without the need for *applets*. snotif was made with minimal memory footprint in mind (although using libnotify somewhat ruins this goal), and all configuration is done at compile time, a lot of configuration is optional and not needed for core functionality


snotif currently boasts the following features:
- Run daemonized
- Battery notifications (charging, discharging, full)
- Critical battery percentage warning, if configured/supported
- Optional extra information about percentage and time left, if configured/supported 
- WLAN notifications (disconnected from network, connected to network)
- mostly undocumented codebase

## setup
1. clone repo
2. `make clean install`
3. edit config.h to your liking
4. go back to step 2

## configuration
as mentioned before all configuration is done by editing `config.h` and recompiling, features left unconfigured(meaning not defined in `config.h`) will not be in your binary.

a few example configs are included, `config.def.h`, the default config. and `config_smapi.def.h`, which is meant for devices with smapi, using the extra features it offers. by default your `config.h` will be generated using `config.def.h`, manually copy `config_smapi.def.h` to `config.h`if you wish to use this as your base instead.

this should be sufficient for most systems allthough I cant promise the format will be compatible with each, if this is the case you'd have to manually correct how the file is parsed in snotif.c, or leave a pr to better implement support for your particular system.

## usage
it's suggested you start snotif with `snotif -d` from your startup script or other means

    usage: snotif [option]
    
    options:
        -d start daemonized
        -v print version info and exit
        -h print this info and exit

## TODO
- comment/document code
- possibly a cleaner solution than checking defines
- switch to less extensive notification library than libnotify 
- write a less scrapped together makefile
- write a small manualpage explaining the stuff mentioned in usage and this page

## bugs and contribution
you can report bugs or make feature requests on the issues page

for contribution, just help fix bugs and send a pr, or do whatever really, if it's useful I'll probably merge it.

## license
see the LICENSE file

## mentions
- [slstatus](https://github.com/drkhsh/slstatus) : inspiration for easily reading system information
- [suckless](http://suckless.org/coding_style) : some of the coding style (all these define checks arent allowed afaik, I'm also unsure if this actually *sucks less*, but there was an attempt)
