# Trifle Psychotic

## About

Trifle Psychotic is a​ retro sci-fi platformer written in ANSI C.

The game can be played and/or downloaded here: [itch.io](https://janorzechowski.itch.io/trifle-psychotic)

My main website: [janorzechowski.com](https://janorzechowski.com/)

Overview of the game's architecture can be found here: [ARCHITECTURE.md](/ARCHITECTURE.md).

## License

The game's code is released under the zlib license.

The game's assets are under different licenses. Levels' data (.tmx files in `data` folder) is under Creative Commmons Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0) license, with attribution to Jan Orzechowski, 2022. Graphical and audio assets are under licenses chosen by their creators - check `credits` files in `audio` and `gfx` folders.

## Building process

### Windows version

Windows version is built using Visual Studio 2022. Solution files are provided in the repository.

The project uses SDL2 (2.42.1), SDL2_image (2.6.2), and SDL2_mixer (2.6.2) libraries, which are not included in this repository. 

You can grab them here:
* [SDL2](https://github.com/libsdl-org/SDL/releases/tag/release-2.24.1)
* [SDL_image](https://github.com/libsdl-org/SDL_image/releases/tag/release-2.6.2)
* [SDL_mixer](https://github.com/libsdl-org/SDL_mixer/releases/tag/release-2.6.2)

You will need DLLs, LIBs, and include headers. To get them, look into `devel` folders in these releases, eg. `SDL2_mixer-devel-2.6.2-VC`. The contents of these folders should be put in `sdl` folder in the repository's main directory, in a subfolder of a respective library, so the paths will look like this: `(main directory)/sdl/SDL2`, `(main directory)/sdl/SDL2_image`, `(main directory)/sdl/SDL2_mixer/`. DLLs should be in the main directory.

You can also change the solution properties and put the libraries somewhere else. Just remember to change both `Configuration Properties>VC++ Directories` and `Configuration Properties>Linker>Input`.

`audio`, `gfx`, `data`, and `mapmaking` folders are copied as a post-build step, defined in `Configuration Properties>Build Events>Post-Build Event`. You might want to update the path to 7-zip or another compression tool there.

The game is compiled for x64.

To debug, set `TRIFLE_DEBUG` and/or `TRIFLE_DEBUG_COLLISION` compile flags. 

To debug a particular map, you can pass a map name - eg. `map_01`a as a command line argument. The game will then start without showing the main menu or the death screen, to speed up map debugging.

### Browser version

The browser version is built using Emscripten (version 3.1.23). 

Emscripten build is done by batch scripts: `emscripten_debug_build` and `emscripten_release_build`.

To use these scripts, you must change some paths in them:
* paths to your local copy of the repository 
* path to your local Emscripten installation
* path to a compressing program, such as 7z (or you can delete the last compression step)
* and path to your browser of choice, in case of the debug build

While debugging an Emscripten build, do not use `SAFE_HEAP` flag. Since the game uses an unaligned push buffer, this flag will cause detection of false issues.