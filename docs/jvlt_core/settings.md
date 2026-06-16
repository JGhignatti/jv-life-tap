# Settings

Persistent user settings stored in NVS (non-volatile storage).

## Stored Values

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `name` | `char[12]` | `"Player"` | Player display name |
| `brightness` | `uint8_t` | `200` | Display brightness (5–255) |

NVS namespace: `"jvlt"`

## API

### Player Name

```c
const char *jvlt_player_name(void);
void        jvlt_set_player_name(const char *name);
```

`set_player_name` rejects empty/NULL names, truncates to 11 chars, null-terminates, and persists to NVS immediately.

### Brightness

```c
uint8_t jvlt_brightness(void);
void    jvlt_set_brightness(uint8_t level);
void    jvlt_adjust_brightness(int delta);
```

- `set_brightness` persists to NVS and marks dirty.
- `adjust_brightness` clamps to 5–255 range, then calls `set_brightness`.
- The app layer is responsible for applying the brightness value to the display hardware.

## Loading

Settings are loaded from NVS during `jvlt_init()`. If the NVS partition has no stored values, defaults are used.
