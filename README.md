# libcsdb _unofficial source repository!_

FabiC 2016-06-16

_Couldn't find any SVN / Git repository for this so I'm quickly adding it under Git control for I need to play with it for a while..._

* Web site: http://libcsdbg.sourceforge.net/
* https://sourceforge.net/projects/libcsdbg/
* by [tasos42 @ SourceForge](https://sourceforge.net/u/tasos42/profile/) <tasos42@users.sourceforge.net>
* License : [LGPLv3](http://libcsdbg.sourceforge.net/index.html#sec1_3)
* This repo. has the content of tarball [version 1.28](https://sourceforge.net/projects/libcsdbg/files/1.28/) released on April 20th 2014.

## Alterations

* Added a `config.h` file : quickfix solves build problem where a system `bfd.h` header file gets in the way.
* Added a symlink `build` to the hidden `.build/` directory.
* _(Note that a `.deps` file left by `make` may get in your way)._

## Build instructions

* See <http://libcsdbg.sourceforge.net/index.html#sec3>
* See the [Makefile](Makefile)

### Just run `make`

    make

_**EOF**_
