# JV Life Tap

ESP32-C3 wireless life counter for Magic: The Gathering (Commander/EDH).

Each player carries a device. They communicate over **ESP-NOW** (peer-to-peer, no router needed, ~200m range).

## Hardware

- **Board:** ESP32-2424S012 (ESP32-C3-MINI-1U)
- **Display:** 1.28" round 240×240 GC9A01 (SPI)
- **Touch:** CST816D (I2C, interrupt-driven)
- **Battery:** LiPo via onboard charger

## Features

- Room creation & discovery (beacon-based)
- Solo mode (offline, configurable opponents)
- Up to 6 players per room
- Configurable starting life (40/30/20)
- Real-time life broadcast to all peers
- Commander damage tracking per opponent
- Poison counter (0–10)
- Dice rolling (D4/D6/D8/D12/D20)
- Coin flip with animation
- Player name editing via T9-style keyboard
- Round 240×240 display with circular clipping
- Interrupt-driven touch with gesture recognition

## Architecture

```
main/
  main.c           – App entry, hardware init
  game_task.c/h    – Input handling, animations, game flow
  ui.c/h           – Screen render functions
  renderer.c/h     – Strip-based framebuffer renderer (3 fonts, clipping)
  keyboard.c/h     – T9-style on-screen keyboard overlay
  display.c/h      – GC9A01 SPI display driver (DMA)
  touch_input.c/h  – CST816D driver (I2C, interrupt + gesture detection)
  icons.h          – 1bpp icon bitmaps
  coin_anim.h      – Coin flip animation frames

components/jvlt_core/
  jvlt.h           – Public API (single header)
  jvlt_init.c      – Entry point, task creation, queues
  jvlt_nav.c       – Screen state machine
  jvlt_match.c     – Match session logic
  jvlt_dice.c      – Dice rolling
  jvlt_coin.c      – Coin flip
  jvlt_settings.c  – NVS persistence (name, brightness)
  jvlt_net.c       – Room/beacon/multiplayer
  jvlt_comm.c      – ESP-NOW transport
```

## FreeRTOS Tasks

| Task   | Priority | Role                                            |
| ------ | -------- | ----------------------------------------------- |
| game   | 5        | Process input events + network packets          |
| touch  | 5        | Wait for INT, read CST816D, emit gesture events |
| render | 3        | Redraw display when state changes (~30fps)      |

## Touch Controls

| Screen              | Gesture               | Action                        |
| ------------------- | --------------------- | ----------------------------- |
| HOME                | Tap solo item         | Solo setup                    |
| HOME                | Tap net item          | Network lobby                 |
| HOME                | Tap extras            | Extras menu                   |
| HOME                | Tap bottom            | Settings                      |
| SOLO SETUP          | Tap `<`/`>` life      | Cycle starting life           |
| SOLO SETUP          | Tap `<`/`>` opponents | Cycle opponent count          |
| SOLO SETUP          | Tap start             | Begin match                   |
| SOLO SETUP          | Swipe right           | Back to home                  |
| MATCH               | Tap left half         | Life −1                       |
| MATCH               | Tap right half        | Life +1                       |
| MATCH               | Long press left/right | Life −5/+5                    |
| MATCH               | Tap damage circle     | Commander damage screen       |
| MATCH               | Tap extras circle     | Extras menu                   |
| MATCH               | Swipe up              | Match settings                |
| MATCH SETTINGS      | Tap reset             | Confirm reset                 |
| MATCH SETTINGS      | Tap leave             | Confirm leave                 |
| MATCH SETTINGS      | Swipe left            | Brightness settings           |
| MATCH SETTINGS      | Swipe right           | Back to match                 |
| CMD DAMAGE          | Tap opponent          | View opponent's damage        |
| CMD DAMAGE          | Swipe right           | Back to match                 |
| CMD DAMAGE          | Swipe left            | Poison counter                |
| CMD DAMAGE OPP      | Tap +/−               | Adjust commander damage       |
| CMD DAMAGE OPP      | Tap name              | Rename opponent               |
| CMD DAMAGE OPP      | Swipe right           | Back to damage select         |
| POISON              | Tap +/−               | Adjust poison (0–10)          |
| POISON              | Swipe right           | Back to damage select         |
| EXTRAS              | Tap roll a die        | Dice screen                   |
| EXTRAS              | Tap flip a coin       | Coin screen                   |
| EXTRAS              | Swipe right           | Back (home or match)          |
| DICE                | Tap center            | Roll                          |
| DICE                | Tap `<`/`>`           | Cycle die type                |
| DICE                | Swipe right           | Back to extras                |
| COIN                | Tap                   | Flip                          |
| COIN                | Swipe right           | Back to extras                |
| SETTINGS NAME       | Tap                   | Open keyboard                 |
| SETTINGS NAME       | Swipe right           | Back to home                  |
| SETTINGS NAME       | Swipe left            | Brightness                    |
| SETTINGS BRIGHTNESS | Tap +/−               | Adjust brightness             |
| SETTINGS BRIGHTNESS | Swipe right           | Back (name or match settings) |

## Build

Requires [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/get-started/) v6.x.

```bash
idf.py set-target esp32c3
idf.py build
idf.py -p PORT flash
```
