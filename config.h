#pragma once
// SPDX-License-Identifier: GPL-3.0-or-later AND MIT

#include <stdint.h>

// =============================================================================
// 1. USER CONFIGURATION OVERRIDES & PARAMETERS (SUNAKU GLORIOUS ENGRAMMER)
// =============================================================================

#define OPERATING_SYSTEM 'M'  // Boots into macOS mode by default ('M'), runtime switchable to Windows/Linux ('W'/'L')
#define DIFFICULTY_LEVEL  4   // Fast, tight timing profile (translates to ~160-180ms hold timeouts)
#define ENABLE_MOUSE_KEYS     // Enable Kaleidoscope-MouseKeys / HID pointing
//#define ENFORCE_BILATERAL   // Disabled: Do not enforce strict cross-hand separation
#define SPACE_FORGIVENESS     // Allow lingering taps on Space (retro-tap / hold-tap forgiveness)
#define THUMB_FORGIVENESS     // Allow lingering taps on thumb keys
#define SHIFT_FORGIVENESS     // Forgiving roll into Shift mod-taps
#define NATURAL_SCROLLING     // Invert mouse scroll wheel directions to match macOS natural scrolling

// Dygma Defy Wireless, Bluetooth & Bazecor Device Settings
#define TRUE_SLEEP_ENABLED                  // Deep power-down of both keyboard sides after idle timeout
#define TRUE_SLEEP_TIMEOUT_S            1800 // 30 minutes (Focus: idleleds.true_sleep_time)
//#define ENERGY_SAVING_MODE                // Auto-dim RGB LEDs when wireless battery < 40% (Focus: wireless.battery.savingMode)
#define HIGHLIGHT_LAYER_CHANGE              // Dim transparent (___) keys on secondary layers (Focus: led.fade)
#define LED_BRIGHTNESS_KEY_WIRED        255 // 100% (Focus: led.brightness)
#define LED_BRIGHTNESS_UG_WIRED         255 // 100% (Focus: led.brightnessUG)
#define LED_BRIGHTNESS_KEY_WIRELESS     102 // 40%  (Focus: led.brightness.wireless)
#define LED_BRIGHTNESS_UG_WIRELESS      64  // 25%  (Focus: led.brightnessUG.wireless)
#define LED_IDLE_TIMEOUT_WIRED_S        600 // 10m  (Focus: idleleds.time_limit)
#define LED_IDLE_TIMEOUT_WIRELESS_S     300 // 5m   (Focus: idleleds.wireless)
#define RF_POWER_LEVEL                  1   // 0=Low, 1=Medium, 2=High (Focus: wireless.rf.power)

// =============================================================================
// 2. TIMING PROFILE DERIVATION (DIFFICULTY_LEVEL -> KALEIDOSCOPE QUKEYS)
// =============================================================================

#if DIFFICULTY_LEVEL == 1
  constexpr uint16_t QUKEYS_HOLD_TIMEOUT_MS       = 250;
  constexpr uint8_t  QUKEYS_OVERLAP_THRESHOLD_PCT = 70;
  constexpr uint16_t QUKEYS_MIN_HOLD_TIME_MS      = 60;
  constexpr uint16_t QUKEYS_MIN_PRIOR_INTERVAL_MS = 125;
#elif DIFFICULTY_LEVEL == 2
  constexpr uint16_t QUKEYS_HOLD_TIMEOUT_MS       = 220;
  constexpr uint8_t  QUKEYS_OVERLAP_THRESHOLD_PCT = 75;
  constexpr uint16_t QUKEYS_MIN_HOLD_TIME_MS      = 55;
  constexpr uint16_t QUKEYS_MIN_PRIOR_INTERVAL_MS = 100;
#elif DIFFICULTY_LEVEL == 3
  constexpr uint16_t QUKEYS_HOLD_TIMEOUT_MS       = 195;
  constexpr uint8_t  QUKEYS_OVERLAP_THRESHOLD_PCT = 78;
  constexpr uint16_t QUKEYS_MIN_HOLD_TIME_MS      = 50;
  constexpr uint16_t QUKEYS_MIN_PRIOR_INTERVAL_MS = 85;
#elif DIFFICULTY_LEVEL == 4
  // Level 4: Fast, tight timing profile (~160-180ms hold timeout)
  constexpr uint16_t QUKEYS_HOLD_TIMEOUT_MS       = 175;
  constexpr uint8_t  QUKEYS_OVERLAP_THRESHOLD_PCT = 80;
  constexpr uint16_t QUKEYS_MIN_HOLD_TIME_MS      = 45;
  constexpr uint16_t QUKEYS_MIN_PRIOR_INTERVAL_MS = 75;
#else
  constexpr uint16_t QUKEYS_HOLD_TIMEOUT_MS       = 155;
  constexpr uint8_t  QUKEYS_OVERLAP_THRESHOLD_PCT = 85;
  constexpr uint16_t QUKEYS_MIN_HOLD_TIME_MS      = 40;
  constexpr uint16_t QUKEYS_MIN_PRIOR_INTERVAL_MS = 60;
#endif

// Forgiveness & Chord Timing Constants
constexpr uint16_t SPACE_RETRO_TAP_WINDOW_MS      = 320; // Lingering Space hold without chord emits Space
constexpr uint16_t THUMB_RETRO_TAP_WINDOW_MS      = 260; // Lingering Thumb hold without chord emits Tap key
constexpr uint16_t SHIFT_FORGIVENESS_WINDOW_MS    = 135; // Fast roll window into index-finger Shift (F / J)
constexpr uint16_t THUMB_CHORD_TIMEOUT_MS         = 45;  // Simultaneous dual-thumb chord detection window
constexpr uint16_t CAPS_WORD_IDLE_TIMEOUT_MS      = 5000;// Auto-timeout for Caps-Word if left idle

// =============================================================================
// 3. LAYER INDICES (MATCHING MOONLANDER / GLORIOUS ENGRAMMER SUITE)
//    (Emoji and Global layers excluded per user specification)
// =============================================================================

enum LayerId : uint8_t {
  LAYER_BASE_MAC = 0,  // [0] Base QWERTY (macOS - CAGS home-row mods)
  LAYER_SYMBOL   = 1,  // [1] Symbol Layer        — Held via Right Thumb LT(1, KC_SPACE)
  LAYER_NUMBER   = 2,  // [2] Number Layer        — Held via Left Thumb  LT(2, KC_DELETE)
  LAYER_CURSOR   = 3,  // [3] Cursor Layer        — Held via Left Thumb  LT(3, KC_BSPC)
  LAYER_MOUSE    = 4,  // [4] Mouse Layer         — Held via Right Thumb LT(4, KC_TAB)
  LAYER_FUNCTION = 5,  // [5] Function / Media    — Held via Left Thumb  LT(5, KC_ESCAPE)
  LAYER_SYS_RGB  = 6,  // [6] System / RGB / Pwr  — Held via Right Thumb LT(6, KC_ENTER)
  LAYER_MAGIC    = 7,  // [7] Magic / Boot / OS   — Held via Outer Lower Thumb MO(7)
  LAYER_GAMING   = 8,  // [8] Gaming Layer        — Toggled via B+LShift or Layer 7 TO(8)
  LAYER_PLAIN    = 9,  // [9] Plain QWERTY (No HRM)— Toggled via N+RShift combo
  LAYER_LOWER    = 10, // [10] Lower / Layer Lock — Held via Inner Thumb LT(10, KC_INSERT)
  LAYER_BASE_WIN = 11, // [11] Base QWERTY (Windows/Linux - GACS home-row mods)
  LAYER_COUNT    = 12
};

// =============================================================================
// 4. CUSTOM MACRO & DUAL-FUNCTION DEFINITIONS
// =============================================================================

enum CustomMacroId : uint8_t {
  // Moonlander ST_MACRO_0 .. ST_MACRO_13 equivalents
  M_ST_MACRO_0 = 0, // ".." (Double period on Symbol Layer)
  M_ST_MACRO_1,     // "```" (Triple backtick on Symbol Layer)
  M_SELECT_ALL,     // ST_MACRO_2 / 5 / 8 / 11: OS-aware Select All (Gui+A on Mac, Ctrl+A on Win/Linux)
  M_SELECT_LINE,    // ST_MACRO_3 / 6 / 9 / 12: Select Line (Shift+Home -> Shift+End)
  M_SELECT_WORD,    // ST_MACRO_4 / 7 / 10 / 13: Select Word (Alt+Shift+Left -> Alt+Shift+Right)

  // Dual-Function Tap/Hold Keys from Moonlander (DUAL_FUNC_0, DUAL_FUNC_1)
  M_DUAL_LPRN_LABK, // DUAL_FUNC_0: Tap '(' [KC_LPRN], Hold '<' [KC_LABK]
  M_DUAL_RPRN_RABK, // DUAL_FUNC_1: Tap ')' [KC_RPRN], Hold '>' [KC_RABK]

  // OS Mode & State Machine Controls
  M_OS_TOGGLE,      // Toggle between macOS (Layer 0: CAGS) and Windows/Linux (Layer 11: GACS) + EEPROM save
  M_OS_MAC,         // Explicitly select macOS (CAGS)
  M_OS_WIN,         // Explicitly select Windows/Linux (GACS)
  M_CAPS_WORD,      // CW_TOGG: Toggle Caps-Word state machine
  M_TO_BASE,        // TO(0): Return to active OS Base Layer (LAYER_BASE_MAC or LAYER_BASE_WIN)
  M_TG_PLAIN,       // TG(9): Toggle Plain QWERTY Layer (no home-row mods)

  // OS-Aware Editing / Navigation Helpers (used on Cursor, Number, Function, Mouse layers)
  M_UNDO,           // LGUI(KC_Z) on Mac / LCTL(KC_Z) on Win
  M_REDO,           // LGUI(LSFT(KC_Z)) on Mac / LCTL(KC_Y) on Win
  M_CUT,            // LGUI(KC_X) on Mac / LCTL(KC_X) on Win
  M_COPY,           // LGUI(KC_C) on Mac / LCTL(KC_C) on Win
  M_PASTE,          // LGUI(KC_V) on Mac / LCTL(KC_V) on Win
  M_FIND,           // LGUI(KC_F) on Mac / LCTL(KC_F) on Win
  M_FIND_NEXT,      // LGUI(KC_G) on Mac / LCTL(KC_G) on Win
  M_FIND_PREV,      // LGUI(LSFT(KC_G)) on Mac / LCTL(LSFT(KC_G)) on Win
  M_FIND_REPLACE,   // LALT(LGUI(KC_F)) on Mac / LCTL(KC_H) on Win
  M_GOTO_LINE,      // LGUI(KC_L) on Mac / LCTL(KC_G) on Win
  M_QUICK_OPEN,     // LGUI(KC_P) on Mac / LCTL(KC_P) on Win
  M_LOCK_SCREEN,    // LCTL(RGUI(KC_Q)) on Mac / LGUI(KC_L) on Win

  // Thumb Chord Actions
  M_ALT_TAB,        // LALT(KC_TAB)
  M_CTRL_TAB,       // LCTL(KC_TAB)
  M_GUI_TAB,        // LGUI(KC_TAB)
  M_HYPER,          // KC_HYPR (Ctrl+Alt+Gui+Shift)
  M_MEH,            // KC_MEH  (Ctrl+Alt+Shift)

  // Dygma Defy Wireless, Bluetooth Multi-Host & Battery Level Controls (Focus 54102..54110)
  M_BAT_LEVEL,      // Show dual-side battery level % on keyboard LED columns (Focus 54102)
  M_BT_PAIR,        // Enter Bluetooth Pairing Mode on active BT channel (Focus 54109)
  M_BT_SEL_1,       // Connect to Bluetooth Host Slot #1 (Focus 54103)
  M_BT_SEL_2,       // Connect to Bluetooth Host Slot #2 (Focus 54104)
  M_BT_SEL_3,       // Connect to Bluetooth Host Slot #3 (Focus 54105)
  M_BT_SEL_4,       // Connect to Bluetooth Host Slot #4 (Focus 54106)
  M_BT_SEL_5,       // Connect to Bluetooth Host Slot #5 (Focus 54107)
  M_RF_MODE,        // Switch to 2.4GHz Wireless RF Neuron Mode (Focus 54108)
  M_WL_TOGGLE       // Toggle between Bluetooth LE & 2.4GHz RF (Focus 54110)
};

// =============================================================================
// 5. PERSISTENT OS MODE STATE (EEPROM STORAGE)
// =============================================================================

constexpr uint8_t EEPROM_MAGIC_BYTE = 0xE6;

struct EngrammerPersistentConfig {
  uint8_t magic;    // Validates initialized EEPROM slice (0xE6)
  char    os_mode;  // 'M' for macOS (CAGS), 'W' or 'L' for Windows/Linux (GACS)
  uint8_t reserved; // Alignment padding
};

// =============================================================================
// 6. NATURAL SCROLLING & MOUSE KEYS RESOLUTION
// =============================================================================

#ifdef ENABLE_MOUSE_KEYS
  #ifdef NATURAL_SCROLLING
    // Invert wheel directions to match macOS natural scrolling
    #define ENGRAMMER_WH_UP    Key_mouseScrollDn
    #define ENGRAMMER_WH_DOWN  Key_mouseScrollUp
    #define ENGRAMMER_WH_LEFT  Key_mouseScrollR
    #define ENGRAMMER_WH_RIGHT Key_mouseScrollL
  #else
    #define ENGRAMMER_WH_UP    Key_mouseScrollUp
    #define ENGRAMMER_WH_DOWN  Key_mouseScrollDn
    #define ENGRAMMER_WH_LEFT  Key_mouseScrollL
    #define ENGRAMMER_WH_RIGHT Key_mouseScrollR
  #endif
  #define ENGRAMMER_MS_UP      Key_mouseUp
  #define ENGRAMMER_MS_DOWN    Key_mouseDn
  #define ENGRAMMER_MS_LEFT    Key_mouseL
  #define ENGRAMMER_MS_RIGHT   Key_mouseR
  #define ENGRAMMER_MS_BTN1    Key_mouseBtnL
  #define ENGRAMMER_MS_BTN2    Key_mouseBtnR
  #define ENGRAMMER_MS_BTN3    Key_mouseBtnM
  #define ENGRAMMER_MS_BTN4    Key_mouseBtnP
  #define ENGRAMMER_MS_BTN5    Key_mouseBtnN
  #define ENGRAMMER_MS_ACCEL0  Key_mouseWarpNW
  #define ENGRAMMER_MS_ACCEL1  Key_mouseWarpNE
  #define ENGRAMMER_MS_ACCEL2  Key_mouseWarpEnd
#else
  #define ENGRAMMER_WH_UP      Key_NoKey
  #define ENGRAMMER_WH_DOWN    Key_NoKey
  #define ENGRAMMER_WH_LEFT    Key_NoKey
  #define ENGRAMMER_WH_RIGHT   Key_NoKey
  #define ENGRAMMER_MS_UP      Key_NoKey
  #define ENGRAMMER_MS_DOWN    Key_NoKey
  #define ENGRAMMER_MS_LEFT    Key_NoKey
  #define ENGRAMMER_MS_RIGHT   Key_NoKey
  #define ENGRAMMER_MS_BTN1    Key_NoKey
  #define ENGRAMMER_MS_BTN2    Key_NoKey
  #define ENGRAMMER_MS_BTN3    Key_NoKey
  #define ENGRAMMER_MS_BTN4    Key_NoKey
  #define ENGRAMMER_MS_BTN5    Key_NoKey
  #define ENGRAMMER_MS_ACCEL0  Key_NoKey
  #define ENGRAMMER_MS_ACCEL1  Key_NoKey
  #define ENGRAMMER_MS_ACCEL2  Key_NoKey
#endif

// =============================================================================
// 7. DYGMA DEFY 70-KEY HARDWARE MATRIX COORDINATES & KEYMAP HELPER
// =============================================================================
//
// Exact Physical Structure of the Dygma Defy (70 Keys = 35 Left + 35 Right):
// - Row 0 (Number Row):      7 keys Left (L00..L06, where L06 is inner col) + 7 keys Right (R00..R06, where R00 is inner col) = 14 keys
// - Row 1 (Top QWERTY Row):  7 keys Left (L10..L16, where L16 is inner col) + 7 keys Right (R10..R16, where R10 is inner col) = 14 keys
// - Row 2 (Home Row):        7 keys Left (L20..L26, where L26 is inner col) + 7 keys Right (R20..R26, where R20 is inner col) = 14 keys
// - Row 3 (Bottom Alpha Row):6 keys Left (L30..L35, no inner col key)       + 6 keys Right (R31..R36, no inner col key)       = 12 keys
//   (Main Finger Grid = 14 + 14 + 14 + 12 = 54 keys total, identical to Moonlander Rows 0..3!)
// - Thumb Cluster (16 keys = 8 Left + 8 Right):
//   * Left Upper Thumb Arc (4 keys: LTU0..LTU3):
//       LTU0 = LT(3, KC_BSPC)   [Tap Backspace, Hold Cursor Layer 3]
//       LTU1 = LT(2, KC_DELETE) [Tap Delete,    Hold Number Layer 2]
//       LTU2 = LT(10, KC_INSERT)[Tap Insert,    Hold Lower Layer 10]
//       LTU3 = LT(5, KC_ESCAPE) [Tap Escape,    Hold Function Layer 5]
//   * Left Lower Thumb Arc (4 keys: LTL0..LTL3) — Moonlander Bottom Row:
//       LTL0 = MO(7)            [Hold Magic/Boot/OS Layer 7]
//       LTL1 = KC_BSLS          [\]
//       LTL2 = DUAL_FUNC_0      [Tap '(', Hold '<']
//       LTL3 = DUAL_FUNC_1      [Tap ')', Hold '>']
//   * Right Upper Thumb Arc (4 keys: RTU3..RTU0, inner to outer):
//       RTU3 = LT(6, KC_ENTER)  [Tap Enter,     Hold Sys/RGB Layer 6]
//       RTU2 = LT(10, KC_INSERT)[Tap Insert,    Hold Lower Layer 10]
//       RTU1 = LT(4, KC_TAB)    [Tap Tab,       Hold Mouse Layer 4]
//       RTU0 = LT(1, KC_SPACE)  [Tap Space,     Hold Symbol Layer 1]
//   * Right Lower Thumb Arc (4 keys: RTL3..RTL0, inner to outer) — Moonlander Bottom Row:
//       RTL3 = KC_LBRC          [ [ ]
//       RTL2 = KC_RBRC          [ ] ]
//       RTL1 = KC_GRAVE         [ ` ]
//       RTL0 = MO(7)            [Hold Magic/Boot/OS Layer 7]

// Home Row Matrix Coordinates (Row 2)
constexpr uint8_t LEFT_HR_ROW      = 2;
constexpr uint8_t LEFT_PINKY_COL   = 1; // 'A'
constexpr uint8_t LEFT_RING_COL    = 2; // 'S'
constexpr uint8_t LEFT_MIDDLE_COL  = 3; // 'D'
constexpr uint8_t LEFT_INDEX_COL   = 4; // 'F'

constexpr uint8_t RIGHT_HR_ROW     = 2;
constexpr uint8_t RIGHT_INDEX_COL  = 11; // 'J'
constexpr uint8_t RIGHT_MIDDLE_COL = 12; // 'K'
constexpr uint8_t RIGHT_RING_COL   = 13; // 'L'
constexpr uint8_t RIGHT_PINKY_COL  = 14; // ';'

// Core Thumb Layer-Tap Matrix Coordinates (Row 4 Cols 0..7 Left / 8..15 Right)
// Matches Dygma Defy hardware matrix (Bazecor geometry-defy.ts THUMBS):
// Left Upper Thumb Arc (LTU0..LTU3 -> defy-t1..t4):
//   LTU0 (Bksp/L3)   = KeyAddr(4, 0)  [defy-t1]
//   LTU1 (Del/L2)    = KeyAddr(4, 1)  [defy-t2]
//   LTU2 (Ins/L10)   = KeyAddr(4, 2)  [defy-t3]
//   LTU3 (Esc/L5)    = KeyAddr(4, 3)  [defy-t4]
// Left Lower Thumb Arc (LTL0..LTL3 -> defy-t5..t8):
//   LTL3 (')' / '>') = KeyAddr(4, 4)  [defy-t8, double-height outer inner-tip key]
//   LTL2 ('(' / '<') = KeyAddr(4, 5)  [defy-t7]
//   LTL1 (\)         = KeyAddr(4, 6)  [defy-t6]
//   LTL0 (MO(7))     = KeyAddr(4, 7)  [defy-t5]
// Right Lower Thumb Arc (RTL0..RTL3 -> defy-tR5..tR8):
//   RTL0 (MO(7))     = KeyAddr(4, 8)  [defy-tR5]
//   RTL1 (`)         = KeyAddr(4, 9)  [defy-tR6]
//   RTL2 (])         = KeyAddr(4, 10) [defy-tR7]
//   RTL3 ([)         = KeyAddr(4, 11) [defy-tR8, double-height outer inner-tip key]
// Right Upper Thumb Arc (RTU3..RTU0 -> defy-tR4..tR1):
//   RTU3 (Enter/L6)  = KeyAddr(4, 12) [defy-tR4]
//   RTU2 (Ins/L10)   = KeyAddr(4, 13) [defy-tR3]
//   RTU1 (Tab/L4)    = KeyAddr(4, 14) [defy-tR2]
//   RTU0 (Space/L1)  = KeyAddr(4, 15) [defy-tR1]

#define ADDR_LT_BSPC   KeyAddr(4, 0)
#define ADDR_LT_DEL    KeyAddr(4, 1)
#define ADDR_LT_INS    KeyAddr(4, 2)
#define ADDR_LT_ESC    KeyAddr(4, 3)
#define ADDR_LTL_RPRN  KeyAddr(4, 4)
#define ADDR_LTL_LPRN  KeyAddr(4, 5)
#define ADDR_LTL_BSLS  KeyAddr(4, 6)
#define ADDR_LTL_MO7   KeyAddr(4, 7)

#define ADDR_RTL_MO7   KeyAddr(4, 8)
#define ADDR_RTL_GRV   KeyAddr(4, 9)
#define ADDR_RTL_RBRC  KeyAddr(4, 10)
#define ADDR_RTL_LBRC  KeyAddr(4, 11)
#define ADDR_RT_ENT    KeyAddr(4, 12)
#define ADDR_RT_INS    KeyAddr(4, 13)
#define ADDR_RT_TAB    KeyAddr(4, 14)
#define ADDR_RT_SPC    KeyAddr(4, 15)

#ifndef KEYMAP_DEFY_70
#define KEYMAP_DEFY_70(                                                                 \
    /* Row 0: 7 Left (Outer + 1..5 + Inner) | 7 Right (Inner + 6..0 + Outer) */         \
    L00, L01, L02, L03, L04, L05, L06,           R00, R01, R02, R03, R04, R05, R06,     \
    /* Row 1: 7 Left (Outer + Q..T + Inner) | 7 Right (Inner + Y..P + Outer) */         \
    L10, L11, L12, L13, L14, L15, L16,           R10, R11, R12, R13, R14, R15, R16,     \
    /* Row 2: 7 Left (Outer + A..G + Inner) | 7 Right (Inner + H..; + Outer) */         \
    L20, L21, L22, L23, L24, L25, L26,           R20, R21, R22, R23, R24, R25, R26,     \
    /* Row 3: 6 Left (Outer + Z..B)         | 6 Right (N../ + Outer)         */         \
    L30, L31, L32, L33, L34, L35,                     R31, R32, R33, R34, R35, R36,     \
    /* Thumb Lower Arc / Extra Row: 4 Left (LTL0..3) + LTU3 | RTU3 + 4 Right (RTL3..0)*/ \
    LTL0, LTL1, LTL2, LTL3, LTU3,                     RTU3, RTL3, RTL2, RTL1, RTL0,     \
    /* Thumb Upper Arc / Piano Keys: 3 Left (LTU0..2) | 3 Right (RTU2..0)    */         \
              LTU0, LTU1, LTU2,                         RTU2, RTU1, RTU0                \
)                                                                                       \
  {                                                                                     \
    { L00,  L01,  L02,  L03,  L04,  L05,  L06,  Key_NoKey,                              \
      Key_NoKey, R00,  R01,  R02,  R03,  R04,  R05,  R06 },                             \
    { L10,  L11,  L12,  L13,  L14,  L15,  L16,  Key_NoKey,                              \
      Key_NoKey, R10,  R11,  R12,  R13,  R14,  R15,  R16 },                             \
    { L20,  L21,  L22,  L23,  L24,  L25,  L26,  Key_NoKey,                              \
      Key_NoKey, R20,  R21,  R22,  R23,  R24,  R25,  R26 },                             \
    { L30,  L31,  L32,  L33,  L34,  L35,  Key_NoKey, Key_NoKey,                         \
      Key_NoKey, Key_NoKey, R31,  R32,  R33,  R34,  R35,  R36 },                        \
    { LTU0, LTU1, LTU2, LTU3, LTL3, LTL2, LTL1, LTL0,                                   \
      RTL0, RTL1, RTL2, RTL3, RTU3, RTU2, RTU1, RTU0 }                                  \
  }
#endif
