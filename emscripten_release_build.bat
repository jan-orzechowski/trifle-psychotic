cd /d C:\Emscripten\emsdk-3.1.23 
call emsdk_env.bat

cd /d D:\JO_Programowanie\TriflePlatformer
if not exist build\wasm\release mkdir build\wasm\release

call emcc -std=c++14 -fno-rtti -fno-exceptions -Wno-switch code/sdl_platform.cpp code/animation.cpp code/backdrops.cpp code/collision.cpp code/debug.cpp code/entities.cpp code/game_data.cpp code/special_entities.cpp code/input.cpp code/jormath.cpp code/jorstring.cpp code/level_initialization.cpp code/level_parsing.cpp code/main.cpp code/map.cpp code/player.cpp code/progress.cpp code/rendering.cpp code/text_rendering.cpp code/tmx_parsing.cpp code/ui.cpp -o build/wasm/release/index.html --preload-file data --preload-file gfx --preload-file audio -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sSDL2_IMAGE_FORMATS=[png] -sUSE_SDL_MIXER=2 -sSDL2_MIXER_FORMATS=[ogg] -sUSE_OGG=1 -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=128MB -sTOTAL_STACK=32MB -O3 --shell-file emscripten_shell.html

del "build\wasm\release\trifle_psychotic_wasm.zip"
"C:\Program Files\7-Zip\7z.exe" a -tzip "build\wasm\release\trifle_psychotic_wasm.zip" %CD%\build\wasm\release\*