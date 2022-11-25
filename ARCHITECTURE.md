# Architecture overview and additional remarks

The game uses SDL, and the entry point is defined in `sdl_platform.c`. Here, using SDL functions, we gather input, play and stop music, and also render all bitmaps. Rendering is done in the `render_list_to_output` function.

`sdl_platform.c` is the only file including SDL headers. The rest of the game's code is both platform-independent and SDL-independent. `sdl_platform.c` calls `main.c`, passing to it a struct containing function pointers to platform-related tasks, such as reading files or loading a new music track.

In `main.c`, we have `main_game_loop` function, which contains scene management, i. e. switching between scenes such as the main menu scene or the level choice scene. The `game_update_and_render` contains actual gameplay.

In this function, we first update all entities - player, enemies, power-ups, gates, switches, moving platforms  - and then perform drawing. First, we draw backdrops, then tiles (in three layers), then entities, bullets, and explosions, and then optionally the message box.

Drawing is done by filling a render list. This is done by functions such as `render_rectangle` or `render_bitmap`. The resulting list isn't sorted - things are rendered in the order in which they were added. This limitation is really visible only in one place. While drawing entities, we first draw switches and gates, since they conceptually are part of the map, and so it looks more natural for them to be drawn below all other entities.

How enemies work: only player movement is handled with full collision detection with both environment and other entities. Enemies move in a more simplified way - see `process_entity_movement`. Each moving enemy moves along a path and does not check collision with tiles. This path is generated at an entity's first update and takes into account the position of gates and switches. If a gate is opened, the paths of enemies will be re-evaluated - see `open_gates_with_given_color` function. 

The map conceptually consists of rectangular tiles. For the purposes of movement and collision detection, the positions of entities and tiles are represented as floating point 2D vectors, relative to 16 by 16 chunks of tiles. Each entity position is stored as a `world_position`, consisting of a chunk position and an offset within it. Translations between various position representations are handled in `map.c`.

This partitioning of a map into chunks is also used for optimization - entities that are more than one chunk away from the player's position are not updated. (See the usage of `is_in_neighbouring_chunk` function).

Each entity has a type - `entity_type` - which holds information such as health, behavior type (is it enemy? does it move? etc.), graphics, and collision rectangle, among others. This means all entities of the same type will always have the same size. One unintuitive consequence of that is that since gates and switches can have different colors and sizes, each such entity must have a unique type. Also, colors and sizes depend on map data, so the types must be generated during map initialization.  See `initialize_current_map` and functions defined in `special_entities.c`.

Entity types are usually not modified during gameplay. The exception is the player type, which max health property changes when the player picks up a health power-up.

The game has checkpoints within levels, which are restored in case the player dies. Checkpoints handling is very simple, and it relies on the fact that memory management is done via push buffers (memory arenas), which causes the pointers to be stable, provided that the map data wasn't modified between the initializations (this would happen only when someone was manually messing with the file outside of the game). 

Each time we load a new level or restore a checkpoint, a map file is parsed. Each parsing results in deterministic pushes to a memory arena. Therefore, we only need to save things that can be updated during gameplay. And things such as entity types for gates and switches, despite being generated during level initialization, don't need to be saved. See `save_checkpoint` and `restore_checkpoint` functions in `progress.c`.

Memory cleanup after each level is simple. In `change_and_initialize_level` we use `begin_temporary_memory`. It marks the first address after which we store level-specific data (scenes other than gameplay, such as the main menu scene, do not allocate memory at all). Then, when we need to load another level, we first call `end_temporary_memory`, which will zero all the memory used after the stored first address, and will give us a clean space for new allocations. 

In general, almost everything is allocated in memory arenas (initialized in `initialize_game_state`), and nothing is allocated on the heap except the temporary allocations for file loading (see the few places where `free` is used). In this way, we can be sure that there are no memory leakages.