# Dice Roll Animation

App-level animation for dice rolling. The core lib (`jvlt_dice_roll`) decides the result immediately; the app layer handles the visual randomization effect.

## Overview

The animation rapidly cycles through random numbers before settling on the actual result. There are no sprite frames — just the number displayed in `FONT_XLARGE` changing each step.

## Animation Logic

### Start (`ltapp_dice_start_anim`)

1. Read current die sides from core (default 20 if unset)
2. Call `jvlt_dice_roll(sides)` — result is decided now
3. Set `total_steps = 18`
4. Set initial display value to a random number (not the result — to avoid flashing the answer on frame 1)
5. Start step counter at 0

### Tick (`ltapp_dice_is_animating`)

Called from the render loop every frame. Advances the animation when enough time has elapsed:

1. Compute delay for current step using quadratic easing:
   ```
   delay = 10 + (steps_done² × 150) / total_steps²
   ```
   - First step: ~10ms (very fast)
   - Last step: ~160ms (slow reveal)
2. If elapsed ≥ delay:
   - Advance step counter
   - If `steps_done >= total_steps`: set display to the real result, animation complete
   - Otherwise: set display to a new random number in `[1, sides]`
3. Mark dirty to trigger redraw

### Easing Curve

Same quadratic pattern as the coin. The animation gives the impression of a die tumbling and slowing down. Total duration is approximately:

- 18 steps × ~85ms average ≈ **1.5 seconds**

## Display Value

The UI reads `ltapp_dice_display_value()` to get what number to render:

- During animation: rapidly changing random values
- After animation: the final result from `jvlt_dice_get()->result`
- Before first roll (value = 0): UI shows nothing or a placeholder

## Input Blocking

While `ltapp_dice_is_animating()` returns true, all input on `SCREEN_DICE` is ignored. The user cannot re-roll, change die type, or navigate away until the animation completes.

## Rendering

The UI renders the display value as a large centered number on top of the die shape background icon:

```c
// Background: die shape (e.g., d20 outline)
ltapp_render_icon_bg(60, 48, icon_die_d20, 120, 120, COLOR_CYAN);

// Foreground: result number
ltapp_render_number_centered(y, ltapp_dice_display_value(), color, FONT_XLARGE, 2);
```

## State Reset

When navigating away from `SCREEN_DICE` (swipe right), the app calls:
- `jvlt_dice_reset()` — clears sides and result in core
- `s_dice_display = 0` — clears the app-level display value
