# jvlt_core Library

Reusable core library for the JV Life Tap MTG life counter. Platform-independent game logic that can be paired with any ESP32 touch display board.

## Overview

`jvlt_core` handles:
- Navigation (screen state machine)
- Match logic (life, commander damage, poison)
- Dice rolling (D4/D6/D8/D12/D20)
- Coin flip
- Settings (NVS persistence)
- Net (optional ESP-NOW multiplayer — documented separately)

The library exposes a single public header: `jvlt.h`.

## Architecture

```
jvlt_init.c      — Entry point, task creation, queues, beacon timer
jvlt_nav.c       — Screen state (get/set)
jvlt_match.c     — Match session logic
jvlt_dice.c      — Dice rolling and side selection
jvlt_coin.c      — Coin flip
jvlt_settings.c  — Player name, brightness (NVS)
jvlt_net.c       — Room/beacon/multiplayer (optional)
jvlt_comm.c      — ESP-NOW transport layer (optional)
```

## Entry Point

```c
#include "jvlt.h"

void app_main(void) {
    // ... hardware init ...

    jvlt_init(&(jvlt_config_t){
        .on_input  = my_input_handler,
        .on_render = my_render_callback,
    });
}
```

`jvlt_init()` performs:
1. Initializes ESP-NOW communication
2. Creates input queue (16 slots) and packet queue (8 slots)
3. Loads settings from NVS (player name, brightness)
4. Sets initial screen to `SCREEN_HOME`
5. Starts a 1-second periodic beacon timer
6. Spawns two FreeRTOS tasks:
   - **game** (priority 5): processes input events and network packets
   - **render** (priority 3): calls `on_render` when dirty flag is set (~30 FPS)

## Callbacks

| Callback | Signature | Called from |
|----------|-----------|-------------|
| `on_input` | `void(const jvlt_input_event_t *ev)` | game task, when input queue has events |
| `on_render` | `void(void)` | render task, when `g_dirty` is true |

The app layer reads state via `jvlt_screen()`, `jvlt_match_get()`, `jvlt_dice_get()`, etc. and renders accordingly. State changes only happen through the core API.

## Dirty Flag

The dirty flag signals the render task that the UI needs a redraw.

```c
jvlt_mark_dirty();   // Set by core after any state change
jvlt_is_dirty();     // Checked by render task
jvlt_clear_dirty();  // Cleared before calling on_render
```

Most core functions set this automatically. The app layer can also call `jvlt_mark_dirty()` for app-specific animations.

## Input Events

Input events are produced by the app's touch driver and pushed to `jvlt_input_queue()`.

```c
typedef struct {
    jvlt_input_type_t type;  // TAP, DOUBLE_TAP, LONG_PRESS, SWIPE_*, etc.
    int16_t x, y;            // Current/final touch position
    int16_t x0, y0;          // Initial touch-down position
} jvlt_input_event_t;
```

## Utilities

### List Cycle/Peek

Generic helpers for cycling through a fixed list of values:

```c
// Wraps around: D20 +1 → D4
int16_t jvlt_list_cycle(list, count, value, dir);

// Returns NULL at boundaries (no wrap)
const int16_t *jvlt_list_peek(list, count, value, dir);
```

Used by dice sides, starting life, and opponent count.
