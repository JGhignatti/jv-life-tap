# Coin Flip Animation

App-level animation for the coin flip. The core lib (`jvlt_coin_flip`) decides the result immediately; the app layer handles the visual spin.

## Frame Cycle

8 frames representing a coin rotating in 3D (1bpp icons, 120×120 each):

```
Index  Frame            Visual
  0    coin_frame_heads     HEADS (front face)
  1    coin_frame_heads_1   Heads tilting away
  2    coin_frame_edge      Edge (thin profile)
  3    coin_frame_tails_2   Tails emerging
  4    coin_frame_tails     TAILS (back face)
  5    coin_frame_tails_1   Tails tilting away
  6    coin_frame_edge      Edge (mirror, reuses frame 2)
  7    coin_frame_heads_2   Heads emerging
```

The frames loop seamlessly: `... → 7 → 0 → 1 → ...`

## Landing Targets

- **Heads:** frame 0 (`COIN_FRAME_HEADS`)
- **Tails:** frame 4 (`COIN_FRAME_TAILS`)

## Animation Logic

### Start (`ltapp_coin_start_anim`)

1. Call `jvlt_coin_flip()` — result is decided now (stored in core)
2. Calculate `total_steps = 24 + target_frame` (3 full rotations + offset to land on correct frame)
3. Start from frame 0, step counter = 0

### Tick (`ltapp_coin_is_animating`)

Called from the render loop every frame. Advances the animation when enough time has elapsed:

1. Compute delay for current step using quadratic easing:
   ```
   delay = 25 + (steps_done² × 175) / total_steps²
   ```
   - First step: ~25ms (fast spin)
   - Last step: ~200ms (slow settle)
2. If elapsed ≥ delay: advance step, set `frame = steps_done % 8`
3. If `steps_done >= total_steps`: animation complete, stop

### Easing Curve

The quadratic formula makes the coin start fast and decelerate naturally, simulating friction. The total animation duration is approximately:

- 3 spins × 8 frames × ~60ms average ≈ **1.4 seconds**

## Input Blocking

While `ltapp_coin_is_animating()` returns true, all input on `SCREEN_FLIP_COIN` is ignored. The user cannot re-flip or navigate away until the animation completes.

## Rendering

The UI layer calls `ltapp_coin_current_frame()` to get the current frame index, then renders the corresponding icon as a background icon:

```c
ltapp_render_icon_bg(60, 60, coin_frames[frame], 120, 120, color);
```
