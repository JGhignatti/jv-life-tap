# Navigation

Simple screen state — no stack, no history. The app layer decides transitions.

## API

```c
jvlt_screen_t jvlt_screen(void);              // Get current screen
void          jvlt_set_screen(jvlt_screen_t); // Set screen (marks dirty)
```

## Screens

| Screen | Description |
|--------|-------------|
| `SCREEN_HOME` | Main menu |
| `SCREEN_SETTINGS_NAME` | Player name editing |
| `SCREEN_SETTINGS_BRIGHTNESS` | Brightness adjustment |
| `SCREEN_SOLO_SETUP` | Configure opponents & starting life |
| `SCREEN_NET_LOBBY` | Multiplayer lobby |
| `SCREEN_MATCH` | Active game — life counter |
| `SCREEN_MATCH_CMD_DAMAGE` | Commander damage select opponent |
| `SCREEN_MATCH_CMD_DAMAGE_OPP` | Commander damage +/− for one opponent |
| `SCREEN_MATCH_POISON_DAMAGE` | Poison counter |
| `SCREEN_MATCH_SETTINGS` | In-match settings (reset, leave) |
| `SCREEN_CONFIRM_RESET` | Confirm reset match |
| `SCREEN_CONFIRM_LEAVE` | Confirm leave match |
| `SCREEN_EXTRAS` | Dice & coin menu |
| `SCREEN_DICE` | Roll a die |
| `SCREEN_FLIP_COIN` | Flip a coin |

## Implementation

`jvlt_nav.c` — a single global `g_screen` variable. Setting it marks the dirty flag to trigger a redraw.

The core does not enforce any transition rules. The app's input handler (`on_input`) decides when to navigate based on gestures and touch areas.
