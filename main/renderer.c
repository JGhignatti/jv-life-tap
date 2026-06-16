#include "renderer.h"
#include "display.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// ─── 5x7 Font ────────────────────────────────────────────────────────────────
#define FONT5_W 5
#define FONT5_H 7

static const uint8_t font5x7[][5] = {
    {0x00,0x00,0x00,0x00,0x00}, // space
    {0x00,0x00,0x5F,0x00,0x00}, // !
    {0x00,0x07,0x00,0x07,0x00}, // "
    {0x14,0x7F,0x14,0x7F,0x14}, // #
    {0x24,0x2A,0x7F,0x2A,0x12}, // $
    {0x23,0x13,0x08,0x64,0x62}, // %
    {0x36,0x49,0x55,0x22,0x50}, // &
    {0x00,0x05,0x03,0x00,0x00}, // '
    {0x00,0x1C,0x22,0x41,0x00}, // (
    {0x00,0x41,0x22,0x1C,0x00}, // )
    {0x08,0x2A,0x1C,0x2A,0x08}, // *
    {0x08,0x08,0x3E,0x08,0x08}, // +
    {0x00,0x50,0x30,0x00,0x00}, // ,
    {0x08,0x08,0x08,0x08,0x08}, // -
    {0x00,0x60,0x60,0x00,0x00}, // .
    {0x20,0x10,0x08,0x04,0x02}, // /
    {0x3E,0x51,0x49,0x45,0x3E}, // 0
    {0x00,0x42,0x7F,0x40,0x00}, // 1
    {0x42,0x61,0x51,0x49,0x46}, // 2
    {0x21,0x41,0x45,0x4B,0x31}, // 3
    {0x18,0x14,0x12,0x7F,0x10}, // 4
    {0x27,0x45,0x45,0x45,0x39}, // 5
    {0x3C,0x4A,0x49,0x49,0x30}, // 6
    {0x01,0x71,0x09,0x05,0x03}, // 7
    {0x36,0x49,0x49,0x49,0x36}, // 8
    {0x06,0x49,0x49,0x29,0x1E}, // 9
    {0x00,0x36,0x36,0x00,0x00}, // :
    {0x00,0x56,0x36,0x00,0x00}, // ;
    {0x00,0x08,0x14,0x22,0x41}, // <
    {0x14,0x14,0x14,0x14,0x14}, // =
    {0x41,0x22,0x14,0x08,0x00}, // >
    {0x02,0x01,0x51,0x09,0x06}, // ?
    {0x32,0x49,0x79,0x41,0x3E}, // @
    {0x7E,0x11,0x11,0x11,0x7E}, // A
    {0x7F,0x49,0x49,0x49,0x36}, // B
    {0x3E,0x41,0x41,0x41,0x22}, // C
    {0x7F,0x41,0x41,0x22,0x1C}, // D
    {0x7F,0x49,0x49,0x49,0x41}, // E
    {0x7F,0x09,0x09,0x01,0x01}, // F
    {0x3E,0x41,0x41,0x51,0x32}, // G
    {0x7F,0x08,0x08,0x08,0x7F}, // H
    {0x00,0x41,0x7F,0x41,0x00}, // I
    {0x20,0x40,0x41,0x3F,0x01}, // J
    {0x7F,0x08,0x14,0x22,0x41}, // K
    {0x7F,0x40,0x40,0x40,0x40}, // L
    {0x7F,0x02,0x04,0x02,0x7F}, // M
    {0x7F,0x04,0x08,0x10,0x7F}, // N
    {0x3E,0x41,0x41,0x41,0x3E}, // O
    {0x7F,0x09,0x09,0x09,0x06}, // P
    {0x3E,0x41,0x51,0x21,0x5E}, // Q
    {0x7F,0x09,0x19,0x29,0x46}, // R
    {0x46,0x49,0x49,0x49,0x31}, // S
    {0x01,0x01,0x7F,0x01,0x01}, // T
    {0x3F,0x40,0x40,0x40,0x3F}, // U
    {0x1F,0x20,0x40,0x20,0x1F}, // V
    {0x7F,0x20,0x18,0x20,0x7F}, // W
    {0x63,0x14,0x08,0x14,0x63}, // X
    {0x03,0x04,0x78,0x04,0x03}, // Y
    {0x61,0x51,0x49,0x45,0x43}, // Z
    {0x00,0x00,0x7F,0x41,0x41}, // [
    {0x02,0x04,0x08,0x10,0x20}, // backslash
    {0x41,0x41,0x7F,0x00,0x00}, // ]
    {0x04,0x02,0x01,0x02,0x04}, // ^
    {0x40,0x40,0x40,0x40,0x40}, // _
    {0x00,0x01,0x02,0x04,0x00}, // `
    {0x20,0x54,0x54,0x54,0x78}, // a
    {0x7F,0x48,0x44,0x44,0x38}, // b
    {0x38,0x44,0x44,0x44,0x20}, // c
    {0x38,0x44,0x44,0x48,0x7F}, // d
    {0x38,0x54,0x54,0x54,0x18}, // e
    {0x08,0x7E,0x09,0x01,0x02}, // f
    {0x08,0x54,0x54,0x54,0x3C}, // g
    {0x7F,0x08,0x04,0x04,0x78}, // h
    {0x00,0x44,0x7D,0x40,0x00}, // i
    {0x20,0x40,0x44,0x3D,0x00}, // j
    {0x00,0x7F,0x10,0x28,0x44}, // k
    {0x00,0x41,0x7F,0x40,0x00}, // l
    {0x7C,0x04,0x18,0x04,0x78}, // m
    {0x7C,0x08,0x04,0x04,0x78}, // n
    {0x38,0x44,0x44,0x44,0x38}, // o
    {0x7C,0x14,0x14,0x14,0x08}, // p
    {0x08,0x14,0x14,0x18,0x7C}, // q
    {0x7C,0x08,0x04,0x04,0x08}, // r
    {0x48,0x54,0x54,0x54,0x20}, // s
    {0x04,0x3F,0x44,0x40,0x20}, // t
    {0x3C,0x40,0x40,0x20,0x7C}, // u
    {0x1C,0x20,0x40,0x20,0x1C}, // v
    {0x3C,0x40,0x30,0x40,0x3C}, // w
    {0x44,0x28,0x10,0x28,0x44}, // x
    {0x0C,0x50,0x50,0x50,0x3C}, // y
    {0x44,0x64,0x54,0x4C,0x44}, // z
    {0x00,0x08,0x36,0x41,0x00}, // {
    {0x00,0x00,0x7F,0x00,0x00}, // |
    {0x00,0x41,0x36,0x08,0x00}, // }
    {0x08,0x04,0x08,0x10,0x08}, // ~
    {0x7F,0x3E,0x1C,0x08,0x00}, // ▶ (char 127) '\x7F'
};

// ─── 8x16 Font (columns, 16 bits each) ──────────────────────────────────────
// VGA ROM font, column-encoded: each column is uint16_t, bit 0 = top row.
#define FONT8_W 8
#define FONT8_H 16

static const uint16_t font8x16[][8] = {
    {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000}, // space
    {0x0000,0x0000,0x0038,0x0DFC,0x0DFC,0x0038,0x0000,0x0000}, // !
    {0x0000,0x000E,0x001E,0x0000,0x0000,0x0000,0x001E,0x000E}, // "
    {0x0000,0x0220,0x0FF8,0x0FF8,0x0220,0x0FF8,0x0FF8,0x0220}, // #
    {0x0000,0x0638,0x0C7C,0x0844,0x3847,0x3847,0x0FCC,0x0798}, // $
    {0x0000,0x0860,0x0C60,0x0600,0x0300,0x0180,0x0CC0,0x0C60}, // %
    {0x0000,0x0880,0x0FB0,0x0778,0x09C8,0x08F8,0x0FB0,0x0700}, // &
    {0x0000,0x0000,0x0000,0x0000,0x000E,0x001E,0x0010,0x0000}, // '
    {0x0000,0x0000,0x03F0,0x07F8,0x0C0C,0x0804,0x0000,0x0000}, // (
    {0x0000,0x0000,0x0000,0x0804,0x0C0C,0x07F8,0x03F0,0x0000}, // )
    {0x0080,0x02A0,0x03E0,0x01C0,0x01C0,0x03E0,0x02A0,0x0080}, // *
    {0x0000,0x0080,0x0080,0x03E0,0x03E0,0x0080,0x0080,0x0000}, // +
    {0x0000,0x0000,0x0000,0x0E00,0x1E00,0x1000,0x0000,0x0000}, // ,
    {0x0000,0x0080,0x0080,0x0080,0x0080,0x0080,0x0080,0x0000}, // -
    {0x0000,0x0000,0x0000,0x0C00,0x0C00,0x0000,0x0000,0x0000}, // .
    {0x0000,0x0C00,0x0600,0x0300,0x0180,0x00C0,0x0060,0x0030}, // /
    {0x0000,0x07F8,0x0FFC,0x0984,0x08C4,0x0864,0x0FFC,0x07F8}, // 0
    {0x0000,0x0000,0x0810,0x0818,0x0FFC,0x0FFC,0x0800,0x0800}, // 1
    {0x0000,0x0E08,0x0F0C,0x0984,0x08C4,0x0864,0x0C3C,0x0E18}, // 2
    {0x0000,0x0408,0x0C0C,0x0844,0x0844,0x0844,0x0FFC,0x07B8}, // 3
    {0x0000,0x00C0,0x00E0,0x00B0,0x0898,0x0FFC,0x0FFC,0x0880}, // 4
    {0x0000,0x047C,0x0C7C,0x0844,0x0844,0x0844,0x0FC4,0x0784}, // 5
    {0x0000,0x07F0,0x0FF8,0x084C,0x0844,0x0844,0x0FC0,0x0780}, // 6
    {0x0000,0x000C,0x000C,0x0F04,0x0F84,0x00C4,0x007C,0x003C}, // 7
    {0x0000,0x07B8,0x0FFC,0x0844,0x0844,0x0844,0x0FFC,0x07B8}, // 8
    {0x0000,0x0078,0x08FC,0x0884,0x0884,0x0C84,0x07FC,0x03F8}, // 9
    {0x0000,0x0000,0x0000,0x0630,0x0630,0x0000,0x0000,0x0000}, // :
    {0x0000,0x0000,0x0000,0x0630,0x0E30,0x0800,0x0000,0x0000}, // ;
    {0x0000,0x0080,0x01C0,0x0360,0x0630,0x0C18,0x0808,0x0000}, // <
    {0x0000,0x0120,0x0120,0x0120,0x0120,0x0120,0x0120,0x0000}, // =
    {0x0000,0x0808,0x0C18,0x0630,0x0360,0x01C0,0x0080,0x0000}, // >
    {0x0000,0x0018,0x001C,0x0004,0x0DC4,0x0DE4,0x003C,0x0018}, // ?
    {0x0000,0x07F0,0x0FF8,0x0808,0x0BC8,0x0BC8,0x0BF8,0x01F0}, // @
    {0x0000,0x0FE0,0x0FF0,0x0098,0x008C,0x0098,0x0FF0,0x0FE0}, // A
    {0x0000,0x0804,0x0FFC,0x0FFC,0x0844,0x0844,0x0FFC,0x07B8}, // B
    {0x0000,0x03F0,0x07F8,0x0C0C,0x0804,0x0804,0x0C0C,0x0618}, // C
    {0x0000,0x0804,0x0FFC,0x0FFC,0x0804,0x0C0C,0x07F8,0x03F0}, // D
    {0x0000,0x0804,0x0FFC,0x0FFC,0x0844,0x08E4,0x0C0C,0x0E1C}, // E
    {0x0000,0x0804,0x0FFC,0x0FFC,0x0844,0x00E4,0x000C,0x001C}, // F
    {0x0000,0x03F0,0x07F8,0x0C0C,0x0884,0x0C84,0x078C,0x0F98}, // G
    {0x0000,0x0FFC,0x0FFC,0x0040,0x0040,0x0040,0x0FFC,0x0FFC}, // H
    {0x0000,0x0000,0x0804,0x0FFC,0x0FFC,0x0804,0x0000,0x0000}, // I
    {0x0000,0x0600,0x0E00,0x0800,0x0804,0x0FFC,0x07FC,0x0004}, // J
    {0x0000,0x0804,0x0FFC,0x0FFC,0x0040,0x01F0,0x0FBC,0x0E0C}, // K
    {0x0000,0x0804,0x0FFC,0x0FFC,0x0804,0x0800,0x0C00,0x0E00}, // L
    {0x0000,0x0FFC,0x0FFC,0x0038,0x0070,0x0038,0x0FFC,0x0FFC}, // M
    {0x0000,0x0FFC,0x0FFC,0x0038,0x0070,0x00E0,0x0FFC,0x0FFC}, // N
    {0x0000,0x07F8,0x0FFC,0x0804,0x0804,0x0804,0x0FFC,0x07F8}, // O
    {0x0000,0x0804,0x0FFC,0x0FFC,0x0844,0x0044,0x007C,0x0038}, // P
    {0x0000,0x07F8,0x0FFC,0x0804,0x0E04,0x3C04,0x3FFC,0x27F8}, // Q
    {0x0000,0x0804,0x0FFC,0x0FFC,0x0044,0x01C4,0x0FFC,0x0E38}, // R
    {0x0000,0x0618,0x0E3C,0x0864,0x0844,0x08C4,0x0F9C,0x0718}, // S
    {0x001C,0x000C,0x0804,0x0FFC,0x0FFC,0x0804,0x000C,0x001C}, // T
    {0x0000,0x07FC,0x0FFC,0x0800,0x0800,0x0800,0x0FFC,0x07FC}, // U
    {0x0000,0x01FC,0x03FC,0x0600,0x0C00,0x0600,0x03FC,0x01FC}, // V
    {0x0000,0x03FC,0x0FFC,0x0E00,0x0380,0x0E00,0x0FFC,0x03FC}, // W
    {0x0000,0x0C0C,0x0F3C,0x03F0,0x01E0,0x03F0,0x0F3C,0x0C0C}, // X
    {0x000C,0x001C,0x0830,0x0FE0,0x0FE0,0x0830,0x001C,0x000C}, // Y
    {0x0000,0x0E1C,0x0F0C,0x0984,0x08C4,0x0864,0x0C3C,0x0E1C}, // Z
    {0x0000,0x0000,0x0FFC,0x0FFC,0x0804,0x0804,0x0000,0x0000}, // [
    {0x0000,0x0018,0x0030,0x0060,0x00C0,0x0180,0x0300,0x0600}, // backslash
    {0x0000,0x0000,0x0804,0x0804,0x0FFC,0x0FFC,0x0000,0x0000}, // ]
    {0x0000,0x0008,0x000C,0x0006,0x0003,0x0006,0x000C,0x0008}, // ^
    {0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000}, // _
    {0x0000,0x0000,0x0000,0x0006,0x000E,0x0008,0x0000,0x0000}, // `
    {0x0000,0x0640,0x0F20,0x0920,0x0920,0x07E0,0x0FC0,0x0800}, // a
    {0x0000,0x0004,0x0FFC,0x0FFC,0x0820,0x0860,0x0FC0,0x0780}, // b
    {0x0000,0x07C0,0x0FE0,0x0820,0x0820,0x0820,0x0C60,0x0440}, // c
    {0x0000,0x0780,0x0FC0,0x0860,0x0824,0x07FC,0x0FFC,0x0800}, // d
    {0x0000,0x07C0,0x0FE0,0x08A0,0x08A0,0x08A0,0x0CE0,0x04C0}, // e
    {0x0000,0x0840,0x0FF8,0x0FFC,0x0844,0x004C,0x0018,0x0000}, // f
    {0x0000,0x13C0,0x37E0,0x2420,0x2420,0x3FC0,0x1FE0,0x0020}, // g
    {0x0000,0x0804,0x0FFC,0x0FFC,0x0040,0x0020,0x0FE0,0x0FC0}, // h
    {0x0000,0x0000,0x0000,0x0820,0x0FEC,0x0FEC,0x0800,0x0000}, // i
    {0x0000,0x1000,0x3000,0x2000,0x2020,0x3FEC,0x1FEC,0x0000}, // j
    {0x0000,0x0804,0x0FFC,0x0FFC,0x0180,0x03C0,0x0E60,0x0C20}, // k
    {0x0000,0x0000,0x0000,0x0804,0x0FFC,0x0FFC,0x0800,0x0000}, // l
    {0x0000,0x0FE0,0x0FE0,0x0040,0x0FE0,0x0060,0x0FE0,0x0FC0}, // m
    {0x0000,0x0020,0x0FE0,0x0FC0,0x0020,0x0020,0x0FE0,0x0FC0}, // n
    {0x0000,0x07C0,0x0FE0,0x0820,0x0820,0x0820,0x0FE0,0x07C0}, // o
    {0x0000,0x2020,0x3FE0,0x3FC0,0x2420,0x0420,0x07E0,0x03C0}, // p
    {0x0000,0x03C0,0x07E0,0x0420,0x2420,0x3FC0,0x3FE0,0x2020}, // q
    {0x0000,0x0820,0x0FE0,0x0FC0,0x0860,0x0020,0x00E0,0x00C0}, // r
    {0x0000,0x0440,0x0CE0,0x08A0,0x09A0,0x0920,0x0F60,0x0640}, // s
    {0x0000,0x0020,0x0020,0x07F8,0x0FFC,0x0820,0x0C20,0x0400}, // t
    {0x0000,0x07E0,0x0FE0,0x0800,0x0800,0x07E0,0x0FE0,0x0800}, // u
    {0x0000,0x01E0,0x03E0,0x0600,0x0C00,0x0600,0x03E0,0x01E0}, // v
    {0x0000,0x03E0,0x0FE0,0x0E00,0x0380,0x0E00,0x0FE0,0x03E0}, // w
    {0x0000,0x0820,0x0C60,0x07C0,0x0380,0x07C0,0x0C60,0x0820}, // x
    {0x0000,0x03E0,0x27E0,0x2400,0x2400,0x3400,0x1FE0,0x0FE0}, // y
    {0x0000,0x0C60,0x0E60,0x0B20,0x09A0,0x08E0,0x0C60,0x0C20}, // z
    {0x0000,0x0040,0x0040,0x07F8,0x0FBC,0x0804,0x0804,0x0000}, // {
    {0x0000,0x0000,0x0000,0x0FBC,0x0FBC,0x0000,0x0000,0x0000}, // |
    {0x0000,0x0804,0x0804,0x0FBC,0x07F8,0x0040,0x0040,0x0000}, // }
    {0x0000,0x0008,0x000C,0x0004,0x000C,0x0008,0x000C,0x0004}, // ~
    {0x0000,0x3FF8,0x1FF0,0x0FE0,0x07C0,0x0380,0x0100,0x0000}, // ▶ (char 127) '\x7F'
};

// ─── 16x32 Terminus Font ─────────────────────────────────────────────────────
// Digits only. Column-encoded: each column is uint32_t, bit 31 = top row.
#define FONT16_W 16
#define FONT16_H 32

static const uint32_t font16x32[][16] = {
    {0x00000000,0x00FFFF00,0x01FFFF80,0x03FFFFC0,0x038071C0,0x0300E0C0,0x0301C0C0,0x030380C0,0x030700C0,0x030E00C0,0x039C01C0,0x03FFFFC0,0x01FFFF80,0x00FFFF00,0x00000000,0x00000000}, // '0'
    {0x00000000,0x00000000,0x00000000,0x006000C0,0x00E000C0,0x01E000C0,0x03FFFFC0,0x03FFFFC0,0x03FFFFC0,0x000000C0,0x000000C0,0x000000C0,0x00000000,0x00000000,0x00000000,0x00000000}, // '1'
    {0x00000000,0x00F801C0,0x01F803C0,0x03F807C0,0x03800EC0,0x03001CC0,0x030038C0,0x030070C0,0x0300E0C0,0x0301C0C0,0x038380C0,0x03FF00C0,0x01FE00C0,0x00FC00C0,0x00000000,0x00000000}, // '2'
    {0x00000000,0x00E00700,0x01E00780,0x03E007C0,0x038181C0,0x030180C0,0x030180C0,0x030180C0,0x030180C0,0x030180C0,0x0383C1C0,0x03FFFFC0,0x01FFFF80,0x00FE7F00,0x00000000,0x00000000}, // '3'
    {0x00000000,0x0000F800,0x0001F800,0x0003F800,0x00071800,0x000E1800,0x001C1800,0x00381800,0x00701800,0x00E01800,0x01C01800,0x03FFFFC0,0x03FFFFC0,0x03FFFFC0,0x00000000,0x00000000}, // '4'
    {0x00000000,0x03FF0300,0x03FF0380,0x03FF03C0,0x030301C0,0x030300C0,0x030300C0,0x030300C0,0x030300C0,0x030300C0,0x030380C0,0x0303FFC0,0x0301FF80,0x0300FF00,0x00000000,0x00000000}, // '5'
    {0x00000000,0x00FFFF00,0x01FFFF80,0x03FFFFC0,0x038301C0,0x030300C0,0x030300C0,0x030300C0,0x030300C0,0x030300C0,0x030381C0,0x0303FFC0,0x0301FF80,0x0000FF00,0x00000000,0x00000000}, // '6'
    {0x00000000,0x03F00000,0x03F00000,0x03F00000,0x03000000,0x03000000,0x03001FC0,0x03007FC0,0x0301FFC0,0x0307E000,0x031F8000,0x03FE0000,0x03F80000,0x03E00000,0x00000000,0x00000000}, // '7'
    {0x00000000,0x00FE7F00,0x01FFFF80,0x03FFFFC0,0x0383C1C0,0x030180C0,0x030180C0,0x030180C0,0x030180C0,0x030180C0,0x0383C1C0,0x03FFFFC0,0x01FFFF80,0x00FE7F00,0x00000000,0x00000000}, // '8'
    {0x00000000,0x00FF0000,0x01FF80C0,0x03FFC0C0,0x0381C0C0,0x0300C0C0,0x0300C0C0,0x0300C0C0,0x0300C0C0,0x0300C0C0,0x0380C1C0,0x03FFFFC0,0x01FFFF80,0x00FFFF00,0x00000000,0x00000000}, // '9'
    {0x00000000,0x00018000,0x00018000,0x00018000,0x00018000,0x00018000,0x00018000,0x00018000,0x00018000,0x00018000,0x00018000,0x00018000,0x00018000,0x00018000,0x00000000,0x00000000}, // '-'
    {0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000}, // ' '
};

// ─── Strip buffers (double-buffered) ─────────────────────────────────────────
static uint16_t s_strip[2][LCD_WIDTH * RENDER_BUF_LINES];
static int s_buf_idx = 0;
static uint16_t s_bg_color;

// ─── Clipping ────────────────────────────────────────────────────────────────
static bool s_clip_enabled = false;
static int  s_clip_cx, s_clip_cy, s_clip_r_sq;

// ─── Primitive list ──────────────────────────────────────────────────────────
typedef enum {
    PRIM_RECT_FILL, PRIM_RECT, PRIM_CIRCLE_FILL, PRIM_CIRCLE,
    PRIM_HLINE, PRIM_VLINE, PRIM_RRECT, PRIM_RRECT_FILL, PRIM_CHAR
} prim_type_t;

typedef struct {
    prim_type_t type;
    int16_t x, y, w, h, r;
    uint16_t color;
    char ch;
    uint8_t scale;
    uint8_t font;
} prim_t;

#define MAX_PRIMS 256
static prim_t s_prims[MAX_PRIMS];
static int s_prim_count = 0;

// Icon entries (rendered separately, not as prims)
typedef struct {
    const uint8_t *data;
    int16_t x, y, w, h;
    uint16_t color;
} icon_t;

#define MAX_ICONS 8
static icon_t s_icons[MAX_ICONS];
static int s_icon_count = 0;

static icon_t s_bg_icon;
static bool s_bg_icon_set = false;

// ─── Helpers ─────────────────────────────────────────────────────────────────

static inline bool clip_test(int px, int py) {
    if (!s_clip_enabled) return true;
    int dx = px - s_clip_cx;
    int dy = py - s_clip_cy;
    return (dx * dx + dy * dy) <= s_clip_r_sq;
}

static inline void put_pixel(uint16_t *buf, int strip_y, int px, int py, uint16_t color) {
    if (px < 0 || px >= LCD_WIDTH) return;
    if (py < strip_y || py >= strip_y + RENDER_BUF_LINES) return;
    if (!clip_test(px, py)) return;
    buf[(py - strip_y) * LCD_WIDTH + px] = color;
}

static void add_prim(prim_t p) {
    if (s_prim_count < MAX_PRIMS) s_prims[s_prim_count++] = p;
}

// ─── Public API ──────────────────────────────────────────────────────────────

void ltapp_render_init(void) {
    s_prim_count = 0;
    s_icon_count = 0;
}

void ltapp_render_begin(uint16_t bg_color) {
    s_bg_color = bg_color;
    s_prim_count = 0;
    s_icon_count = 0;
    s_bg_icon_set = false;
    s_clip_enabled = false;
}

void ltapp_render_set_clip_circle(int cx, int cy, int r) {
    s_clip_enabled = true;
    s_clip_cx = cx;
    s_clip_cy = cy;
    s_clip_r_sq = r * r;
}

void ltapp_render_clear_clip(void) {
    s_clip_enabled = false;
}

void ltapp_render_rect(int x, int y, int w, int h, uint16_t color) {
    add_prim((prim_t){.type = PRIM_RECT, .x = x, .y = y, .w = w, .h = h, .color = color});
}

void ltapp_render_rect_fill(int x, int y, int w, int h, uint16_t color) {
    add_prim((prim_t){.type = PRIM_RECT_FILL, .x = x, .y = y, .w = w, .h = h, .color = color});
}

void ltapp_render_rounded_rect(int x, int y, int w, int h, int r, uint16_t color) {
    add_prim((prim_t){.type = PRIM_RRECT, .x = x, .y = y, .w = w, .h = h, .r = r, .color = color});
}

void ltapp_render_rounded_rect_fill(int x, int y, int w, int h, int r, uint16_t color) {
    add_prim((prim_t){.type = PRIM_RRECT_FILL, .x = x, .y = y, .w = w, .h = h, .r = r, .color = color});
}

void ltapp_render_circle(int cx, int cy, int r, uint16_t color) {
    add_prim((prim_t){.type = PRIM_CIRCLE, .x = cx, .y = cy, .r = r, .color = color});
}

void ltapp_render_circle_fill(int cx, int cy, int r, uint16_t color) {
    add_prim((prim_t){.type = PRIM_CIRCLE_FILL, .x = cx, .y = cy, .r = r, .color = color});
}

void ltapp_render_hline(int x, int y, int w, uint16_t color) {
    add_prim((prim_t){.type = PRIM_HLINE, .x = x, .y = y, .w = w, .color = color});
}

void ltapp_render_vline(int x, int y, int h, uint16_t color) {
    add_prim((prim_t){.type = PRIM_VLINE, .x = x, .y = y, .h = h, .color = color});
}

void ltapp_render_hline_dashed(int x, int y, int w, int dash, int gap, uint16_t color) {
    int step = dash + gap;
    for (int i = 0; i < w; i += step) {
        int len = (i + dash > w) ? w - i : dash;
        add_prim((prim_t){.type = PRIM_HLINE, .x = x + i, .y = y, .w = len, .color = color});
    }
}

void ltapp_render_vline_dashed(int x, int y, int h, int dash, int gap, uint16_t color) {
    int step = dash + gap;
    for (int i = 0; i < h; i += step) {
        int len = (i + dash > h) ? h - i : dash;
        add_prim((prim_t){.type = PRIM_VLINE, .x = x, .y = y + i, .h = len, .color = color});
    }
}

void ltapp_render_char(int x, int y, char c, uint16_t color, int font, int scale) {
    add_prim((prim_t){.type = PRIM_CHAR, .x = x, .y = y, .color = color, .ch = c, .scale = scale, .font = font});
}

void ltapp_render_string(int x, int y, const char *str, uint16_t color, int font, int scale) {
    int cw;
    if (font == FONT_XLARGE) cw = FONT16_W;
    else if (font == FONT_BASE) cw = FONT8_W;
    else cw = FONT5_W;
    while (*str) {
        ltapp_render_char(x, y, *str, color, font, scale);
        x += cw * scale + 1;
        str++;
    }
}

void ltapp_render_string_centered(int y, const char *str, uint16_t color, int font, int scale) {
    int w = ltapp_render_text_width(str, font, scale);
    ltapp_render_string((LCD_WIDTH - w) / 2, y, str, color, font, scale);
}

void ltapp_render_number(int x, int y, int num, uint16_t color, int font, int scale) {
    char buf[12];
    snprintf(buf, sizeof(buf), "%d", num);
    ltapp_render_string(x, y, buf, color, font, scale);
}

void ltapp_render_number_centered(int y, int num, uint16_t color, int font, int scale) {
    char buf[12];
    snprintf(buf, sizeof(buf), "%d", num);
    ltapp_render_string_centered(y, buf, color, font, scale);
}

void ltapp_render_icon(int x, int y, const uint8_t *bitmap, int w, int h, uint16_t color) {
    if (s_icon_count >= MAX_ICONS) return;
    s_icons[s_icon_count++] = (icon_t){.data = bitmap, .x = x, .y = y, .w = w, .h = h, .color = color};
}

void ltapp_render_icon_bg(int x, int y, const uint8_t *bitmap, int w, int h, uint16_t color) {
    s_bg_icon = (icon_t){.data = bitmap, .x = x, .y = y, .w = w, .h = h, .color = color};
    s_bg_icon_set = true;
}

int ltapp_render_text_width(const char *str, int font, int scale) {
    int len = strlen(str);
    if (len == 0) return 0;
    int cw;
    if (font == FONT_XLARGE) cw = FONT16_W;
    else if (font == FONT_BASE) cw = FONT8_W;
    else cw = FONT5_W;
    return len * (cw * scale + 1) - 1;
}

int ltapp_render_number_width(int num, int font, int scale) {
    char buf[12];
    snprintf(buf, sizeof(buf), "%d", num);
    return ltapp_render_text_width(buf, font, scale);
}

int ltapp_render_text_height(int font, int scale) {
    if (font == FONT_XLARGE) return 32 * scale;
    if (font == FONT_BASE) return 16 * scale;
    return 7 * scale;
}

int ltapp_render_number_height(int font, int scale) {
    return ltapp_render_text_height(font, scale);
}

int ltapp_circle_width_at(int r, int y_from_center) {
    if (abs(y_from_center) >= r) return 0;
    return 2 * (int)sqrtf((float)(r * r - y_from_center * y_from_center));
}

int ltapp_circle_height_at(int r, int x_from_center) {
    if (abs(x_from_center) >= r) return 0;
    return 2 * (int)sqrtf((float)(r * r - x_from_center * x_from_center));
}

// ─── Rasterizer ──────────────────────────────────────────────────────────────

static void rast_hline(uint16_t *buf, int strip_y, int x0, int x1, int y, uint16_t color) {
    if (y < strip_y || y >= strip_y + RENDER_BUF_LINES) return;
    if (x0 < 0) x0 = 0;
    if (x1 > LCD_WIDTH) x1 = LCD_WIDTH;
    uint16_t *line = &buf[(y - strip_y) * LCD_WIDTH];
    if (s_clip_enabled) {
        for (int x = x0; x < x1; x++) {
            if (clip_test(x, y)) line[x] = color;
        }
    } else {
        for (int x = x0; x < x1; x++) line[x] = color;
    }
}

static void rast_circle_fill(uint16_t *buf, int strip_y, int cx, int cy, int r, uint16_t color) {
    int sy0 = strip_y;
    int sy1 = strip_y + RENDER_BUF_LINES;
    int y0 = cy - r < sy0 ? sy0 : cy - r;
    int y1 = cy + r >= sy1 ? sy1 - 1 : cy + r;
    int r_sq = r * r;
    for (int row = y0; row <= y1; row++) {
        int dy = row - cy;
        int dx = (int)sqrtf((float)(r_sq - dy * dy));
        rast_hline(buf, strip_y, cx - dx, cx + dx + 1, row, color);
    }
}

static void rast_rounded_rect_fill(uint16_t *buf, int strip_y, int x, int y, int w, int h, int r, uint16_t color) {
    int sy0 = strip_y;
    int sy1 = strip_y + RENDER_BUF_LINES;
    int y0 = y < sy0 ? sy0 : y;
    int y1 = (y + h) > sy1 ? sy1 : (y + h);

    for (int row = y0; row < y1; row++) {
        int lx = x, rx = x + w;
        // Top corners
        if (row < y + r) {
            int dy = (y + r) - row;
            int dx = r - (int)sqrtf((float)(r * r - dy * dy));
            lx += dx;
            rx -= dx;
        }
        // Bottom corners
        if (row >= y + h - r) {
            int dy = row - (y + h - r - 1);
            int dx = r - (int)sqrtf((float)(r * r - dy * dy));
            lx += dx;
            rx -= dx;
        }
        rast_hline(buf, strip_y, lx, rx, row, color);
    }
}

static void rast_rounded_rect(uint16_t *buf, int strip_y, int x, int y, int w, int h, int r, uint16_t color) {
    // Top/bottom edges (between corners)
    rast_hline(buf, strip_y, x + r, x + w - r, y, color);
    rast_hline(buf, strip_y, x + r, x + w - r, y + h - 1, color);
    // Left/right edges (between corners)
    int sy0 = strip_y;
    int sy1 = strip_y + RENDER_BUF_LINES;
    for (int row = y + r; row < y + h - r; row++) {
        if (row < sy0 || row >= sy1) continue;
        put_pixel(buf, strip_y, x, row, color);
        put_pixel(buf, strip_y, x + w - 1, row, color);
    }
    // Corner arcs (Bresenham quarter-circles)
    int cx, cy_c, px = r, py_c = 0, err = 1 - r;
    while (px >= py_c) {
        // Top-left
        cx = x + r; cy_c = y + r;
        put_pixel(buf, strip_y, cx - px, cy_c - py_c, color);
        put_pixel(buf, strip_y, cx - py_c, cy_c - px, color);
        // Top-right
        cx = x + w - 1 - r; cy_c = y + r;
        put_pixel(buf, strip_y, cx + px, cy_c - py_c, color);
        put_pixel(buf, strip_y, cx + py_c, cy_c - px, color);
        // Bottom-left
        cx = x + r; cy_c = y + h - 1 - r;
        put_pixel(buf, strip_y, cx - px, cy_c + py_c, color);
        put_pixel(buf, strip_y, cx - py_c, cy_c + px, color);
        // Bottom-right
        cx = x + w - 1 - r; cy_c = y + h - 1 - r;
        put_pixel(buf, strip_y, cx + px, cy_c + py_c, color);
        put_pixel(buf, strip_y, cx + py_c, cy_c + px, color);

        py_c++;
        if (err < 0) {
            err += 2 * py_c + 1;
        } else {
            px--;
            err += 2 * (py_c - px) + 1;
        }
    }
}

static void rast_char(uint16_t *buf, int strip_y, const prim_t *p) {
    if (p->ch < 32 || p->ch > 127) return;
    int idx = p->ch - 32;
    int scale = p->scale;

    if (p->font == FONT_XLARGE) {
        // Only digits, minus, space
        int fi;
        if (p->ch >= '0' && p->ch <= '9') fi = p->ch - '0';
        else if (p->ch == '-') fi = 10;
        else fi = 11; // space / fallback
        const uint32_t *glyph = font16x32[fi];
        for (int col = 0; col < FONT16_W; col++) {
            uint32_t bits = glyph[col];
            for (int row = 0; row < FONT16_H; row++) {
                if (!(bits & (1u << (31 - row)))) continue;
                int px_base = p->x + col * scale;
                int py_base = p->y + row * scale;
                for (int sy = 0; sy < scale; sy++) {
                    for (int sx = 0; sx < scale; sx++) {
                        put_pixel(buf, strip_y, px_base + sx, py_base + sy, p->color);
                    }
                }
            }
        }
    } else if (p->font == FONT_BASE) {
        const uint16_t *glyph = font8x16[idx];
        for (int col = 0; col < FONT8_W; col++) {
            uint16_t bits = glyph[col];
            for (int row = 0; row < FONT8_H; row++) {
                if (!(bits & (1 << row))) continue;
                int px_base = p->x + col * scale;
                int py_base = p->y + row * scale;
                for (int sy = 0; sy < scale; sy++) {
                    for (int sx = 0; sx < scale; sx++) {
                        put_pixel(buf, strip_y, px_base + sx, py_base + sy, p->color);
                    }
                }
            }
        }
    } else {
        const uint8_t *glyph = font5x7[idx];
        for (int col = 0; col < FONT5_W; col++) {
            uint8_t bits = glyph[col];
            for (int row = 0; row < FONT5_H; row++) {
                if (!(bits & (1 << row))) continue;
                int px_base = p->x + col * scale;
                int py_base = p->y + row * scale;
                for (int sy = 0; sy < scale; sy++) {
                    for (int sx = 0; sx < scale; sx++) {
                        put_pixel(buf, strip_y, px_base + sx, py_base + sy, p->color);
                    }
                }
            }
        }
    }
}

static void rasterize_prim(const prim_t *p, int strip_y, uint16_t *buf) {
    int sy0 = strip_y;
    int sy1 = strip_y + RENDER_BUF_LINES;

    switch (p->type) {
    case PRIM_RECT_FILL: {
        int y0 = p->y < sy0 ? sy0 : p->y;
        int y1 = (p->y + p->h) > sy1 ? sy1 : (p->y + p->h);
        for (int row = y0; row < y1; row++) {
            rast_hline(buf, strip_y, p->x, p->x + p->w, row, p->color);
        }
        break;
    }
    case PRIM_RECT: {
        rast_hline(buf, strip_y, p->x, p->x + p->w, p->y, p->color);
        rast_hline(buf, strip_y, p->x, p->x + p->w, p->y + p->h - 1, p->color);
        int y0 = p->y + 1 < sy0 ? sy0 : p->y + 1;
        int y1 = (p->y + p->h - 1) > sy1 ? sy1 : (p->y + p->h - 1);
        for (int row = y0; row < y1; row++) {
            put_pixel(buf, strip_y, p->x, row, p->color);
            put_pixel(buf, strip_y, p->x + p->w - 1, row, p->color);
        }
        break;
    }
    case PRIM_CIRCLE_FILL:
        rast_circle_fill(buf, strip_y, p->x, p->y, p->r, p->color);
        break;
    case PRIM_CIRCLE: {
        // Bresenham circle outline
        int cx = p->x, cy = p->y, r = p->r;
        int px = r, py = 0, err = 1 - r;
        while (px >= py) {
            put_pixel(buf, strip_y, cx + px, cy + py, p->color);
            put_pixel(buf, strip_y, cx - px, cy + py, p->color);
            put_pixel(buf, strip_y, cx + px, cy - py, p->color);
            put_pixel(buf, strip_y, cx - px, cy - py, p->color);
            put_pixel(buf, strip_y, cx + py, cy + px, p->color);
            put_pixel(buf, strip_y, cx - py, cy + px, p->color);
            put_pixel(buf, strip_y, cx + py, cy - px, p->color);
            put_pixel(buf, strip_y, cx - py, cy - px, p->color);
            py++;
            if (err < 0) {
                err += 2 * py + 1;
            } else {
                px--;
                err += 2 * (py - px) + 1;
            }
        }
        break;
    }
    case PRIM_HLINE:
        rast_hline(buf, strip_y, p->x, p->x + p->w, p->y, p->color);
        break;
    case PRIM_VLINE: {
        int y0 = p->y < sy0 ? sy0 : p->y;
        int y1 = (p->y + p->h) > sy1 ? sy1 : (p->y + p->h);
        for (int row = y0; row < y1; row++) {
            put_pixel(buf, strip_y, p->x, row, p->color);
        }
        break;
    }
    case PRIM_RRECT_FILL:
        rast_rounded_rect_fill(buf, strip_y, p->x, p->y, p->w, p->h, p->r, p->color);
        break;
    case PRIM_RRECT:
        rast_rounded_rect(buf, strip_y, p->x, p->y, p->w, p->h, p->r, p->color);
        break;
    case PRIM_CHAR:
        rast_char(buf, strip_y, p);
        break;
    }
}

void ltapp_render_flush(void) {
    for (int strip_y = 0; strip_y < LCD_HEIGHT; strip_y += RENDER_BUF_LINES) {
        int lines = (strip_y + RENDER_BUF_LINES > LCD_HEIGHT) ? (LCD_HEIGHT - strip_y) : RENDER_BUF_LINES;
        uint16_t *buf = s_strip[s_buf_idx];

        // Fill background (with clip)
        for (int row = 0; row < lines; row++) {
            for (int col = 0; col < LCD_WIDTH; col++) {
                buf[row * LCD_WIDTH + col] = s_bg_color;
            }
        }

        // Rasterize background icon (underneath primitives)
        if (s_bg_icon_set) {
            icon_t *ic = &s_bg_icon;
            int stride = (ic->w + 7) / 8;
            for (int row = 0; row < ic->h; row++) {
                int py = ic->y + row;
                if (py < strip_y || py >= strip_y + lines) continue;
                for (int col = 0; col < ic->w; col++) {
                    if (ic->data[row * stride + col / 8] & (0x80 >> (col % 8))) {
                        int px = ic->x + col;
                        if (px >= 0 && px < LCD_WIDTH && clip_test(px, py))
                            buf[(py - strip_y) * LCD_WIDTH + px] = ic->color;
                    }
                }
            }
        }

        // Rasterize all primitives
        for (int i = 0; i < s_prim_count; i++) {
            rasterize_prim(&s_prims[i], strip_y, buf);
        }

        // Rasterize icons
        for (int i = 0; i < s_icon_count; i++) {
            icon_t *ic = &s_icons[i];
            int stride = (ic->w + 7) / 8;
            for (int row = 0; row < ic->h; row++) {
                int py = ic->y + row;
                if (py < strip_y || py >= strip_y + lines) continue;
                for (int col = 0; col < ic->w; col++) {
                    if (ic->data[row * stride + col / 8] & (0x80 >> (col % 8))) {
                        int px = ic->x + col;
                        if (px >= 0 && px < LCD_WIDTH && clip_test(px, py))
                            buf[(py - strip_y) * LCD_WIDTH + px] = ic->color;
                    }
                }
            }
        }

        ltapp_display_flush_area(0, strip_y, LCD_WIDTH, lines, buf);
        s_buf_idx ^= 1;
    }
}
