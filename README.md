# mio2it

mio2it is a tool that converts music data from WarioWare DIY, stored in MIO files, into an [Impulse Tracker](https://en.wikipedia.org/wiki/Impulse_Tracker) module.

## Download

Releases are published at https://github.com/dakrk/mio2it/releases.

## Building

Please view [BUILDING.md](BUILDING.md).

## Why?

The sequence data and the way you compose with the WarioWare DIY music composer is analogous to a tracker:

- You have a finite number of channels, which can only play a single note at a time
- The song is split up into phrases (like how trackers have patterns)
- Notes must be placed on-beat (unlike many other sequenced formats)

Impulse Tracker modules are supported by a wide range of software, and can be edited with modern trackers such as [OpenMPT](https://openmpt.org), and can be played back with a lot of music players.

## TODO:

- Use instrument/sample data from SDAT
- Optimise pattern writing (they are *way* too large at the moment)
