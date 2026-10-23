// -*- mode: c++ -*-
// SPDX-License-Identifier: GPL-3.0-or-later AND MIT
// =============================================================================
// EngrammerDefy.ino
// Complete Kaleidoscope Firmware Port of Sunaku's "Glorious Engrammer" (QWERTY)
// Adapted for the Dygma Defy (70 Keys)
//
// Licenses & Attributions:
// - Kaleidoscope firmware & Dygma Defy plugin integration:
//   Copyright (c) DygmaLab SE and Keyboardio, Inc. (Licensed under GPL-3.0)
// - Glorious Engrammer keymap layout & layer architecture:
//   Copyright (c) Suraj N. Kurapati <https://github.com/sunaku/glove80-keymaps> (MIT)
// =============================================================================

#include "Kaleidoscope.h"
#include "Kaleidoscope-EEPROM-Settings.h"
#include "Kaleidoscope-FocusSerial.h"
#include "Kaleidoscope-LEDControl.h"
#include "Kaleidoscope-Macros.h"
#include "Kaleidoscope-MagicCombo.h"
#include "Kaleidoscope-MouseKeys.h"
#include "Kaleidoscope-OneShot.h"
#include "Kaleidoscope-Qukeys.h"

#include "config.h"

// =============================================================================
// 1. PERSISTENT OS MANAGER PLUGIN (CAGS <-> GACS RUNTIME SWITCHING)
// =============================================================================

namespace kaleidoscope {
namespace plugin {

class EngrammerOSManager : public kaleidoscope::Plugin {
 public:
  EventHandlerResult onSetup() {
    eeprom_slice_ = ::EEPROMSettings.requestSlice(sizeof(EngrammerPersistentConfig));
    Runtime.storage().get(eeprom_slice_, config_);

    if (config_.magic != EEPROM_MAGIC_BYTE ||
        (config_.os_mode != 'M' && config_.os_mode != 'W' && config_.os_mode != 'L')) {
      config_.magic    = EEPROM_MAGIC_BYTE;
      config_.os_mode  = OPERATING_SYSTEM;
      config_.reserved = 0;
      Runtime.storage().put(eeprom_slice_, config_);
      Runtime.storage().commit();
    }

    applyOSLayer();
    return EventHandlerResult::OK;
  }

  void setOSMode(char mode) {
    config_.os_mode = (mode == 'M') ? 'M' : 'W';
    Runtime.storage().put(eeprom_slice_, config_);
    Runtime.storage().commit();
    applyOSLayer();
  }

  void toggleOSMode() {
    setOSMode(isMacOS() ? 'W' : 'M');
  }

  bool isMacOS() const {
    return config_.os_mode == 'M';
  }

  void applyOSLayer() {
    // Deactivate transient modal base overrides (Gaming / Plain) when resetting base
    Layer.deactivate(LAYER_GAMING);
    Layer.deactivate(LAYER_PLAIN);

    if (isMacOS()) {
      Layer.activate(LAYER_BASE_MAC);
      Layer.deactivate(LAYER_BASE_WIN);
    } else {
      Layer.activate(LAYER_BASE_WIN);
      Layer.deactivate(LAYER_BASE_MAC);
    }
  }

 private:
  uint16_t                  eeprom_slice_{0};
  EngrammerPersistentConfig config_{EEPROM_MAGIC_BYTE, OPERATING_SYSTEM, 0};
};

} // namespace plugin
} // namespace kaleidoscope

kaleidoscope::plugin::EngrammerOSManager OSManager;

// =============================================================================
// 2. CAPS-WORD STATE MACHINE PLUGIN (CW_TOGG)
// =============================================================================

namespace kaleidoscope {
namespace plugin {

class CapsWordPlugin : public kaleidoscope::Plugin {
 public:
  void enable() {
    active_        = true;
    last_activity_ = Runtime.millisAtCycleStart();
  }

  void disable() {
    active_ = false;
  }

  void toggle() {
    if (active_) {
      disable();
    } else {
      enable();
    }
  }

  bool isActive() const {
    return active_;
  }

  EventHandlerResult onKeyswitchEvent(KeyEvent &event) {
    if (!active_ || !keyToggledOn(event.state)) {
      return EventHandlerResult::OK;
    }

    last_activity_ = Runtime.millisAtCycleStart();
    Key k = event.key;

    if (k.isKeyboardModifier() || k.isLayerShift() || k == M(M_CAPS_WORD)) {
      return EventHandlerResult::OK;
    }

    // Alpha keys A..Z -> dynamically apply LeftShift
    if (k.getKeyCode() >= Key_A.getKeyCode() && k.getKeyCode() <= Key_Z.getKeyCode() &&
        k.getFlags() == KEY_FLAGS) {
      event.key = LSHIFT(k);
      return EventHandlerResult::OK;
    }

    // Word-continuing keys: numbers 1..0, Minus/Underscore, Backspace, Delete
    if ((k.getKeyCode() >= Key_1.getKeyCode() && k.getKeyCode() <= Key_0.getKeyCode()) ||
        k == Key_Minus || k == LSHIFT(Key_Minus) ||
        k == Key_Backspace || k == Key_Delete) {
      return EventHandlerResult::OK;
    }

    // Space, Enter, Escape, Tab, or any navigation/non-alpha key deactivates Caps-Word
    disable();
    return EventHandlerResult::OK;
  }

  EventHandlerResult afterEachCycle() {
    if (active_ && Runtime.hasTimeExpired(last_activity_, CAPS_WORD_IDLE_TIMEOUT_MS)) {
      disable();
    }
    return EventHandlerResult::OK;
  }

 private:
  bool     active_{false};
  uint16_t last_activity_{0};
};

} // namespace plugin
} // namespace kaleidoscope

kaleidoscope::plugin::CapsWordPlugin CapsWord;

// =============================================================================
// 3. FORGIVENESS & DUAL-FUNCTION PLUGIN
//    - SPACE_FORGIVENESS & THUMB_FORGIVENESS (Retro-Tap on lingering thumb holds)
//    - SHIFT_FORGIVENESS (Fast-roll into index-finger Shift F / J)
//    - DUAL_FUNC_0: Tap '(' [LSHIFT(Key_9)], Hold '<' [LSHIFT(Key_Comma)]
//    - DUAL_FUNC_1: Tap ')' [LSHIFT(Key_0)], Hold '>' [LSHIFT(Key_Period)]
// =============================================================================

namespace kaleidoscope {
namespace plugin {

class ForgivenessAndDualFuncPlugin : public kaleidoscope::Plugin {
 public:
  EventHandlerResult onKeyswitchEvent(KeyEvent &event) {
    if (event.state & INJECTED) {
      return EventHandlerResult::OK;
    }

    uint16_t now = Runtime.millisAtCycleStart();

    // -------------------------------------------------------------------------
    // A. Sunaku Glorious Engrammer Shift-Morph ("parang_left" / "parang_right")
    //    + Dual-Function Hold fallback:
    //    - M_DUAL_LPRN_LABK: Unshifted Tap = '(' [LSHIFT(Key_9)],
    //                        Shifted (Shift + Tap) OR Hold = '<' [LSHIFT(Key_Comma)]
    //    - M_DUAL_RPRN_RABK: Unshifted Tap = ')' [LSHIFT(Key_0)],
    //                        Shifted (Shift + Tap) OR Hold = '>' [LSHIFT(Key_Period)]
    // -------------------------------------------------------------------------
    if (event.key == M(M_DUAL_LPRN_LABK)) {
      if (keyToggledOn(event.state)) {
        if (isShiftActive()) {
          dual0_down_ = false;
          tapKey(event.addr, LSHIFT(Key_Comma)); // Shift + '(' -> '<'
        } else {
          dual0_down_     = true;
          dual0_press_ms_ = now;
        }
      } else if (keyToggledOff(event.state) && dual0_down_) {
        dual0_down_ = false;
        Key out = (isShiftActive() || (uint16_t)(now - dual0_press_ms_) >= QUKEYS_HOLD_TIMEOUT_MS)
                      ? LSHIFT(Key_Comma) // '<'
                      : LSHIFT(Key_9);    // '('
        tapKey(event.addr, out);
      }
      return EventHandlerResult::EVENT_CONSUMED;
    }

    if (event.key == M(M_DUAL_RPRN_RABK)) {
      if (keyToggledOn(event.state)) {
        if (isShiftActive()) {
          dual1_down_ = false;
          tapKey(event.addr, LSHIFT(Key_Period)); // Shift + ')' -> '>'
        } else {
          dual1_down_     = true;
          dual1_press_ms_ = now;
        }
      } else if (keyToggledOff(event.state) && dual1_down_) {
        dual1_down_ = false;
        Key out = (isShiftActive() || (uint16_t)(now - dual1_press_ms_) >= QUKEYS_HOLD_TIMEOUT_MS)
                      ? LSHIFT(Key_Period) // '>'
                      : LSHIFT(Key_0);     // ')'
        tapKey(event.addr, out);
      }
      return EventHandlerResult::EVENT_CONSUMED;
    }

#ifdef SHIFT_FORGIVENESS
    // -------------------------------------------------------------------------
    // B. SHIFT_FORGIVENESS: Forgiving fast roll into index-finger Shift (F / J)
    // -------------------------------------------------------------------------
    if (!Layer.isActive(LAYER_PLAIN) && !Layer.isActive(LAYER_GAMING)) {
      if (event.addr == KeyAddr(LEFT_HR_ROW, LEFT_INDEX_COL)) {
        if (keyToggledOn(event.state)) {
          left_shift_down_     = true;
          left_shift_press_ms_ = now;
        } else if (keyToggledOff(event.state)) {
          left_shift_down_ = false;
        }
      } else if (event.addr == KeyAddr(RIGHT_HR_ROW, RIGHT_INDEX_COL)) {
        if (keyToggledOn(event.state)) {
          right_shift_down_     = true;
          right_shift_press_ms_ = now;
        } else if (keyToggledOff(event.state)) {
          right_shift_down_ = false;
        }
      } else if (keyToggledOn(event.state) && isAlphaKey(event.key)) {
        if (left_shift_down_ && isRightHand(event.addr) &&
            (uint16_t)(now - left_shift_press_ms_) <= SHIFT_FORGIVENESS_WINDOW_MS) {
          event.key = LSHIFT(event.key);
        } else if (right_shift_down_ && isLeftHand(event.addr) &&
                   (uint16_t)(now - right_shift_press_ms_) <= SHIFT_FORGIVENESS_WINDOW_MS) {
          event.key = RSHIFT(event.key);
        }
      }
    }
#endif

#if defined(SPACE_FORGIVENESS) || defined(THUMB_FORGIVENESS)
    // -------------------------------------------------------------------------
    // C. SPACE_FORGIVENESS & THUMB_FORGIVENESS: Retro-tap on lingering thumb holds
    // -------------------------------------------------------------------------
    if (isCoreThumbKey(event.addr)) {
      if (keyToggledOn(event.state)) {
        last_thumb_addr_     = event.addr;
        last_thumb_press_ms_ = now;
        chorded_since_thumb_ = false;
      } else if (keyToggledOff(event.state) && event.addr == last_thumb_addr_) {
        uint16_t held_ms        = now - last_thumb_press_ms_;
        bool     is_space_thumb = (event.addr == ADDR_RT_SPC);
        uint16_t max_window     = is_space_thumb ? SPACE_RETRO_TAP_WINDOW_MS
                                                 : THUMB_RETRO_TAP_WINDOW_MS;

        if (!chorded_since_thumb_ &&
            held_ms > QUKEYS_HOLD_TIMEOUT_MS &&
            held_ms <= max_window) {
          Key tap_key = Layer.lookupOnActiveLayer(event.addr);
          if (!tap_key.isLayerShift() && tap_key != Key_NoKey) {
            tapKey(event.addr, tap_key);
          }
        }
        last_thumb_addr_ = KeyAddr::none();
      }
    } else if (keyToggledOn(event.state)) {
      chorded_since_thumb_ = true;
    }
#endif

    return EventHandlerResult::OK;
  }

 private:
  bool isShiftActive() const {
    return left_shift_down_ || right_shift_down_ ||
           Runtime.hid().keyboard().wasModifierKeyActive(Key_LeftShift) ||
           Runtime.hid().keyboard().wasModifierKeyActive(Key_RightShift) ||
           ::OneShot.isActive(Key_LeftShift) ||
           ::OneShot.isActive(Key_RightShift);
  }

  static void tapKey(KeyAddr addr, Key k) {
    Runtime.handleKeyEvent(KeyEvent(addr, IS_PRESSED | INJECTED, k));
    Runtime.handleKeyEvent(KeyEvent(addr, WAS_PRESSED | INJECTED, k));
  }

  static bool isLeftHand(KeyAddr addr) {
    return addr.col() < 8;
  }

  static bool isRightHand(KeyAddr addr) {
    return addr.col() >= 8;
  }

  static bool isCoreThumbKey(KeyAddr addr) {
    return addr == ADDR_LT_ESC  || addr == ADDR_LT_BSPC ||
           addr == ADDR_LT_DEL  || addr == ADDR_LT_INS  ||
           addr == ADDR_RT_ENT  || addr == ADDR_RT_INS  ||
           addr == ADDR_RT_TAB  || addr == ADDR_RT_SPC;
  }

  static bool isAlphaKey(Key k) {
    return (k.getKeyCode() >= Key_A.getKeyCode() &&
            k.getKeyCode() <= Key_Z.getKeyCode());
  }

  bool     dual0_down_{false};
  bool     dual1_down_{false};
  uint16_t dual0_press_ms_{0};
  uint16_t dual1_press_ms_{0};

  bool     left_shift_down_{false};
  bool     right_shift_down_{false};
  uint16_t left_shift_press_ms_{0};
  uint16_t right_shift_press_ms_{0};

  KeyAddr  last_thumb_addr_{KeyAddr::none()};
  uint16_t last_thumb_press_ms_{0};
  bool     chorded_since_thumb_{false};
};

} // namespace plugin
} // namespace kaleidoscope

kaleidoscope::plugin::ForgivenessAndDualFuncPlugin Forgiveness;

// =============================================================================
// 4. KEYMAP DEFINITIONS (EXACT PORT OF USER'S MOONLANDER GLORIOUS ENGRAMMER)
// =============================================================================

#define ___ Key_Transparent
#define XXX Key_NoKey

// Shorthand aliases for shifted symbols matching QMK keycodes in keymap.c
#define KC_EXLM LSHIFT(Key_1)
#define KC_AT   LSHIFT(Key_2)
#define KC_HASH LSHIFT(Key_3)
#define KC_DLR  LSHIFT(Key_4)
#define KC_PERC LSHIFT(Key_5)
#define KC_CIRC LSHIFT(Key_6)
#define KC_AMPR LSHIFT(Key_7)
#define KC_ASTR LSHIFT(Key_8)
#define KC_LPRN LSHIFT(Key_9)
#define KC_RPRN LSHIFT(Key_0)
#define KC_UNDS LSHIFT(Key_Minus)
#define KC_PLUS LSHIFT(Key_Equals)
#define KC_LCBR LSHIFT(Key_LeftBracket)
#define KC_RCBR LSHIFT(Key_RightBracket)
#define KC_PIPE LSHIFT(Key_Backslash)
#define KC_COLN LSHIFT(Key_Semicolon)
#define KC_DQUO LSHIFT(Key_Quote)
#define KC_LABK LSHIFT(Key_Comma)
#define KC_RABK LSHIFT(Key_Period)
#define KC_QUES LSHIFT(Key_Slash)
#define KC_TILD LSHIFT(Key_Backtick)

// clang-format off
KEYMAPS(

  // ---------------------------------------------------------------------------
  // LAYER 0: BASE QWERTY (macOS - CAGS Home-Row Modifiers via Qukeys)
  // Matches Moonlander [0] 1-to-1:
  // - Outer Left Col:  CapsLock, CW_TOGG, =, OSM(LShift)
  // - Inner Left Col:  F11, PageUp, PageDown
  // - Inner Right Col: F12, WheelDown, WheelUp
  // - Outer Right Col: CapsLock, -, ', OSM(RShift)
  // - Left Lower Arc:  MO(7), \, DUAL_FUNC_0 ('(' / '<'), DUAL_FUNC_1 (')' / '>')
  // - Left Upper Arc:  LT(3,Bksp), LT(2,Del), LT(10,Ins), LT(5,Esc)
  // - Right Upper Arc: LT(6,Enter), LT(10,Ins), LT(4,Tab), LT(1,Space)
  // - Right Lower Arc: [, ], `, MO(7)
  // ---------------------------------------------------------------------------
  [LAYER_BASE_MAC] = KEYMAP_DEFY_70(
    // Row 0 (7 Left | 7 Right)
    Key_CapsLock,       Key_1,          Key_2,             Key_3,             Key_4,     Key_5,     Key_F11,          Key_F12,           Key_6,          Key_7,          Key_8,     Key_9,          Key_0,         Key_CapsLock,
    // Row 1 (7 Left | 7 Right)
    M(M_CAPS_WORD),     Key_Q,          Key_W,             Key_E,             Key_R,     Key_T,     Key_PageUp,       ENGRAMMER_WH_DOWN, Key_Y,          Key_U,          Key_I,     Key_O,          Key_P,         Key_Minus,
    // Row 2 (7 Left | 7 Right) — CAGS Home-Row Mods bound via Qukeys
    Key_Equals,         Key_A,          Key_S,             Key_D,             Key_F,     Key_G,     Key_PageDown,     ENGRAMMER_WH_UP,   Key_H,          Key_J,          Key_K,     Key_L,          Key_Semicolon, Key_Quote,
    // Row 3 (6 Left | 6 Right)
    OSM(LeftShift),     Key_Z,          Key_X,             Key_C,             Key_V,     Key_B,                                          Key_N,          Key_M,          Key_Comma, Key_Period,     Key_Slash,     OSM(RightShift),
    // Thumb Lower Arc (4 Left) + Left Upper Inner (LTU3) | Right Upper Inner (RTU3) + Thumb Lower Arc (4 Right)
    ShiftToLayer(LAYER_MAGIC), Key_Backslash, M(M_DUAL_LPRN_LABK), M(M_DUAL_RPRN_RABK), Key_Escape,                   Key_Enter,         Key_LeftBracket,Key_RightBracket,Key_Backtick, ShiftToLayer(LAYER_MAGIC),
    // Thumb Upper Arc Piano Keys (3 Left: Bksp/L3, Del/L2, Ins/L10 | 3 Right: Ins/L10, Tab/L4, Space/L1)
                        Key_Backspace,  Key_Delete,        Key_Insert,                                                                   Key_Insert,     Key_Tab,        Key_Spacebar
  ),

  // ---------------------------------------------------------------------------
  // LAYER 1: SYMBOL LAYER — Held via Right Thumb LT(1, KC_SPACE)
  // Matches Moonlander [1] 1-to-1:
  // - Left Hand: Programming Symbols (` ( ) ; , | ! [ { } ] ? | # ^ = _ $ * . | ~ < | - > / | .. & ' " + \ % :)
  // - Right Hand: Arrow Navigation (Left/Up/Down/Right), ., ,, Bksp, Tab, Space, Enter, ``` (ST_MACRO_1), ; " ' ` \
  // ---------------------------------------------------------------------------
  [LAYER_SYMBOL] = KEYMAP_DEFY_70(
    // Row 0
    ___,                Key_Backtick,   KC_LPRN,           KC_RPRN,           Key_Semicolon, Key_Comma, ___,          ___,               ___,            ___,            ___,       ___,            ___,           ___,
    // Row 1
    KC_EXLM,            Key_LeftBracket,KC_LCBR,           KC_RCBR,           Key_RightBracket, KC_QUES, ___,         ___,               Key_Period,     Key_LeftArrow,  Key_UpArrow, Key_DownArrow,Key_RightArrow,___,
    // Row 2
    KC_HASH,            KC_CIRC,        Key_Equals,        KC_UNDS,           KC_DLR,    KC_ASTR,   Key_Period,       ___,               Key_Comma,      Key_Backspace,  Key_Tab,   Key_Spacebar,   Key_Enter,     M(M_ST_MACRO_1),
    // Row 3
    KC_TILD,            KC_LABK,        KC_PIPE,           Key_Minus,         KC_RABK,   Key_Slash,                                      Key_Semicolon,  KC_DQUO,        Key_Quote, Key_Backtick,   Key_Backslash, ___,
    // Thumb Lower Arc + Inner Upper
    M(M_ST_MACRO_0),    KC_AMPR,        Key_Quote,         KC_DQUO,           Key_Backslash,                          ___,               Key_LeftGui,    Key_LeftAlt,    Key_LeftControl, ___,
    // Thumb Upper Arc Piano Keys
                        KC_PERC,        KC_COLN,           ___,                                                                          ___,            ___,            ___
  ),

  // ---------------------------------------------------------------------------
  // LAYER 2: NUMBER LAYER — Held via Left Thumb LT(2, KC_DELETE)
  // Matches Moonlander [2] 1-to-1:
  // - Left Hand: Esc/Ins/S-Tab/Del, Enter/Space/Tab/Bksp/0, Same-Hand Mods + X, E/SelAll/SelLine/SelWord/Find/F, _/Undo/Redo/FindPrev/FindNext
  // - Right Hand: | # $ ^ ~ !, > G 7 8 9 : %, < K 4 5 6 - +, J 1 2 3 / *, , ( ) @ = . 0
  // ---------------------------------------------------------------------------
  [LAYER_NUMBER] = KEYMAP_DEFY_70(
    // Row 0
    ___,                Key_Escape,     Key_Insert,        LSHIFT(Key_Tab),   Key_Delete, ___,      ___,              ___,               KC_PIPE,        KC_HASH,        KC_DLR,    KC_CIRC,        KC_TILD,       KC_EXLM,
    // Row 1
    ___,                Key_Enter,      Key_Spacebar,      Key_Tab,           Key_Backspace, Key_Keypad0, ___,        KC_RABK,           LSHIFT(Key_G),  Key_Keypad7,    Key_Keypad8, Key_Keypad9,  KC_COLN,       KC_PERC,
    // Row 2
    OSM(LeftShift),     Key_LeftControl,Key_LeftAlt,       Key_LeftGui,       Key_LeftShift, Key_X, ___,              KC_LABK,           Key_K,          Key_Keypad4,    Key_Keypad5, Key_Keypad6,  Key_KeypadSubtract, Key_KeypadAdd,
    // Row 3
    Key_E,              M(M_SELECT_ALL),M(M_SELECT_LINE),  M(M_SELECT_WORD),  M(M_FIND), Key_F,                                          Key_J,          Key_Keypad1,    Key_Keypad2, Key_Keypad3,  Key_KeypadDivide, Key_KeypadMultiply,
    // Thumb Lower Arc + Inner Upper
    KC_UNDS,            M(M_UNDO),      M(M_REDO),         M(M_FIND_PREV),    M(M_FIND_NEXT),                         Key_Comma,         KC_LPRN,        ___,            KC_RPRN,   KC_AT,
    // Thumb Upper Arc Piano Keys
                        ___,            ___,               ___,                                                                          Key_Equals,     Key_KeypadDot,  Key_Keypad0
  ),

  // ---------------------------------------------------------------------------
  // LAYER 3: CURSOR LAYER — Held via Left Thumb LT(3, KC_BSPC)
  // Matches Moonlander [3] 1-to-1:
  // - Left Hand: Esc/Ins/S-Tab/Del/Ins, Enter/Space/Tab/Bksp/Cut, Same-Hand Mods + Copy, GotoLine/SelAll/SelLine/SelWord/Find/Paste
  // - Right Hand: Cut/Bksp/Undo/Redo/Tab, Copy + Left/Up/Down/Right + OSM(LSft), Paste + Home/PgUp/PgDn/End + GotoLine
  // ---------------------------------------------------------------------------
  [LAYER_CURSOR] = KEYMAP_DEFY_70(
    // Row 0
    ___,                Key_Escape,     Key_Insert,        LSHIFT(Key_Tab),   Key_Delete, Key_Insert, ___,            ___,               Key_Insert,     ___,            ___,       ___,            ___,           ___,
    // Row 1
    ___,                Key_Enter,      Key_Spacebar,      Key_Tab,           Key_Backspace, M(M_CUT), ___,           ___,               M(M_CUT),       Key_Backspace,  M(M_UNDO), M(M_REDO),      Key_Tab,       ___,
    // Row 2
    OSM(LeftShift),     Key_LeftControl,Key_LeftAlt,       Key_LeftGui,       Key_LeftShift, M(M_COPY), ___,          ___,               M(M_COPY),      Key_LeftArrow,  Key_UpArrow, Key_DownArrow,Key_RightArrow,OSM(LeftShift),
    // Row 3
    M(M_GOTO_LINE),     M(M_SELECT_ALL),M(M_SELECT_LINE),  M(M_SELECT_WORD),  M(M_FIND), M(M_PASTE),                                     M(M_PASTE),     Key_Home,       Key_PageUp,Key_PageDown,   Key_End,       M(M_GOTO_LINE),
    // Thumb Lower Arc + Inner Upper
    M(M_QUICK_OPEN),    M(M_UNDO),      M(M_REDO),         M(M_FIND_PREV),    M(M_FIND_NEXT),                         LALT(LSHIFT(Key_RightArrow)), M(M_FIND), M(M_FIND_PREV), M(M_FIND_NEXT), M(M_FIND_REPLACE),
    // Thumb Upper Arc Piano Keys
                        ___,            ___,               ___,                                                                          M(M_SELECT_ALL),M(M_SELECT_LINE),M(M_SELECT_WORD)
  ),

  // ---------------------------------------------------------------------------
  // LAYER 4: MOUSE LAYER — Held via Right Thumb LT(4, KC_TAB)
  // Matches Moonlander [4] 1-to-1:
  // - Left & Right Mouse Cursor Movement, Natural Wheel Scrolling, Buttons 1..5, Cut/Copy/Paste, Modifiers
  // ---------------------------------------------------------------------------
  [LAYER_MOUSE] = KEYMAP_DEFY_70(
    // Row 0
    ___,                ___,                ___,               ___,                ___,                ___,               ___,                 ___,            ___,                OSM(LeftShift),     OSM(LeftGui),       OSM(LeftAlt),       OSM(LeftControl),   ___,
    // Row 1
    ___,                ___,                ENGRAMMER_WH_RIGHT,ENGRAMMER_MS_UP,    ENGRAMMER_WH_LEFT,  M(M_CUT),          ENGRAMMER_MS_BTN5,   ___,            ENGRAMMER_WH_RIGHT, ENGRAMMER_MS_BTN1,  ENGRAMMER_MS_BTN2,  ENGRAMMER_MS_BTN3,  ENGRAMMER_WH_LEFT,  ___,
    // Row 2
    ___,                ENGRAMMER_WH_DOWN,  ENGRAMMER_MS_LEFT, ENGRAMMER_MS_DOWN,  ENGRAMMER_MS_RIGHT, ENGRAMMER_WH_DOWN, ENGRAMMER_MS_BTN4,   ___,            ENGRAMMER_WH_DOWN,  ENGRAMMER_MS_ACCEL1,ENGRAMMER_MS_ACCEL2,ENGRAMMER_MS_ACCEL0,___,                ENGRAMMER_MS_BTN4,
    // Row 3
    ___,                ENGRAMMER_WH_UP,    ENGRAMMER_MS_ACCEL0,ENGRAMMER_MS_ACCEL2,ENGRAMMER_MS_ACCEL1,ENGRAMMER_WH_UP,                                       ENGRAMMER_WH_UP,    ENGRAMMER_MS_LEFT,  ENGRAMMER_MS_UP,    ENGRAMMER_MS_DOWN,  ENGRAMMER_MS_RIGHT, ENGRAMMER_MS_BTN5,
    // Thumb Lower Arc + Inner Upper
    ___,                M(M_CUT),           M(M_COPY),         M(M_PASTE),         ENGRAMMER_MS_BTN3,                                          ___,            Key_LeftShift,      Key_LeftGui,        Key_LeftAlt,        Key_LeftControl,
    // Thumb Upper Arc Piano Keys
                        ENGRAMMER_MS_BTN1,  ENGRAMMER_MS_BTN2, ENGRAMMER_MS_BTN4,                                                                              ___,                ___,                ___
  ),

  // ---------------------------------------------------------------------------
  // LAYER 5: FUNCTION / MEDIA LAYER — Held via Left Thumb LT(5, KC_ESCAPE)
  // Matches Moonlander [5] 1-to-1:
  // - Left Hand: Same-Hand Modifiers, Editing/Selection Macros, Volume Up/Down
  // - Right Hand: Media Transport Controls + 3x3 F1..F12 + F13..F15 + Mute/VolDn/VolUp on Right Thumb
  // ---------------------------------------------------------------------------
  [LAYER_FUNCTION] = KEYMAP_DEFY_70(
    // Row 0
    ___,                Key_Escape,     Key_Insert,        LSHIFT(Key_Tab),   Key_Delete, ___,       ___,                       ___,                       Key_RightGui,   Consumer_PlaySlashPause, Consumer_ScanPreviousTrack, Consumer_ScanNextTrack, Consumer_Stop, ___,
    // Row 1
    ___,                Key_Enter,      Key_Spacebar,      Key_Tab,           Key_Backspace, ___,    Consumer_VolumeIncrement,  Consumer_ScanNextTrack,    ___,            Key_F7,                  Key_F8,                     Key_F9,                 Key_F10,       Key_F13,
    // Row 2
    OSM(LeftShift),     Key_LeftControl,Key_LeftAlt,       Key_LeftGui,       Key_LeftShift, ___,    Consumer_VolumeDecrement,  Consumer_ScanPreviousTrack,___,            Key_F4,                  Key_F5,                     Key_F6,                 Key_F11,       Key_F14,
    // Row 3
    ___,                M(M_SELECT_ALL),M(M_SELECT_LINE),  M(M_SELECT_WORD),  M(M_FIND),  ___,                                                             ___,            Key_F1,                  Key_F2,                     Key_F3,                 Key_F12,       Key_F15,
    // Thumb Lower Arc + Inner Upper
    ___,                M(M_UNDO),      M(M_REDO),         M(M_FIND_PREV),    M(M_FIND_NEXT),                                   Consumer_PlaySlashPause,   Key_LEDEffectNext, Key_LEDEffectPrevious, ___,                       ___,
    // Thumb Upper Arc Piano Keys
                        ___,            ___,               ___,                                                                                            Consumer_Mute,  Consumer_VolumeDecrement, Consumer_VolumeIncrement
  ),

  // ---------------------------------------------------------------------------
  // LAYER 6: SYSTEM / RGB / POWER / WIRELESS LAYER — Held via Right Thumb LT(6, KC_ENTER)
  // Matches Moonlander [6] + OS Switch + Dygma Defy Wireless/Bluetooth/Battery Controls:
  // - Row 0 Left:  OS Toggle, OS Mac, OS Win, Battery Level (LED%), BT Pair, 2.4GHz RF Mode, Wireless Toggle
  // - Row 0 Right: Bluetooth Host Slots BT_1 .. BT_5, OS Toggle
  // - Rows 1..5:   RGB Controls, PrintScreen, Pause, ScrollLock, NumLock, CapsLock, Lock Screen, System Sleep/Power
  // ---------------------------------------------------------------------------
  [LAYER_SYS_RGB] = KEYMAP_DEFY_70(
    // Row 0
    M(M_OS_TOGGLE),     M(M_OS_MAC),    M(M_OS_WIN),       M(M_BAT_LEVEL),    M(M_BT_PAIR), M(M_RF_MODE), M(M_WL_TOGGLE), ___,           M(M_BT_SEL_1),  M(M_BT_SEL_2),  M(M_BT_SEL_3), M(M_BT_SEL_4), M(M_BT_SEL_5), M(M_OS_TOGGLE),
    // Row 1
    ___,                Key_LEDEffectNext, Key_LEDEffectNext, Key_LEDEffectNext, Key_LEDEffectNext, Key_LEDEffectNext, ___, ___,         Key_LEDEffectNext, Key_LEDEffectNext, Key_LEDEffectNext, Key_LEDEffectNext, Key_LEDEffectNext, ___,
    // Row 2
    ___,                Key_LEDEffectPrevious, Key_LEDEffectPrevious, Key_LEDEffectPrevious, Key_LEDEffectPrevious, Key_LEDEffectNext, ___, ___, Key_LEDEffectNext, Key_LEDEffectPrevious, Key_LEDEffectPrevious, Key_LEDEffectPrevious, Key_LEDEffectPrevious, ___,
    // Row 3
    ___,                ___,            Key_PcApplication, Key_PrintScreen,   Key_Pause,  Key_LEDToggle,                                 Key_LEDToggle,  Key_ScrollLock, Key_KeypadNumLock, Key_CapsLock, M(M_LOCK_SCREEN), Consumer_Sleep,
    // Thumb Lower Arc + Inner Upper
    ___,                ___,            ___,               ___,               Consumer_Sleep,                         ___,               ___,            ___,            ___,       System_PowerDown,
    // Thumb Upper Arc Piano Keys
                        M(M_LOCK_SCREEN), System_PowerDown, ___,                                                                         Key_PcApplication, Key_PrintScreen, Key_Pause
  ),

  // ---------------------------------------------------------------------------
  // LAYER 7: MAGIC / BOOT / OS-SWITCH LAYER — Held via Outer Lower Thumb MO(7)
  // Matches Moonlander [7] + Persistent OS Switch (CAGS <-> GACS):
  // - TO(0) on L00 & Left Inner Thumb, TO(8) Gaming on Right Inner Thumb, LED Controls, OS Switch
  // ---------------------------------------------------------------------------
  [LAYER_MAGIC] = KEYMAP_DEFY_70(
    // Row 0
    M(M_TO_BASE),       M(M_OS_MAC),    M(M_OS_WIN),       M(M_OS_TOGGLE),    ___,        ___,       ___,             ___,               ___,            ___,            ___,       ___,            ___,           ___,
    // Row 1
    ___,                Key_LEDEffectNext, Key_LEDEffectNext, Key_LEDEffectNext, Key_LEDEffectNext, Key_LEDToggle, ___, ___,             ___,            ___,            ___,       ___,            ___,           ___,
    // Row 2
    M(M_OS_TOGGLE),     Key_LEDEffectPrevious, Key_LEDEffectPrevious, Key_LEDEffectPrevious, Key_LEDEffectPrevious, Key_LEDEffectNext, ___, ___, ___,    ___,            ___,       ___,            ___,           M(M_OS_TOGGLE),
    // Row 3
    ___,                ___,            ___,               ___,               ___,        ___,                                           ___,            ___,            ___,       ___,            ___,           ___,
    // Thumb Lower Arc + Inner Upper
    ___,                ___,            ___,               ___,               ___,                                    ___,               ___,            ___,            ___,       ___,
    // Thumb Upper Arc Piano Keys
                        ___,            ___,               M(M_TO_BASE),                                                                 MoveToLayer(LAYER_GAMING), ___, ___
  ),

  // ---------------------------------------------------------------------------
  // LAYER 8: GAMING LAYER — Entered via B+LShift Combo or Layer 7 -> TO(8)
  // Matches Moonlander [8] 1-to-1 (Exited via F+C Combo or Layer 7 -> TO(0))
  // ---------------------------------------------------------------------------
  [LAYER_GAMING] = KEYMAP_DEFY_70(
    // Row 0
    Key_M,              Key_6,          Key_7,             Key_8,             Key_9,      Key_0,     Key_Escape,      Key_F1,            Key_F2,         Key_F3,         Key_F4,    Key_F5,         Key_F6,        Key_F7,
    // Row 1
    Key_I,              Key_1,          Key_2,             Key_3,             Key_4,      Key_5,     Key_T,           Key_F8,            Key_F9,         Key_F10,        Key_F11,   Key_F12,        ___,           ___,
    // Row 2
    Key_T,              Key_Tab,        Key_Q,             Key_W,             Key_E,      Key_R,     Key_N,           ___,               Key_H,          Key_J,          Key_K,     Key_L,          Key_Semicolon, Key_Quote,
    // Row 3
    Key_G,              Key_LeftShift,  Key_A,             Key_S,             Key_D,      Key_F,                                         Key_N,          Key_M,          Key_Comma, Key_Period,     Key_Slash,     Key_Backslash,
    // Thumb Lower Arc + Inner Upper
    Key_B,              Key_LeftControl,Key_Z,             Key_X,             Key_LeftShift,                          Key_UpArrow,       Key_Minus,      Key_LeftBracket,Key_RightBracket, Key_Equals,
    // Thumb Upper Arc Piano Keys
                        Key_Spacebar,   Key_LeftControl,   Key_Enter,                                                                    Key_LeftArrow,  Key_DownArrow,  Key_RightArrow
  ),

  // ---------------------------------------------------------------------------
  // LAYER 9: PLAIN QWERTY LAYER (No Home-Row Modifiers) — Toggled via N+RShift Combo
  // Matches Moonlander [9] 1-to-1
  // ---------------------------------------------------------------------------
  [LAYER_PLAIN] = KEYMAP_DEFY_70(
    // Row 0
    Key_CapsLock,       Key_1,          Key_2,             Key_3,             Key_4,      Key_5,     ___,             ___,               Key_6,          Key_7,          Key_8,     Key_9,          Key_0,         Key_CapsLock,
    // Row 1
    M(M_CAPS_WORD),     Key_Q,          Key_W,             Key_E,             Key_R,      Key_T,     ___,             ___,               Key_Y,          Key_U,          Key_I,     Key_O,          Key_P,         Key_Minus,
    // Row 2 (Plain A S D F / J K L ; without Qukeys)
    Key_Equals,         Key_A,          Key_S,             Key_D,             Key_F,      Key_G,     ___,             ___,               Key_H,          Key_J,          Key_K,     Key_L,          Key_Semicolon, Key_Quote,
    // Row 3
    OSM(LeftShift),     Key_Z,          Key_X,             Key_C,             Key_V,      Key_B,                                         Key_N,          Key_M,          Key_Comma, Key_Period,     Key_Slash,     OSM(RightShift),
    // Thumb Lower Arc + Inner Upper
    ___,                Key_Backslash,  M(M_DUAL_LPRN_LABK), M(M_DUAL_RPRN_RABK), Key_Escape,                         Key_Enter,         ___,            ___,            ___,       ___,
    // Thumb Upper Arc Piano Keys
                        Key_Backspace,  Key_Delete,        Key_Insert,                                                                   Key_Insert,     Key_Tab,        Key_Spacebar
  ),

  // ---------------------------------------------------------------------------
  // LAYER 10: LOWER LAYER — Held via Inner Thumb LT(10, KC_INSERT)
  // Matches Moonlander [10] 1-to-1:
  // - Row 1: Keypad 1..5 (Left), Keypad 6..0 (Right)
  // - Row 2: One-Shot Home-Row Mods (OSM LCTL/LALT/LGUI/LSFT) + KP_Minus / KP_Slash
  // - Row 3: ( , . ) + (Left), * NumLock ___ = KP_Enter (Right)
  // - Thumbs: Direct Layer Locks TO(0), TO(5), TO(3), TO(2) & TO(6), TO(0), TO(4), TO(1)
  // ---------------------------------------------------------------------------
  [LAYER_LOWER] = KEYMAP_DEFY_70(
    // Row 0
    ___,                ___,            ___,               ___,               ___,        ___,       ___,             ___,               ___,            ___,            ___,       ___,            ___,           ___,
    // Row 1
    ___,                Key_Keypad1,    Key_Keypad2,       Key_Keypad3,       Key_Keypad4,Key_Keypad5, ___,           ___,               Key_Keypad6,    Key_Keypad7,    Key_Keypad8, Key_Keypad9,  Key_Keypad0,   ___,
    // Row 2
    ___,                OSM(LeftControl), OSM(LeftAlt),    OSM(LeftGui),      OSM(LeftShift), Key_KeypadSubtract, ___, ___,              Key_KeypadDivide, OSM(LeftShift), OSM(LeftGui), OSM(LeftAlt), OSM(LeftControl), ___,
    // Row 3
    ___,                KC_LPRN,        Key_Comma,         Key_Period,        KC_RPRN,    Key_KeypadAdd,                                 Key_KeypadMultiply, Key_KeypadNumLock, ___, Key_Equals,   Key_KeypadEnter, ___,
    // Thumb Lower Arc + Inner Upper
    M(M_TO_BASE),       ___,            ___,               ___,               MoveToLayer(LAYER_FUNCTION),            MoveToLayer(LAYER_SYS_RGB), ___,   ___,            ___,       M(M_TO_BASE),
    // Thumb Upper Arc Piano Keys
                        MoveToLayer(LAYER_CURSOR), MoveToLayer(LAYER_NUMBER), ___,                                                       ___,            MoveToLayer(LAYER_MOUSE), MoveToLayer(LAYER_SYMBOL)
  ),

  // ---------------------------------------------------------------------------
  // LAYER 11: BASE QWERTY (Windows/Linux - GACS Home-Row Modifiers via Qukeys)
  // Identical to Layer 0 except Qukeys bind GACS (A=LGui, S=LAlt, D=LCtrl, F=LShift | J=RShift, K=RCtrl, L=LAlt, ;=RGui)
  // ---------------------------------------------------------------------------
  [LAYER_BASE_WIN] = KEYMAP_DEFY_70(
    // Row 0
    Key_CapsLock,       Key_1,          Key_2,             Key_3,             Key_4,     Key_5,     Key_F11,          Key_F12,           Key_6,          Key_7,          Key_8,     Key_9,          Key_0,         Key_CapsLock,
    // Row 1
    M(M_CAPS_WORD),     Key_Q,          Key_W,             Key_E,             Key_R,     Key_T,     Key_PageUp,       ENGRAMMER_WH_DOWN, Key_Y,          Key_U,          Key_I,     Key_O,          Key_P,         Key_Minus,
    // Row 2 — GACS Home-Row Mods bound via Qukeys
    Key_Equals,         Key_A,          Key_S,             Key_D,             Key_F,     Key_G,     Key_PageDown,     ENGRAMMER_WH_UP,   Key_H,          Key_J,          Key_K,     Key_L,          Key_Semicolon, Key_Quote,
    // Row 3
    OSM(LeftShift),     Key_Z,          Key_X,             Key_C,             Key_V,     Key_B,                                          Key_N,          Key_M,          Key_Comma, Key_Period,     Key_Slash,     OSM(RightShift),
    // Thumb Lower Arc + Inner Upper
    ShiftToLayer(LAYER_MAGIC), Key_Backslash, M(M_DUAL_LPRN_LABK), M(M_DUAL_RPRN_RABK), Key_Escape,                   Key_Enter,         Key_LeftBracket,Key_RightBracket,Key_Backtick, ShiftToLayer(LAYER_MAGIC),
    // Thumb Upper Arc Piano Keys
                        Key_Backspace,  Key_Delete,        Key_Insert,                                                                   Key_Insert,     Key_Tab,        Key_Spacebar
  )
)
// clang-format on

// =============================================================================
// 5. MACRO EXECUTION ENGINE (MOONLANDER ST_MACRO_0..13 & OS-AWARE SHORTCUTS)
// =============================================================================

const macro_t *macroAction(uint8_t macro_id, KeyEvent &event) {
  if (!keyToggledOn(event.state)) {
    return MACRO_NONE;
  }

  const bool is_mac = OSManager.isMacOS();

  switch (macro_id) {
    // ST_MACRO_0: ".."
    case M_ST_MACRO_0:
      return MACRO(T(Period), I(1), T(Period));

    // ST_MACRO_1: "```"
    case M_ST_MACRO_1:
      return MACRO(T(Backtick), I(1), T(Backtick), I(1), T(Backtick));

    // ST_MACRO_2 / 5 / 8 / 11: Select All
    case M_SELECT_ALL:
      return is_mac ? MACRO(D(LeftGui), T(A), U(LeftGui))
                    : MACRO(D(LeftControl), T(A), U(LeftControl));

    // ST_MACRO_3 / 6 / 9 / 12: Select Line (Shift+Home -> Shift+End)
    case M_SELECT_LINE:
      return MACRO(D(LeftShift), T(Home), U(LeftShift), W(100), D(LeftShift), T(End), U(LeftShift));

    // ST_MACRO_4 / 7 / 10 / 13: Select Word (Alt+Shift+Left -> Alt+Shift+Right)
    case M_SELECT_WORD:
      return is_mac ? MACRO(D(LeftAlt), D(LeftShift), T(LeftArrow), U(LeftShift), U(LeftAlt), W(100),
                            D(LeftAlt), D(LeftShift), T(RightArrow), U(LeftShift), U(LeftAlt))
                    : MACRO(D(LeftControl), D(LeftShift), T(LeftArrow), U(LeftShift), U(LeftControl), W(100),
                            D(LeftControl), D(LeftShift), T(RightArrow), U(LeftShift), U(LeftControl));

    // OS Mode & Base Layer Switching
    case M_OS_TOGGLE:
      OSManager.toggleOSMode();
      return MACRO_NONE;

    case M_OS_MAC:
      OSManager.setOSMode('M');
      return MACRO_NONE;

    case M_OS_WIN:
      OSManager.setOSMode('W');
      return MACRO_NONE;

    case M_CAPS_WORD:
      CapsWord.toggle();
      return MACRO_NONE;

    case M_TO_BASE:
      OSManager.applyOSLayer();
      return MACRO_NONE;

    case M_TG_PLAIN:
      if (Layer.isActive(LAYER_PLAIN)) {
        Layer.deactivate(LAYER_PLAIN);
      } else {
        Layer.activate(LAYER_PLAIN);
      }
      return MACRO_NONE;

    // OS-Aware Editing & Search Shortcuts
    case M_UNDO:
      return is_mac ? MACRO(D(LeftGui), T(Z), U(LeftGui))
                    : MACRO(D(LeftControl), T(Z), U(LeftControl));

    case M_REDO:
      return is_mac ? MACRO(D(LeftGui), D(LeftShift), T(Z), U(LeftShift), U(LeftGui))
                    : MACRO(D(LeftControl), T(Y), U(LeftControl));

    case M_CUT:
      return is_mac ? MACRO(D(LeftGui), T(X), U(LeftGui))
                    : MACRO(D(LeftControl), T(X), U(LeftControl));

    case M_COPY:
      return is_mac ? MACRO(D(LeftGui), T(C), U(LeftGui))
                    : MACRO(D(LeftControl), T(C), U(LeftControl));

    case M_PASTE:
      return is_mac ? MACRO(D(LeftGui), T(V), U(LeftGui))
                    : MACRO(D(LeftControl), T(V), U(LeftControl));

    case M_FIND:
      return is_mac ? MACRO(D(LeftGui), T(F), U(LeftGui))
                    : MACRO(D(LeftControl), T(F), U(LeftControl));

    case M_FIND_NEXT:
      return is_mac ? MACRO(D(LeftGui), T(G), U(LeftGui))
                    : MACRO(D(LeftControl), T(G), U(LeftControl));

    case M_FIND_PREV:
      return is_mac ? MACRO(D(LeftGui), D(LeftShift), T(G), U(LeftShift), U(LeftGui))
                    : MACRO(D(LeftControl), D(LeftShift), T(G), U(LeftShift), U(LeftControl));

    case M_FIND_REPLACE:
      return is_mac ? MACRO(D(LeftAlt), D(LeftGui), T(F), U(LeftGui), U(LeftAlt))
                    : MACRO(D(LeftControl), T(H), U(LeftControl));

    case M_GOTO_LINE:
      return is_mac ? MACRO(D(LeftGui), T(L), U(LeftGui))
                    : MACRO(D(LeftControl), T(G), U(LeftControl));

    case M_QUICK_OPEN:
      return is_mac ? MACRO(D(LeftGui), T(P), U(LeftGui))
                    : MACRO(D(LeftControl), T(P), U(LeftControl));

    case M_LOCK_SCREEN:
      return is_mac ? MACRO(D(LeftControl), D(RightGui), T(Q), U(RightGui), U(LeftControl))
                    : MACRO(D(LeftGui), T(L), U(LeftGui));

    // Thumb Chord Switchers
    case M_ALT_TAB:
      return MACRO(D(LeftAlt), T(Tab), U(LeftAlt));

    case M_CTRL_TAB:
      return MACRO(D(LeftControl), T(Tab), U(LeftControl));

    case M_GUI_TAB:
      return MACRO(D(LeftGui), T(Tab), U(LeftGui));

    case M_HYPER:
      return MACRO(D(LeftControl), D(LeftAlt), D(LeftGui), D(LeftShift),
                   U(LeftShift), U(LeftGui), U(LeftAlt), U(LeftControl));

    case M_MEH:
      return MACRO(D(LeftControl), D(LeftAlt), D(LeftShift),
                   U(LeftShift), U(LeftAlt), U(LeftControl));

    // Dygma Defy Wireless, Bluetooth Multi-Host & Battery Level Controls
    // (Dispatched to Dygma BleManager / RadioManager / BatteryStatus on Defy hardware;
    //  also mapped directly to Bazecor Focus keycodes 54102..54110 in EEPROM keymaps)
    case M_BAT_LEVEL:
    case M_BT_PAIR:
    case M_BT_SEL_1:
    case M_BT_SEL_2:
    case M_BT_SEL_3:
    case M_BT_SEL_4:
    case M_BT_SEL_5:
    case M_RF_MODE:
    case M_WL_TOGGLE:
      return MACRO_NONE;

    default:
      return MACRO_NONE;
  }
}

// =============================================================================
// 6. COMBOS & CHORDS (PORTED DIRECTLY FROM MOONLANDER combo0..combo18)
// =============================================================================

enum MoonlanderComboId : uint8_t {
  COMBO_0_ENT_INS = 0, // LT(6,Enter) + LT(10,Insert) -> LSFT(KC_LEFT_CTRL)
  COMBO_1_GAMING_ON,   // KC_B + OSM(MOD_LSFT)        -> TO(8)
  COMBO_2_GAMING_OFF,  // KC_F + KC_C                 -> TO(0)
  COMBO_4_BSPC_DEL,    // LT(3,Bspc) + LT(2,Del)      -> CW_TOGG
  COMBO_5_RINS_TAB,    // LT(10,Ins) + LT(4,Tab)      -> KC_CAPS
  COMBO_6_SPC_TAB,     // LT(1,Space) + LT(4,Tab)     -> CW_TOGG
  COMBO_7_DEL_LINS,    // LT(2,Del) + LT(10,Ins)      -> KC_CAPS
  COMBO_8_ESC_BSPC,    // LT(5,Esc) + LT(3,Bspc)      -> LALT(KC_TAB)
  COMBO_9_ESC_DEL,     // LT(5,Esc) + LT(2,Del)       -> LCTL(KC_TAB)
  COMBO_10_ESC_LINS,   // LT(5,Esc) + LT(10,Ins)      -> LGUI(KC_TAB)
  COMBO_11_ENT_SPC,    // LT(6,Enter) + LT(1,Space)   -> KC_HYPR
  COMBO_12_ENT_TAB,    // LT(6,Enter) + LT(4,Tab)     -> KC_MEH
  COMBO_13_PLAIN_TG    // KC_N + OSM(MOD_RSFT)        -> TG(9)
};

static void tapComboKey(Key k) {
  Runtime.handleKeyEvent(KeyEvent(KeyAddr::none(), IS_PRESSED | INJECTED, k));
  Runtime.handleKeyEvent(KeyEvent(KeyAddr::none(), WAS_PRESSED | INJECTED, k));
}

static void onCombo0EntIns(uint8_t)    { tapComboKey(LSHIFT(Key_LeftControl)); }
static void onCombo1GamingOn(uint8_t)  { Layer.move(LAYER_GAMING); }
static void onCombo2GamingOff(uint8_t) { OSManager.applyOSLayer(); }
static void onComboCapsWord(uint8_t)   { CapsWord.toggle(); }
static void onComboCapsLock(uint8_t)   { tapComboKey(Key_CapsLock); }
static void onCombo8AltTab(uint8_t)    { KeyEvent e(KeyAddr::none(), IS_PRESSED | INJECTED, M(M_ALT_TAB));  macroAction(M_ALT_TAB, e); }
static void onCombo9CtrlTab(uint8_t)   { KeyEvent e(KeyAddr::none(), IS_PRESSED | INJECTED, M(M_CTRL_TAB)); macroAction(M_CTRL_TAB, e); }
static void onCombo10GuiTab(uint8_t)   { KeyEvent e(KeyAddr::none(), IS_PRESSED | INJECTED, M(M_GUI_TAB));  macroAction(M_GUI_TAB, e); }
static void onCombo11Hyper(uint8_t)    { KeyEvent e(KeyAddr::none(), IS_PRESSED | INJECTED, M(M_HYPER));    macroAction(M_HYPER, e); }
static void onCombo12Meh(uint8_t)      { KeyEvent e(KeyAddr::none(), IS_PRESSED | INJECTED, M(M_MEH));      macroAction(M_MEH, e); }
static void onCombo13PlainTg(uint8_t)  { KeyEvent e(KeyAddr::none(), IS_PRESSED | INJECTED, M(M_TG_PLAIN)); macroAction(M_TG_PLAIN, e); }

USE_MAGIC_COMBOS(
  [COMBO_0_ENT_INS]   = { .action = onCombo0EntIns,    .keys = { R3C9,  R4C9  } }, // Right Enter + Right Insert -> Ctrl+Shift
  [COMBO_1_GAMING_ON] = { .action = onCombo1GamingOn,  .keys = { R3C5,  R3C0  } }, // B + Left OSM(Shift)        -> TO(8) Gaming
  [COMBO_2_GAMING_OFF]= { .action = onCombo2GamingOff, .keys = { R2C4,  R3C3  } }, // F + C                      -> TO(0) Base
  [COMBO_4_BSPC_DEL]  = { .action = onComboCapsWord,   .keys = { R4C4,  R4C5  } }, // Left Bksp + Left Del       -> CW_TOGG
  [COMBO_5_RINS_TAB]  = { .action = onComboCapsLock,   .keys = { R4C9,  R4C10 } }, // Right Ins + Right Tab      -> CapsLock
  [COMBO_6_SPC_TAB]   = { .action = onComboCapsWord,   .keys = { R4C11, R4C10 } }, // Right Space + Right Tab    -> CW_TOGG
  [COMBO_7_DEL_LINS]  = { .action = onComboCapsLock,   .keys = { R4C5,  R4C6  } }, // Left Del + Left Ins        -> CapsLock
  [COMBO_8_ESC_BSPC]  = { .action = onCombo8AltTab,    .keys = { R3C6,  R4C4  } }, // Left Esc + Left Bksp       -> Alt+Tab
  [COMBO_9_ESC_DEL]   = { .action = onCombo9CtrlTab,   .keys = { R3C6,  R4C5  } }, // Left Esc + Left Del        -> Ctrl+Tab
  [COMBO_10_ESC_LINS] = { .action = onCombo10GuiTab,   .keys = { R3C6,  R4C6  } }, // Left Esc + Left Ins        -> GUI+Tab
  [COMBO_11_ENT_SPC]  = { .action = onCombo11Hyper,    .keys = { R3C9,  R4C11 } }, // Right Enter + Right Space  -> Hyper
  [COMBO_12_ENT_TAB]  = { .action = onCombo12Meh,      .keys = { R3C9,  R4C10 } }, // Right Enter + Right Tab    -> Meh
  [COMBO_13_PLAIN_TG] = { .action = onCombo13PlainTg,  .keys = { R3C10, R3C15 } }  // N + Right OSM(Shift)       -> TG(9) Plain
);

// =============================================================================
// 7. PLUGIN INITIALIZATION & SETUP
// =============================================================================

KALEIDOSCOPE_INIT_PLUGINS(
  EEPROMSettings,
  OSManager,
  Qukeys,
  Forgiveness,
  CapsWord,
  MagicCombo,
  OneShot,
  Macros,
#ifdef ENABLE_MOUSE_KEYS
  MouseKeys,
#endif
  LEDControl,
  Focus
);

void setup() {
  // ---------------------------------------------------------------------------
  // Configure Home-Row Modifiers & Thumb Layer-Taps (Kaleidoscope-Qukeys)
  // Exact match to Moonlander MT(...) and LT(...) bindings
  // ---------------------------------------------------------------------------
  QUKEYS(
    // =========================================================================
    // Layer 0: macOS Base Layer (CAGS: Control, Alt/Option, GUI/Command, Shift)
    // =========================================================================
    kaleidoscope::plugin::Qukey(LAYER_BASE_MAC, KeyAddr(LEFT_HR_ROW, LEFT_PINKY_COL),   Key_LeftControl), // MT(MOD_LCTL, KC_A)
    kaleidoscope::plugin::Qukey(LAYER_BASE_MAC, KeyAddr(LEFT_HR_ROW, LEFT_RING_COL),    Key_LeftAlt),     // MT(MOD_LALT, KC_S)
    kaleidoscope::plugin::Qukey(LAYER_BASE_MAC, KeyAddr(LEFT_HR_ROW, LEFT_MIDDLE_COL),  Key_LeftGui),     // MT(MOD_LGUI, KC_D)
    kaleidoscope::plugin::Qukey(LAYER_BASE_MAC, KeyAddr(LEFT_HR_ROW, LEFT_INDEX_COL),   Key_LeftShift),   // MT(MOD_LSFT, KC_F)

    kaleidoscope::plugin::Qukey(LAYER_BASE_MAC, KeyAddr(RIGHT_HR_ROW, RIGHT_INDEX_COL),  Key_RightShift),   // MT(MOD_RSFT, KC_J)
    kaleidoscope::plugin::Qukey(LAYER_BASE_MAC, KeyAddr(RIGHT_HR_ROW, RIGHT_MIDDLE_COL), Key_RightGui),     // MT(MOD_RGUI, KC_K)
    kaleidoscope::plugin::Qukey(LAYER_BASE_MAC, KeyAddr(RIGHT_HR_ROW, RIGHT_RING_COL),   Key_LeftAlt),      // MT(MOD_LALT, KC_L)
    kaleidoscope::plugin::Qukey(LAYER_BASE_MAC, KeyAddr(RIGHT_HR_ROW, RIGHT_PINKY_COL),  Key_RightControl), // MT(MOD_RCTL, KC_SCLN)

    // Layer 0 Thumb Layer-Tap Bindings (Exact match to Moonlander LT(...) keys)
    kaleidoscope::plugin::Qukey(LAYER_BASE_MAC, ADDR_LT_ESC,  ShiftToLayer(LAYER_FUNCTION)), // LT(5, KC_ESCAPE)
    kaleidoscope::plugin::Qukey(LAYER_BASE_MAC, ADDR_LT_BSPC, ShiftToLayer(LAYER_CURSOR)),   // LT(3, KC_BSPC)
    kaleidoscope::plugin::Qukey(LAYER_BASE_MAC, ADDR_LT_DEL,  ShiftToLayer(LAYER_NUMBER)),   // LT(2, KC_DELETE)
    kaleidoscope::plugin::Qukey(LAYER_BASE_MAC, ADDR_LT_INS,  ShiftToLayer(LAYER_LOWER)),    // LT(10, KC_INSERT)
    kaleidoscope::plugin::Qukey(LAYER_BASE_MAC, ADDR_RT_ENT,  ShiftToLayer(LAYER_SYS_RGB)),  // LT(6, KC_ENTER)
    kaleidoscope::plugin::Qukey(LAYER_BASE_MAC, ADDR_RT_INS,  ShiftToLayer(LAYER_LOWER)),    // LT(10, KC_INSERT)
    kaleidoscope::plugin::Qukey(LAYER_BASE_MAC, ADDR_RT_TAB,  ShiftToLayer(LAYER_MOUSE)),    // LT(4, KC_TAB)
    kaleidoscope::plugin::Qukey(LAYER_BASE_MAC, ADDR_RT_SPC,  ShiftToLayer(LAYER_SYMBOL)),   // LT(1, KC_SPACE)

    // =========================================================================
    // Layer 11: Windows/Linux Base Layer (GACS: GUI/Win, Alt, Control, Shift)
    // =========================================================================
    kaleidoscope::plugin::Qukey(LAYER_BASE_WIN, KeyAddr(LEFT_HR_ROW, LEFT_PINKY_COL),   Key_LeftGui),     // MT(MOD_LGUI, KC_A)
    kaleidoscope::plugin::Qukey(LAYER_BASE_WIN, KeyAddr(LEFT_HR_ROW, LEFT_RING_COL),    Key_LeftAlt),     // MT(MOD_LALT, KC_S)
    kaleidoscope::plugin::Qukey(LAYER_BASE_WIN, KeyAddr(LEFT_HR_ROW, LEFT_MIDDLE_COL),  Key_LeftControl), // MT(MOD_LCTL, KC_D)
    kaleidoscope::plugin::Qukey(LAYER_BASE_WIN, KeyAddr(LEFT_HR_ROW, LEFT_INDEX_COL),   Key_LeftShift),   // MT(MOD_LSFT, KC_F)

    kaleidoscope::plugin::Qukey(LAYER_BASE_WIN, KeyAddr(RIGHT_HR_ROW, RIGHT_INDEX_COL),  Key_RightShift),   // MT(MOD_RSFT, KC_J)
    kaleidoscope::plugin::Qukey(LAYER_BASE_WIN, KeyAddr(RIGHT_HR_ROW, RIGHT_MIDDLE_COL), Key_RightControl), // MT(MOD_RCTL, KC_K)
    kaleidoscope::plugin::Qukey(LAYER_BASE_WIN, KeyAddr(RIGHT_HR_ROW, RIGHT_RING_COL),   Key_LeftAlt),      // MT(MOD_LALT, KC_L)
    kaleidoscope::plugin::Qukey(LAYER_BASE_WIN, KeyAddr(RIGHT_HR_ROW, RIGHT_PINKY_COL),  Key_RightGui),     // MT(MOD_RGUI, KC_SCLN)

    // Layer 11 Thumb Layer-Tap Bindings
    kaleidoscope::plugin::Qukey(LAYER_BASE_WIN, ADDR_LT_ESC,  ShiftToLayer(LAYER_FUNCTION)), // LT(5, KC_ESCAPE)
    kaleidoscope::plugin::Qukey(LAYER_BASE_WIN, ADDR_LT_BSPC, ShiftToLayer(LAYER_CURSOR)),   // LT(3, KC_BSPC)
    kaleidoscope::plugin::Qukey(LAYER_BASE_WIN, ADDR_LT_DEL,  ShiftToLayer(LAYER_NUMBER)),   // LT(2, KC_DELETE)
    kaleidoscope::plugin::Qukey(LAYER_BASE_WIN, ADDR_LT_INS,  ShiftToLayer(LAYER_LOWER)),    // LT(10, KC_INSERT)
    kaleidoscope::plugin::Qukey(LAYER_BASE_WIN, ADDR_RT_ENT,  ShiftToLayer(LAYER_SYS_RGB)),  // LT(6, KC_ENTER)
    kaleidoscope::plugin::Qukey(LAYER_BASE_WIN, ADDR_RT_INS,  ShiftToLayer(LAYER_LOWER)),    // LT(10, KC_INSERT)
    kaleidoscope::plugin::Qukey(LAYER_BASE_WIN, ADDR_RT_TAB,  ShiftToLayer(LAYER_MOUSE)),    // LT(4, KC_TAB)
    kaleidoscope::plugin::Qukey(LAYER_BASE_WIN, ADDR_RT_SPC,  ShiftToLayer(LAYER_SYMBOL))    // LT(1, KC_SPACE)
  );

  // Apply DIFFICULTY_LEVEL 4 Timing Parameters
  Qukeys.setHoldTimeout(QUKEYS_HOLD_TIMEOUT_MS);                // 175ms tight window
  Qukeys.setOverlapThreshold(QUKEYS_OVERLAP_THRESHOLD_PCT);     // 80% rollover threshold
  Qukeys.setMinimumHoldTime(QUKEYS_MIN_HOLD_TIME_MS);           // 45ms minimum hold
  Qukeys.setMinimumPriorInterval(QUKEYS_MIN_PRIOR_INTERVAL_MS); // 75ms streak idle guard
  Qukeys.activate();

  MagicCombo.min_interval = THUMB_CHORD_TIMEOUT_MS;

#ifdef ENABLE_MOUSE_KEYS
  MouseKeys.speed          = 6;
  MouseKeys.speedDelay     = 1;
  MouseKeys.accelSpeed     = 12;
  MouseKeys.accelDelay     = 64;
  MouseKeys.wheelSpeed     = 2;
  MouseKeys.wheelDelay     = 50;
  MouseKeys.setWarpGridSize(MOUSE_WARP_GRID_2X2);
#endif

  Kaleidoscope.setup();
}

void loop() {
  Kaleidoscope.loop();
}
