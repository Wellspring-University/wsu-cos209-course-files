# Arduino Uno sketches

WSU-COS 209 The Circuit-to-Cloud Workshop. (c) 2026 Wellspring University, Faculty of Computing.

Open a sketch in the Arduino IDE (the folder name must match the `.ino` name), choose **Tools > Board > Arduino Uno**
and the right port, upload, then open **Tools > Serial Monitor** at **115200 baud**.
Drive pins are D2 upward, chip outputs are read on D8 upward unless the table says otherwise.
Switches and buttons use the internal pull-up resistors (`INPUT_PULLUP`): pressed or ON reads LOW.
Every chip needs power: connect its VCC pin to 5V and its GND pin to GND, and tie every unused input to GND or 5V.
Test sketches print a table and a last line `RESULT: PASS (n/n)` or `RESULT: FAIL (k/n)`; press the Uno RESET button to run again.

Sketches marked **starter** contain `TODO` comments for you to finish; the other sketches are complete and are
used as given.

| Sketch | Practical | What it does | Wiring summary |
|---|---|---|---|
| `w01_blink` | Week 1 | Built-in LED and an external LED blink alternately every 500 ms; prints `Hello, COS 209! Blink n`. | LED on D7 via 330 ohm to GND |
| `p01_dip_reader` (starter) | P1 Number systems | Reads an 8-way DIP switch and prints binary, hex, decimal (you add signed, Gray, BCD). | Switch 1..8 = D2..D9 (bit 0 = D2), other side GND |
| `p02_gate_tester` (starter) | P2 Gates | Tests gate 1 of a 74HC00/08/32/86; serial `0` NAND, `8` AND, `3` OR, `6` XOR. You add `a` (all four gates). | D2 = A (pin 1), D3 = B (pin 2), D8 = Y (pin 3) |
| `p03_prime_digit` | P3 K-maps | Tests F = B.C' + C.A (prime digits 2, 3, 5, 7) built from one 74HC00, hardware vs formula. | A,B,C,D = D2..D5, F (pin 11) -> D8 |
| `p04_adder_test` | P4 Adders | Tests all 512 cases of a 74HC283 and prints failures, a summary and a JSON line. | A0..A3 = D2..D5, B0..B3 = D6..D9, Cin = D10; S0..S3 -> A0..A3, Cout -> A4 |
| `p04_comparator_test` | P4 Comparators | Tests all 256 cases of a 74HC85 (cascade A=B tied high). | A0..A3 = D2..D5, B0..B3 = D6..D9; A>B, A=B, A<B -> A0, A1, A2 |
| `p05_decoder_test` | P5 Decoders | Tests the active-low outputs of a 74HC138 for inputs 0 to 7. | A0..A2 = D2..D4; /E1, /E2 GND, E3 5V; Y0..Y7 -> D6..D13 |
| `p05_bcd_counter` (starter) | P5 Displays | Counts 0 to 9 once a second on a 74HC4511 + 7-segment display (you add the ROM look-up mode, commands `r` and `h`). | A,B,C,D = D2..D5; /LT, /BL 5V; LE GND; 330 ohm per segment |
| `p05_mux_logic` | P5 Multiplexers | Checks P3's F built on a 74HC151 (inputs hard-wired 0,0,1,1,0,1,0,1). | S0..S2 = D2..D4; /E GND; Y -> D8 |
| `p06_bounce_counter` (starter) | P6 Latches, bounce | Counts raw button edges (bounce) against the clean output of a 74HC00 SR latch; `z` zeroes. You add the software-debounced count. | Button D2 to GND; latch Q -> D3; S and R buttons with 10k pull-ups |
| `p06_dff_test` | P6 Flip-flops | Proves a 74HC74 flip-flop copies D only on the rising clock edge. | D = D2, CLK = D3; Q -> D8, /Q -> D9; /PRE, /CLR 5V |
| `p07_counter_test` | P7 Counters | Clocks a 74HC161 20 times and checks count and TC; `s` slow mode (500 ms), `f` fast. | CP = D3, /MR = D4; Q0..Q3 -> D8..D11, TC -> D12; CEP, CET, /PE 5V; D0..D3 GND |
| `p07_shift_patterns` | P7 Shift registers | Plays LED patterns on a 74HC595 (Knight Rider); `p 01 03 07 0F` loads a new pattern. | DS = D11, SHCP = D12, STCP = D8; /OE GND; /MR 5V; 8 LEDs with 330 ohm |
| `p07_tristate_bus` | P7 Buses | Two 74HC125 buffers share one bus line; tests each alone and none (pull-down). Never enables both. | A1 = D2, A2 = D3, /OE1 = D4, /OE2 = D5; bus -> D8 and 10k to GND |
| `p08_traffic_fsm` | P8 State machines | Non-blocking traffic light + pedestrian crossing; logs every transition; serial `r` = button press. | Red D5, Amber D6, Green D7, Walk D9, Don't walk D10 (330 ohm each); button D2 |
| `p09_sensor_stream` | P9 Web Serial | Sends `{"t":..,"light":..,"button":..,"count":..}` every 500 ms; `L1` / `L0` switch the LED. | LDR + 10k divider on A0; button D2; LED D7 via 330 ohm |

Chip pin numbers are in the comment block at the top of each sketch (checked against the 74HC datasheets).
Close the Serial Monitor before using the P9 web dashboard: only one program can open the port.
