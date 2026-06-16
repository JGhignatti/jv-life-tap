# Coin Flip

Simple heads/tails coin flip.

## Data Structure

```c
typedef struct {
    bool flipped;  // has been flipped at least once
    bool heads;    // true = heads, false = tails
} jvlt_coin_t;
```

## API

```c
const jvlt_coin_t *jvlt_coin_get(void);
void  jvlt_coin_flip(void);   // generate result
void  jvlt_coin_reset(void);  // clear state
```

## Behavior

- `jvlt_coin_flip()` uses `esp_random() & 1` for 50/50 odds.
- `flipped` is set to `true` after the first flip (used by UI to distinguish "not yet flipped" from "result is tails").
- `jvlt_coin_reset()` clears both fields.
- Marks dirty on flip.

## Animation

The coin flip animation is **app-owned** (in `game_task.c` + `coin_anim.h`), not part of the core. The core decides the result immediately on `jvlt_coin_flip()`. The app layer handles the visual spin animation with multiple icon frames before revealing the result.

### Animation frames (app layer)

8-frame cycle: heads → heads_1 → edge → tails_2 → tails → tails_1 → edge → heads_2

The animation calculates how many steps to spin so it naturally lands on the target frame (heads=0 or tails=4), avoiding jarring jumps.
