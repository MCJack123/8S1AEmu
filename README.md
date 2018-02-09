# 8S1AEmu
An emulator for my WIP Minecraft 8-bit CPU.

# The 8S1A Chip
* 8-bit CPU
* 256 bytes of cache-memory
* 8 instructions
* Extensible to allow more instructions

## Instructions
* `0x00 <addend1> <addend2>`: Add `addend1` and `addend2`
* `0x01 <first> <second>`: Subtract `second` from `first`
* `0x02 <position> <value>`: Set `position` to `value`
* `0x03 <num1> <num2> <instructions...> 0xFE`:    Run `instructions` if `num1` equals `num2`
* `0x04 <num1> <num2> <instructions...> 0xFE`:    Run `instructions` if `num1` does not equal `num2`
* `0x05 <num1> <num2> <instructions...> 0xFE`:    Run `instructions` if `num1` is greater than `num2`
* `0x06 <num1> <num2> <instructions...> 0xFE`:    Run `instructions` if `num1` is less than `num2`
* `0x07 <number>`:                                Send `number` to output
* Variables can be used by replacing the number with `0xFF` and then the memory position.

# The 8H1A Extension
* Storage extension for 8S1A
* 16-bit address space
* 4 new instructions

## Instructions
Note: `<b1> <b0>` indicates a 16-bit number split into two 8-bit numbers (ex: 16-bit number `1100110111011111` would be two 8-bit numbers `11001101 11011111`)
* `0x08 <b1> <b0> <pos>`:                    Load value `b` from drive to `pos` in memory
* `0x09 <pos> <b1> <b0>`:                    Save value of `pos` to drive position `b`
* `0x0A <b1> <b0>`:                          Run instructions from memory starting at `b`
* `0x0B`: Stop memory execution (only in programs)

# How to build
Requires ncurses to build the emulator
* `$ cd emu`
* `$ g++ -o 8H1A ../8H1A/8H1A.cpp ../8S1A/8S1A.cpp 8H1AEmu.cpp -lncurses`
* `$ g++ -o as-8s1a as-8s1a.cpp`

# Assembly Syntax
Works mainly like normal assembly, but has a few differences.
* The `.data` and `.bss` sections must come before the `.text` section
* Programs must end with the `ret` instruction
* Positions in CPU memory are given with a `$` before the position
* Comments can also be placed before the first section and after `ret`

## Instructions
* `add -> 0x00`
* `sub -> 0x01`
* `set -> 0x02`
* `ieq -> 0x03`
* `ine -> 0x04`
* `igt -> 0x05`
* `ilt -> 0x06`
* `set -> 0x07`
* `lod -> 0x08`
* `sav -> 0x09`
* `jmp -> 0x0A`
* `ret -> 0x0B`
* `end -> 0xFE`
