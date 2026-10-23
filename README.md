# Glorious Engrammer (QWERTY) for Dygma Defy (70 Keys)

Complete custom **Kaleidoscope** firmware port of Sunaku's [Glorious Engrammer](https://github.com/sunaku/glove80-keymaps) (`QWERTY` variant), mapped 1-to-1 from your **ZSA Moonlander** layout (`zsa_moonlander_bruno-s-first-layout-fork_source`) onto the **Dygma Defy (70-key split columnar keyboard)**.

---

## 1. How the 72-Key Moonlander Maps 1-to-1 Onto the 70-Key Dygma Defy

* **Main Finger Grid (54 keys = 27 per side — 100% identical to Moonlander Rows 0–3):**
  * **Row 0 (14 keys = 7 Left + 7 Right):**
    * Left: `CapsLock`, `1`, `2`, `3`, `4`, `5`, `F11` (inner col)
    * Right: `F12` (inner col), `6`, `7`, `8`, `9`, `0`, `CapsLock`
  * **Row 1 (14 keys = 7 Left + 7 Right):**
    * Left: `CW_TOGG` (`Caps-Word Toggle`), `Q`, `W`, `E`, `R`, `T`, `PageUp` (inner col)
    * Right: `WheelDown` (inner col), `Y`, `U`, `I`, `O`, `P`, `-`
  * **Row 2 (14 keys = 7 Left + 7 Right):**
    * Left: `=`, `A (LCtrl/LGui)`, `S (LAlt)`, `D (LGui/LCtrl)`, `F (LShift)`, `G`, `PageDown` (inner col)
    * Right: `WheelUp` (inner col), `H`, `J (RShift)`, `K (RGui/RCtrl)`, `L (LAlt)`, `; (RCtrl/RGui)`, `'`
  * **Row 3 (12 keys = 6 Left + 6 Right):**
    * Left: `OSM(LShift)`, `Z`, `X`, `C`, `V`, `B`
    * Right: `N`, `M`, `,`, `.`, `/`, `OSM(RShift)`
* **Thumb Cluster & Extra Row (16 keys = 8 per side):**
  *(The only 2-key difference between the 72-key Moonlander and 70-key Defy is that Moonlander had duplicate `OSM(MOD_LSFT)` / `OSM(MOD_RSFT)` keys on both Row 3 and Row 4; removing that single duplicate per half yields an exact 35-key per half match)*:
  * **Left Upper Thumb Arc (4 core layer-tap keys):**
    * `LT(3, Backspace)` -> Tap: `Backspace`, Hold: **Layer 3 (Cursor)**
    * `LT(2, Delete)`    -> Tap: `Delete`,    Hold: **Layer 2 (Number)**
    * `LT(10, Insert)`   -> Tap: `Insert`,    Hold: **Layer 10 (Lower)**
    * `LT(5, Escape)`    -> Tap: `Escape`,    Hold: **Layer 5 (Function / Media)**
  * **Left Lower Thumb Arc (4 extra keys from Moonlander Row 4):**
    * `MO(7)`            -> Hold: **Layer 7 (Magic / Boot / OS Switch)**
    * `\` (`Key_Backslash`)
    * `DUAL_FUNC_0`      -> Tap: `(`, Hold: `<`
    * `DUAL_FUNC_1`      -> Tap: `)`, Hold: `>`
  * **Right Upper Thumb Arc (4 core layer-tap keys, inner to outer):**
    * `LT(6, Enter)`     -> Tap: `Enter`,     Hold: **Layer 6 (System / RGB / Power)**
    * `LT(10, Insert)`   -> Tap: `Insert`,    Hold: **Layer 10 (Lower)**
    * `LT(4, Tab)`       -> Tap: `Tab`,       Hold: **Layer 4 (Mouse)**
    * `LT(1, Space)`     -> Tap: `Space`,     Hold: **Layer 1 (Symbol)**
  * **Right Lower Thumb Arc (4 extra keys from Moonlander Row 4, inner to outer):**
    * `[` (`Key_LeftBracket`)
    * `]` (`Key_RightBracket`)
    * `` ` `` (`Key_Backtick`)
    * `MO(7)`            -> Hold: **Layer 7 (Magic / Boot / OS Switch)**

---

## 2. 70-Key Keymap Matrix Diagrams

### Layer 0 (`LAYER_BASE_MAC`): Base QWERTY — macOS (`CAGS` Home-Row Mods)
*(Layer 11 `LAYER_BASE_WIN` is identical except Home-Row Mods swap `Ctrl` and `Gui` to `GACS`: `A=LGui, S=LAlt, D=LCtrl, F=LShift` | `J=RShift, K=RCtrl, L=LAlt, ;=RGui`)*

```text
+----------------------------------------------------------------------------------+   +----------------------------------------------------------------------------------+
| CapsLock |    1     |    2     |    3     |    4     |    5     |   F11    |     |   |     |   F12    |    6     |    7     |    8     |    9     |    0     | CapsLock |
|----------+----------+----------+----------+----------+----------+----------|     |   |     |----------+----------+----------+----------+----------+----------+----------|
| CW_TOGG  |    Q     |    W     |    E     |    R     |    T     |  PageUp  |     |   |     | WheelDn  |    Y     |    U     |    I     |    O     |    P     |    -     |
|----------+----------+----------+----------+----------+----------+----------|     |   |     |----------+----------+----------+----------+----------+----------+----------|
|    =     | A (LCtl) | S (LAlt) | D (LGui) | F (LSft) |    G     |  PageDn  |     |   |     | WheelUp  |    H     | J (RSft) | K (RGui) | L (LAlt) | ; (RCtl) |    '     |
|----------+----------+----------+----------+----------+----------+----------+     |   |     +----------+----------+----------+----------+----------+----------+----------|
| OSM(LSf) |    Z     |    X     |    C     |    V     |    B     |                |   |                |    N     |    M     |    ,     |    .     |    /     | OSM(RSf) |
+-----------------------------------------------------------------+                |   |                +-----------------------------------------------------------------+
                      +----------+----------+----------+----------+                |   |                +----------+----------+----------+----------+
       Thumb Upper:   | Bksp/L3  |  Del/L2  | Ins/L10  |  Esc/L5  |                |   |                |  Ent/L6  | Ins/L10  |  Tab/L4  | Space/L1 |
                      +----------+----------+----------+----------+                |   |                +----------+----------+----------+----------+
       Thumb Lower:   |  MO(7)   |    \     | ( / <    | ) / >    |                |   |                |    [     |    ]     |    `     |  MO(7)   |
                      +----------+----------+----------+----------+                +   +                +----------+----------+----------+----------+
```

---

### Layer 1 (`LAYER_SYMBOL`): Programming Symbol Layer — Held via Right Thumb `LT(1, Space)`

```text
+----------------------------------------------------------------------------------+   +----------------------------------------------------------------------------------+
|   ___    |    `     |    (     |    )     |    ;     |    ,     |   ___    |     |   |     |   ___    |   ___    |   ___    |   ___    |   ___    |   ___    |   ___    |
|----------+----------+----------+----------+----------+----------+----------|     |   |     |----------+----------+----------+----------+----------+----------+----------|
|    !     |    [     |    {     |    }     |    ]     |    ?     |   ___    |     |   |     |   ___    |    .     |   Left   |    Up    |   Down   |  Right   |   ___    |
|----------+----------+----------+----------+----------+----------+----------|     |   |     |----------+----------+----------+----------+----------+----------+----------|
|    #     |    ^     |    =     |    _     |    $     |    *     |    .     |     |   |     |   ___    |    ,     |   Bksp   |   Tab    |  Space   |  Enter   |   ```    |
|----------+----------+----------+----------+----------+----------+----------+     |   |     +----------+----------+----------+----------+----------+----------+----------|
|    ~     |    <     |    |     |    -     |    >     |    /     |                |   |                |    ;     |    "     |    '     |    `     |    \     |   ___    |
+-----------------------------------------------------------------+                |   |                +-----------------------------------------------------------------+
                      +----------+----------+----------+----------+                |   |                +----------+----------+----------+----------+
       Thumb Upper:   |    %     |    :     |   ___    |    \     |                |   |                |   ___    |   ___    |   ___    |  (Held)  |
                      +----------+----------+----------+----------+                |   |                +----------+----------+----------+----------+
       Thumb Lower:   |    ..    |    &     |    '     |    "     |                |   |                |   LGui   |   LAlt   |  LCtrl   |   ___    |
                      +----------+----------+----------+----------+                +   +                +----------+----------+----------+----------+
```

---

### Layer 2 (`LAYER_NUMBER`): Number & Numpad Layer — Held via Left Thumb `LT(2, Delete)`

```text
+----------------------------------------------------------------------------------+   +----------------------------------------------------------------------------------+
|   ___    |   Esc    |   Ins    |  S-Tab   |   Del    |   ___    |   ___    |     |   |     |   ___    |    |     |    #     |    $     |    ^     |    ~     |    !     |
|----------+----------+----------+----------+----------+----------+----------|     |   |     |----------+----------+----------+----------+----------+----------+----------|
|   ___    |  Enter   |  Space   |   Tab    |   Bksp   |   KP_0   |   ___    |     |   |     |    >     |    G     |   KP_7   |   KP_8   |   KP_9   |    :     |    %     |
|----------+----------+----------+----------+----------+----------+----------|     |   |     |----------+----------+----------+----------+----------+----------+----------|
| OSM(LSf) |  LCtrl   |   LAlt   |   LGui   |  LShift  |    X     |   ___    |     |   |     |    <     |    K     |   KP_4   |   KP_5   |   KP_6   |   KP_-   |   KP_+   |
|----------+----------+----------+----------+----------+----------+----------+     |   |     +----------+----------+----------+----------+----------+----------+----------|
|    E     |  SelAll  | SelLine  | SelWord  |   Find   |    F     |                |   |                |    J     |   KP_1   |   KP_2   |   KP_3   |   KP_/   |   KP_*   |
+-----------------------------------------------------------------+                |   |                +-----------------------------------------------------------------+
                      +----------+----------+----------+----------+                |   |                +----------+----------+----------+----------+
       Thumb Upper:   |   ___    |  (Held)  |   ___    | FindNext |                |   |                |    ,     |    =     |   KP_.   |   KP_0   |
                      +----------+----------+----------+----------+                |   |                +----------+----------+----------+----------+
       Thumb Lower:   |    _     |   Undo   |   Redo   | FindPrev |                |   |                |    (     |   ___    |    )     |    @     |
                      +----------+----------+----------+----------+                +   +                +----------+----------+----------+----------+
```

---

### Layer 3 (`LAYER_CURSOR`): Cursor & Navigation Layer — Held via Left Thumb `LT(3, Backspace)`

```text
+----------------------------------------------------------------------------------+   +----------------------------------------------------------------------------------+
|   ___    |   Esc    |   Ins    |  S-Tab   |   Del    |   Ins    |   ___    |     |   |     |   ___    |   Ins    |   ___    |   ___    |   ___    |   ___    |   ___    |
|----------+----------+----------+----------+----------+----------+----------|     |   |     |----------+----------+----------+----------+----------+----------+----------|
|   ___    |  Enter   |  Space   |   Tab    |   Bksp   |   Cut    |   ___    |     |   |     |   ___    |   Cut    |   Bksp   |   Undo   |   Redo   |   Tab    |   ___    |
|----------+----------+----------+----------+----------+----------+----------|     |   |     |----------+----------+----------+----------+----------+----------+----------|
| OSM(LSf) |  LCtrl   |   LAlt   |   LGui   |  LShift  |   Copy   |   ___    |     |   |     |   ___    |   Copy   |   Left   |    Up    |   Down   |  Right   | OSM(LSf) |
|----------+----------+----------+----------+----------+----------+----------+     |   |     +----------+----------+----------+----------+----------+----------+----------|
| GotoLine |  SelAll  | SelLine  | SelWord  |   Find   |  Paste   |                |   |                |  Paste   |   Home   |  PageUp  |  PageDn  |   End    | GotoLine |
+-----------------------------------------------------------------+                |   |                +-----------------------------------------------------------------+
                      +----------+----------+----------+----------+                |   |                +----------+----------+----------+----------+
       Thumb Upper:   |  (Held)  |   ___    |   ___    | FindNext |                |   |                | OptSftRt |  SelAll  | SelLine  | SelWord  |
                      +----------+----------+----------+----------+                |   |                +----------+----------+----------+----------+
       Thumb Lower:   | QuickOpn |   Undo   |   Redo   | FindPrev |                |   |                |   Find   | FindPrev | FindNext | FindRepl |
                      +----------+----------+----------+----------+                +   +                +----------+----------+----------+----------+
```

---

## 3. Compilation & Flashing Guide (`arduino-cli`)

### Step 1: Install `arduino-cli` & Dygma Defy Core Dependencies

```bash
sudo apt-get update && sudo apt-get install -y arduino-cli dfu-util picotool git

arduino-cli config init
arduino-cli config add board_manager.additional_urls \
  https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
arduino-cli core update-index
arduino-cli core install rp2040:rp2040

mkdir -p ~/Arduino/hardware/dygma ~/Arduino/libraries
git clone --recursive https://github.com/Dygmalab/Kaleidoscope.git ~/Arduino/libraries/Kaleidoscope
```

### Step 2: Compile `EngrammerDefy.ino`

```bash
cd dygma-defy-engrammer

arduino-cli compile \
  --fqbn dygma:rp2040:defy \
  --build-property "build.extra_flags=-DDYGMA_DEFY=1" \
  --build-path ./build \
  --output-dir ./dist \
  EngrammerDefy.ino
```

### Step 3: Flash Over USB Targeting the Dygma Neuron Bootloader

1. Connect the Defy halves and Neuron directly over USB-C.
2. Enter bootloader mode via Focus serial (`printf "device.bootloader\n" > /dev/ttyACM0`) or by holding the top-left key (`CapsLock`) while plugging in USB.
3. Flash the compiled firmware:
   ```bash
   arduino-cli upload --fqbn dygma:rp2040:defy --port /dev/ttyACM0 --input-dir ./dist
   # Or via UF2 / picotool:
   picotool load -x ./dist/EngrammerDefy.ino.uf2
   ```

---

## 4. License & Attributions

This repository is distributed under the **GNU General Public License v3.0 (`GPL-3.0-or-later`)** with keymap design elements under the **MIT License**. See [LICENSE](./LICENSE) for full license texts.

* **Dygma Defy Hardware Definitions, SVG Geometry & Focus Protocol**:
  * Adapted from [DygmaLab/Bazecor](https://github.com/Dygmalab/Bazecor) and [DygmaLab/Kaleidoscope](https://github.com/Dygmalab/Kaleidoscope) — Copyright © DygmaLab SE & Keyboardio, Inc. (`GPL-3.0`).
* **Glorious Engrammer Keymap & Symbol Layer Architecture**:
  * Adapted from [sunaku/glove80-keymaps](https://github.com/sunaku/glove80-keymaps) — Copyright © Suraj N. Kurapati (`MIT License`).
