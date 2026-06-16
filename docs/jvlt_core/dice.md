# Dice

Roll a die with support for D4, D6, D8, D12, and D20.

## Data Structure

```c
typedef struct {
    uint8_t sides;   // 4, 6, 8, 12, or 20 (0 = unset, defaults to 20)
    uint8_t result;  // last roll result (1..sides), 0 = not rolled
} jvlt_dice_t;
```

## API

```c
const jvlt_dice_t *jvlt_dice_get(void);

void jvlt_dice_roll(uint8_t sides);               // roll and store result
void jvlt_dice_cycle_sides(int8_t dir);           // cycle through options (wraps)
const int16_t *jvlt_dice_peek_sides(int8_t dir);  // peek next/prev (NULL at boundary)
void jvlt_dice_reset(void);                       // clear sides and result
```

## Side Options

Fixed list: `{4, 6, 8, 12, 20}`

- `jvlt_dice_cycle_sides(+1)` wraps: D20 → D4
- `jvlt_dice_peek_sides(+1)` returns NULL at D20 (no wrap)

## Behavior

- `jvlt_dice_roll()` uses `esp_random()` for the RNG.
- `jvlt_dice_cycle_sides()` clears the result (resets to "not rolled").
- When `sides == 0` (initial state), functions treat it as 20.
- All state changes mark dirty.

## Animation

The dice roll animation is **app-owned** (in `game_task.c`), not part of the core. The core only provides the final result via `jvlt_dice_get()->result`. The app layer handles the visual animation of random numbers cycling before landing on the result.
