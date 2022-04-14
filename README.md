# Description
A simple daemon which plays music.

# Design
The design is a bit weird, trying to emulate a plan9 file server thing without actually using 9p.
This results in a decently modular design, everything but simple play and pause are provided as a separate script, but results in some race conditions.
I'll probably redesign this at some point, but for now it works and is kind of cool.

Support can be easily added for more file formats besides ogg and wav by adding new entries to the `song_readers` structure in `config.h`.

# Dependencies
+ [libao](https://xiph.org/ao/)
+ [libsndfile](https://libsndfile.github.io/libsndfile/)
+ [dmenu](https://tools.suckless.org/dmenu/)
