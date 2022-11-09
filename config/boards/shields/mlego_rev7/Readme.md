## build

west build -d mlego2 -b blackpill_f401ce -- -DSHIELD=mlego5x13

## Pins and leds rev4 stm32f401

the pinout is the same for stm32f411

| Rows  | C0    | C1    | C2    | C3    | C4    | C5    | C6    | C7    | C8    | C9    | C10   | C11   | C12   | Pins  |
| :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| R0    | Esc   | 1     | 2     | 3     | 4     | 5     | 6     | 7     | 8     | 9     | 0     | -     | Bksp  | B10   |
| R1    | Tab   | q     | w     | e     | r     | t     | y     | u     | i     | o     | p     | [     | ]     | A5    |
| R2    | #     | a     | s     | d     | f     | g     | h     | j     | k     | l     | ;     | '     | Enter | A6    |
| R3    | Shift | \     | z     | x     | c     | v     | b     | n     | m     | ,     | .     | Up    | /     | A7    |
| R4    | Ctrl  | Menu  | Lower | Alt   | Raise | Space | Space | Space | AltGr | Shift | Left  | Down  | Right | B0    |
|       | B14   | A8    | A10   | A15   | B3    | B4    | B5    | B7    | A1    | A2    | A3    | A4    | B1    |       |

### Encoders

  - Pad_A: A0
  - Pad_B: B6

### Oled

  - SDA: B9
  - SCL/SCK: B8

### LEDS

| Leds        | Pin |
| ----------- | --- |
| NUM_LOCK    | B12 |
| CAPS_LOCK   | C13 |
| SCROLL_LOCK | B13 |
| RBG_DI      | B15 |

