# Renderer

Strip-based immediate-mode renderer for the 240×240 round GC9A01 display.

## Design

The renderer uses a **primitive list** approach. Each frame follows this cycle:

1. `ltapp_render_begin(bg_color)` — reset primitive/icon lists, set background color
2. Draw calls — append primitives and icons to internal lists
3. `ltapp_render_flush()` — rasterize all primitives in horizontal strips, DMA each strip to the display

There is no retained scene graph. Every frame is built from scratch.

## Coordinate System

- Origin (0, 0) is the **top-left** corner of the 240×240 pixel grid.
- X increases to the right, Y increases downward.
- The display center is at (120, 120).
- Convenience defines: `LCD_WIDTH_HALF` (120), `LCD_HEIGHT_HALF` (120).

## Strip Rendering

The full framebuffer (240×240×2 = 115,200 bytes) doesn't fit in ESP32-C3 RAM. Instead, the display is rendered in **40-line horizontal strips** (240×40×2 = 19,200 bytes each).

Two strip buffers alternate (double-buffered) so the CPU can render the next strip while the previous one is being DMA-transferred to the display:

```
Strip 0: rows   0–39   → render into buf[0], DMA send
Strip 1: rows  40–79   → render into buf[1], DMA send
Strip 2: rows  80–119  → render into buf[0], DMA send
Strip 3: rows 120–159  → render into buf[1], DMA send
Strip 4: rows 160–199  → render into buf[0], DMA send
Strip 5: rows 200–239  → render into buf[1], DMA send
```

Each primitive is rasterized against **every strip** — only pixels that fall within the current strip's row range are actually drawn. This means draw order is preserved naturally (later primitives overdraw earlier ones).

## Rendering Pipeline (per strip)

Within each strip, rendering happens in this fixed order:

1. **Background fill** — every pixel set to `bg_color`
2. **Background icon** — single 1bpp bitmap drawn UNDER primitives (e.g., a large die outline that text overlays)
3. **Primitives** — rects, circles, rounded rects, lines, characters (max 256 per frame)
4. **Foreground icons** — up to 8 1bpp bitmaps drawn ON TOP of everything

This layering is fixed and cannot be reordered per-frame.

## Circular Clipping

Since the physical display is round, `ltapp_render_set_clip_circle(120, 120, 120)` is called at the start of every screen render. It enables per-pixel clipping: any `put_pixel` call checks `(px - cx)² + (py - cy)² <= r²` before writing. Pixels outside the circle are discarded.

The clip applies to **all rendering** (background fill does NOT respect the clip — it fills the full rectangular strip buffer, but primitives, icons, and text all respect it).

`ltapp_render_clear_clip()` disables the clip mask (rarely needed).

## Primitives

Primitives are stored in a flat array (`MAX_PRIMS = 256`). Each draw call appends a `prim_t` struct containing the type, position, dimensions, color, and optional font/scale/char data.

### Primitive types

| Type | Rasterization method |
|------|---------------------|
| `PRIM_RECT_FILL` | Scanline fill within bounds |
| `PRIM_RECT` | Four edges (hline/vline) |
| `PRIM_RRECT_FILL` | Rect fill + corner arcs (filled) |
| `PRIM_RRECT` | Rect outline + corner arcs |
| `PRIM_CIRCLE_FILL` | Horizontal scanlines per row (sqrt-based) |
| `PRIM_CIRCLE` | Bresenham 8-point symmetry |
| `PRIM_HLINE` | Direct pixel row |
| `PRIM_VLINE` | Vertical pixel column |
| `PRIM_CHAR` | Bitmap font lookup + scaled pixel output |

Dashed lines (`ltapp_render_hline_dashed`, `ltapp_render_vline_dashed`) expand into multiple `PRIM_HLINE`/`PRIM_VLINE` primitives at draw time.

## Icons

Icons are 1bpp bitmaps stored separately from the primitive list (they don't count against the 256-prim limit).

| Function | Layer | Limit | Use case |
|----------|-------|-------|----------|
| `ltapp_render_icon()` | Foreground (on top) | 8 per frame | UI icons, buttons, indicators |
| `ltapp_render_icon_bg()` | Background (under prims) | 1 per frame | Large decorative shapes with text overlay |

### Icon data format

- Row-major, MSB-first, packed 1bpp
- Stride = `(width + 7) / 8` bytes per row
- Total size = stride × height bytes
- Set pixels (1) are drawn in the specified color
- Clear pixels (0) are transparent (background shows through)

### Icons in use

| Icon | Size | Bytes | Description |
|------|------|-------|-------------|
| `icon_die` | 40×40 | 200 | Small die icon |
| `icon_crossed_swords` | 40×40 | 200 | Commander damage indicator |
| `icon_coin_small` | 40×40 | 200 | Small coin icon |
| `icon_user` | 30×30 | 120 | Player/opponent avatar |
| `icon_die_d4/d6/d8/d12/d20` | 120×120 | 1800 | Large die shape backgrounds |
| Coin animation frames | 120×120 | 1800 | Coin flip animation (in `coin_anim.h`) |

### Adding a new icon

1. Create a 1bpp bitmap (e.g., using an image editor → export as raw binary, or generate programmatically)
2. Pack as `static const uint8_t icon_name[STRIDE * HEIGHT]` in `icons.h`
3. Render with `ltapp_render_icon(x, y, icon_name, WIDTH, HEIGHT, color)`

## Fonts

| ID | Constant | Char size | Coverage | Source | Use case |
|----|----------|-----------|----------|--------|----------|
| 0 | `FONT_SMALL` | 5×7 | Full ASCII (32–127) | Classic LED matrix | Labels, compact text |
| 1 | `FONT_BASE` | 8×16 | Full ASCII (32–127) | VGA ROM / CP437 | Default UI text |
| 2 | `FONT_XLARGE` | 16×32 | Digits + minus only (0–9, `-`) | Terminus-derived | Life counter, dice result |

### Text rendering details

- **Scale:** The `scale` parameter multiplies each font pixel (scale=2 → each pixel becomes a 2×2 block).
- **Spacing:** 1px gap between characters, regardless of font or scale.
- **Pixel width formula:** `text_width = (char_width × scale + 1) × strlen - 1`
- **Custom glyph:** Character `\x7F` (DEL) renders as a right-pointing triangle (▶) in `FONT_SMALL` and `FONT_BASE`.
- **String centering:** `ltapp_render_string_centered(y, ...)` computes X to horizontally center the string in the 240px display width.

## Colors

All colors are 16-bit RGB565, byte-swapped for the GC9A01's SPI byte order:

```c
#define RGB565_RAW(r, g, b) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | (((b) & 0xF8) >> 3))
#define RGB565(r, g, b)     __builtin_bswap16(RGB565_RAW(r, g, b))
```

### Palette

| Constant | RGB | Hex | Notes |
|----------|-----|-----|-------|
| `COLOR_BLACK` | (5, 7, 10) | #05070A | Near-black (not pure 0,0,0) |
| `COLOR_WHITE` | (242, 244, 248) | #F2F4F8 | Soft white |
| `COLOR_RED` | (255, 49, 49) | #FF3131 | |
| `COLOR_PINK_RED` | (255, 90, 159) | #FF5A9F | |
| `COLOR_GREEN` | (93, 255, 106) | #5DFF6A | |
| `COLOR_YELLOW` | (255, 216, 61) | #FFD83D | |
| `COLOR_CYAN` | (34, 215, 255) | #22D7FF | |
| `COLOR_CYAN_BRIGHT` | (58, 232, 255) | #3AE8FF | Brighter cyan variant |
| `COLOR_MAGENTA` | (255, 79, 216) | #FF4FD8 | Primary accent |
| `COLOR_ORANGE` | (255, 155, 61) | #FF9B3D | |
| `COLOR_GRAY` | (168, 176, 188) | #A8B0BC | |
| `COLOR_DGRAY` | (42, 47, 56) | #2A2F38 | Subtle separators |

Custom colors can be created with the `RGB565(r, g, b)` macro.

## Measurement Utilities

These functions calculate pixel dimensions **without drawing anything**. Use them for layout:

```c
int ltapp_render_text_width(str, font, scale);   // total pixel width of string
int ltapp_render_number_width(num, font, scale); // pixel width of integer as string
int ltapp_render_text_height(font, scale);       // pixel height (same for all chars in a font)
int ltapp_render_number_height(font, scale);     // same as text_height
```

### Circle geometry helpers

For laying out content within the round display:

```c
int ltapp_circle_width_at(int r, int y_from_center);
int ltapp_circle_height_at(int r, int x_from_center);
```

- `ltapp_circle_width_at(120, 0)` → 240 (full width at center)
- `ltapp_circle_width_at(120, 60)` → ~208 (narrower near top/bottom)
- `y_from_center` is signed: negative = above center, positive = below center
- Returns 0 if `|y_from_center| >= r`

Typical use: calculating how wide a horizontal line or text area can be at a given Y coordinate without being clipped.

## API Reference

### Lifecycle

```c
void ltapp_render_init(void);            // Call once at startup
void ltapp_render_begin(uint16_t bg);    // Start new frame
void ltapp_render_flush(void);           // Rasterize + DMA to display
```

### Clipping

```c
void ltapp_render_set_clip_circle(int cx, int cy, int r);
void ltapp_render_clear_clip(void);
```

### Primitives

```c
void ltapp_render_rect(int x, int y, int w, int h, uint16_t color);
void ltapp_render_rect_fill(int x, int y, int w, int h, uint16_t color);
void ltapp_render_rounded_rect(int x, int y, int w, int h, int r, uint16_t color);
void ltapp_render_rounded_rect_fill(int x, int y, int w, int h, int r, uint16_t color);
void ltapp_render_circle(int cx, int cy, int r, uint16_t color);
void ltapp_render_circle_fill(int cx, int cy, int r, uint16_t color);
void ltapp_render_hline(int x, int y, int w, uint16_t color);
void ltapp_render_vline(int x, int y, int h, uint16_t color);
void ltapp_render_hline_dashed(int x, int y, int w, int dash, int gap, uint16_t color);
void ltapp_render_vline_dashed(int x, int y, int h, int dash, int gap, uint16_t color);
```

### Text

```c
void ltapp_render_char(int x, int y, char c, uint16_t color, int font, int scale);
void ltapp_render_string(int x, int y, const char *str, uint16_t color, int font, int scale);
void ltapp_render_string_centered(int y, const char *str, uint16_t color, int font, int scale);
void ltapp_render_number(int x, int y, int num, uint16_t color, int font, int scale);
void ltapp_render_number_centered(int y, int num, uint16_t color, int font, int scale);
```

### Icons

```c
void ltapp_render_icon(int x, int y, const uint8_t *bitmap, int w, int h, uint16_t color);
void ltapp_render_icon_bg(int x, int y, const uint8_t *bitmap, int w, int h, uint16_t color);
```

### Measurement

```c
int ltapp_render_text_width(const char *str, int font, int scale);
int ltapp_render_number_width(int num, int font, int scale);
int ltapp_render_text_height(int font, int scale);
int ltapp_render_number_height(int font, int scale);
int ltapp_circle_width_at(int r, int y_from_center);
int ltapp_circle_height_at(int r, int x_from_center);
```

## Performance

- **Frame time:** ~33ms target (30 FPS cap in render task)
- **DMA transfer:** Non-blocking — CPU renders next strip while previous transfers
- **Primitive limit:** 256 per frame (`MAX_PRIMS`; exceeding silently drops primitives)
- **Icon limit:** 8 foreground + 1 background per frame
- **RAM usage:** 2 × 19,200 bytes (strip buffers) + primitive array + icon array ≈ 42KB total
- **No allocations:** All buffers are static. Zero heap usage during rendering.
