# Match

Game session logic for MTG life tracking. Supports solo mode (1 player + N opponents) and networked mode.

## Data Structures

```c
#define JVLT_MAX_PLAYERS     6
#define JVLT_MAX_COMMANDERS  2

typedef struct {
    char    name[12];
    int16_t life;
    uint8_t poison;                              // 0–10
    uint8_t num_commanders;                      // 1 or 2
    int16_t cmd_dmg[JVLT_MAX_PLAYERS][JVLT_MAX_COMMANDERS];
} jvlt_player_t;

typedef struct {
    bool    active;
    uint8_t my_index;                            // always 0 in solo
    uint8_t player_count;                        // includes self
    int16_t starting_life;                       // 20, 30, or 40
    uint8_t cmd_sel_opp;                         // currently viewed opponent
    jvlt_player_t players[JVLT_MAX_PLAYERS];
} jvlt_match_t;
```

## API

### Access

```c
const jvlt_match_t *jvlt_match_get(void);  // NULL if no active match
```

### Setup

```c
void jvlt_match_begin_solo(uint8_t opponents, int16_t starting_life);
void jvlt_match_end(void);
```

`begin_solo` initializes a match with 1 player + N opponents (1–5). All players start at the given life total. Opponent names default to "opp_1", "opp_2", etc.

### Pre-match configuration

```c
void jvlt_match_set_starting_life(int16_t life);
void jvlt_match_cycle_starting_life(int delta);       // cycles through {20, 30, 40}
const int16_t *jvlt_match_peek_starting_life(int8_t dir); // NULL at boundary

void jvlt_match_set_opponents(uint8_t count);         // 1–5
const int16_t *jvlt_match_peek_opponents(int8_t dir); // NULL at boundary
```

### In-match actions

```c
void jvlt_match_update_life(int16_t delta);           // +1/-1 to own life
void jvlt_match_add_cmd_dmg(uint8_t opp, uint8_t commander_idx, int16_t delta);
void jvlt_match_add_poison(int16_t delta);            // clamped 0–10
void jvlt_match_set_num_commanders(uint8_t opp, uint8_t count);
void jvlt_match_nav_opponent(int delta);              // cycle cmd_sel_opp
void jvlt_match_select_opponent(uint8_t opp_idx);     // set cmd_sel_opp directly
void jvlt_match_rename_player(uint8_t player_idx, const char *name);
void jvlt_match_reset_counters(void);                 // reset life/poison/cmd_dmg, keep match active
```

## Behavior

- Life is unbounded (can go negative).
- Poison is `uint8_t`, clamped to 0–10.
- Commander damage is tracked per opponent × per commander slot.
- `cmd_sel_opp` wraps around the opponent list.
- All state-changing functions mark dirty automatically.
