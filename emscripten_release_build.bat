cd /d C:\Emscripten\emsdk-3.1.23 
call emsdk_env.bat

cd /d D:\JO_Programowanie\Projekty\TriflePsychotic
if not exist build\wasm\release mkdir build\wasm\release

call emcc -std=c99 -Wno-switch -Wno-typedef-redefinition code/sdl_platform.c code/animation.c code/backdrops.c code/collision.c code/debug.c code/entities.c code/game_data.c code/special_entities.c code/input.c code/jormath.c code/jorstring.c code/level_initialization.c code/level_parsing.c code/main.c code/map.c code/player.c code/progress.c code/rendering.c code/text_rendering.c code/tmx_parsing.c code/ui.c -o build/wasm/release/index.html --preload-file data --preload-file gfx --preload-file audio -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sSDL2_IMAGE_FORMATS=[png] -sUSE_SDL_MIXER=2 -sSDL2_MIXER_FORMATS=[ogg] -sUSE_OGG=1 -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=128MB -sTOTAL_STACK=32MB -O3 --shell-file emscripten_shell.html

del "build\wasm\release\trifle_psychotic_wasm.zip"
"C:\Program Files\7-Zip\7z.exe" a -tzip "build\wasm\release\trifle_psychotic_wasm.zip" %CD%\build\wasm\release\*