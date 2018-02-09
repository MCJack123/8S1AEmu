#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <iomanip>
#include <stdint.h>
#include <math.h>
#include <ncurses.h>
#include "../8H1A/8H1A.hpp"

C8H1A cpu(0);
std::string data;
bool ascii;
int size;

std::vector<std::string> split(const std::string &s, char delim) {
  std::stringstream ss(s);
  std::string item;
  std::vector<std::string> elems;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
    // elems.push_back(std::move(item)); // if C++11 (based on comment from @mchiasson)
  }
  return elems;
}

std::pair<byte, byte> splitByte(byte16 in) {
    byte a;
    byte b;
    a[7] = in[15];
    a[6] = in[14];
    a[5] = in[13];
    a[4] = in[12];
    a[3] = in[11];
    a[2] = in[10];
    a[1] = in[9];
    a[0] = in[8];
    b[7] = in[7];
    b[6] = in[6];
    b[5] = in[5];
    b[4] = in[4];
    b[3] = in[3];
    b[2] = in[2];
    b[1] = in[1];
    b[0] = in[0];
    return std::make_pair(a, b);
}

std::string intToHex( unsigned long i , size_t width )
{
  std::stringstream stream;
  stream << std::setfill ('0') << std::setw(width) << std::hex << i;
  return stream.str();
}

char makeChar(unsigned long l) {
    if (l >= 32 && l < 128) return (char)l;
    else return '.';
}

/*
in 80x24 format:
--------------------------------------------------------------------------------
| 0x0000 | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  | ................ |
| 0x0010 | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  | ................ |
| 0x0020 | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  | ................ |
| 0x0030 | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  | ................ |
| 0x0040 | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  | ................ |
| 0x0050 | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  | ................ |
| 0x0060 | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  | ................ |
| 0x0070 | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  | ................ |
| 0x0080 | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  | ................ |
| 0x0090 | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  | ................ |
| 0x0100 | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  | ................ |
| 0x0110 | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  | ................ |
| 0x0120 | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  | ................ |
| 0x0130 | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  | ................ |
| 0x0140 | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  | ................ |
| 0x0150 | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  | ................ |
| 0x0160 | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  | ................ |
| 0x0170 | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  | ................ |
| 0x0180 | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  | ................ |
| 0x0190 | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  | ................ |
| 0x0200 | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  | ................ |
| 0x0210 | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  | ................ |
--------------------------------------------------------------------------------
*/

std::map<byte16, byte> memEdit(std::map<byte16, byte> mem, int mapsize) {
    mapsize--;
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    for (int x = 0; x < COLS; x++) {
        mvaddch(0, x, '-');
        mvaddch(LINES-1, x, '-');
    }
    for (int y = 1; y < LINES - 1; y++) {
        mvaddch(y, 0, '|');
        mvaddch(y, 9, '|');
        mvaddch(y, COLS-1, '|');
        mvaddch(y, COLS-20, '|');
        mvaddch(y, 2, '0');
        mvaddch(y, 3, 'x');
        refresh();
    }
    unsigned int current_offset = 0x0000;
    int typing = -1;
    unsigned int top_offset = 0x0000;
    unsigned int bottom_offset = 0x0010 * (LINES - 3);
    while (true) {
        for (int y = 1; y < LINES - 1; y++) {
            mvaddstr(y, 4, intToHex(0x0010 * (y - 1) + top_offset, 4).c_str());
            for (int off = 0; off < 16; off++) {
                mvaddstr(y, 11 + (3*off), intToHex((uint8_t)mem[byte16((0x0010 * (y - 1)) + off + top_offset)].to_ulong(), 2).c_str());
                mvaddch(y, COLS - 2 - (16-off), makeChar(mem[byte16((0x0010 * (y - 1)) + off + top_offset)].to_ulong()));
            }
        }
        refresh();
        while (true) {
            bool exit = false;
            const char * curhex = intToHex((uint8_t)mem[byte16(current_offset)].to_ulong(), 2).c_str();
            //mvaddstr(0, 0, intToHex(bottom_offset, 4).c_str());
            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, curhex[0] | A_STANDOUT);
            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 12, curhex[1] | A_STANDOUT);
            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, COLS - 2 - (16-(current_offset % 0x0010)), makeChar(mem[byte16(current_offset)].to_ulong()) | A_STANDOUT);
            refresh();
            Loop:
                int ch = getch();
                switch (ch) {
                    case KEY_UP:
                        if (current_offset >= 0x0010) {
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, curhex[0]);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 12, curhex[1]);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, COLS - 2 - (16-(current_offset % 0x0010)), makeChar(mem[byte16(current_offset)].to_ulong()));
                            current_offset -= 0x0010;
                        } else break;
                        if (top_offset > current_offset) {
                            top_offset -= 0x0010;
                            bottom_offset -= 0x0010;
                            int oldx, oldy;
                            getyx(stdscr, oldy, oldx);
                            move(LINES - 1, oldx);
                            exit = true;
                        }
                        break;
                    case KEY_DOWN:
                        if (current_offset <= mapsize - 0x0010) {
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, curhex[0]);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 12, curhex[1]);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, COLS - 2 - (16-(current_offset % 0x0010)), makeChar(mem[byte16(current_offset)].to_ulong()));
                            current_offset += 0x0010;
                        } else break;
                        if (bottom_offset < current_offset) {
                            bottom_offset += 0x0010;
                            top_offset += 0x0010;
                            int oldx, oldy;
                            getyx(stdscr, oldy, oldx);
                            move(LINES - 1, oldx);
                            refresh();
                            exit = true;
                        }
                        break;
                    case KEY_LEFT:
                        if (current_offset > 0x0000 && current_offset % 0x0010 > 0) {
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, curhex[0]);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 12, curhex[1]);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, COLS - 2 - (16-(current_offset % 0x0010)), makeChar(mem[byte16(current_offset)].to_ulong()));
                            current_offset--;
                        }
                        break;
                    case KEY_RIGHT:
                        if (current_offset < mapsize && current_offset % 16 < 15) {
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, curhex[0]);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 12, curhex[1]);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, COLS - 2 - (16-(current_offset % 0x0010)), makeChar(mem[byte16(current_offset)].to_ulong()));
                            current_offset++;
                        }
                        break;
                    case '0':
                        if (typing == -1) {
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, ch | A_STANDOUT);
                            typing = 0;
                            refresh();
                            goto Loop;
                        } else {
                            mem[byte16(current_offset)] = byte((typing * 16) + 0);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 12, ch);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, intToHex(typing, 1).c_str()[0]);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, COLS - 2 - (16-(current_offset % 0x0010)), makeChar(mem[byte16(current_offset)].to_ulong()));
                            typing = -1;
                            current_offset++;
                        }
                        break;
                    case '1':
                        if (typing == -1) {
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, ch | A_STANDOUT);
                            typing = 1;
                            refresh();
                            goto Loop;
                        } else {
                            mem[byte16(current_offset)] = byte((typing * 16) + 1);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 12, ch);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, intToHex(typing, 1).c_str()[0]);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, COLS - 2 - (16-(current_offset % 0x0010)), makeChar(mem[byte16(current_offset)].to_ulong()));
                            typing = -1;
                            current_offset++;
                        }
                        break;
                    case '2':
                        if (typing == -1) {
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, ch | A_STANDOUT);
                            typing = 2;
                            refresh();
                            goto Loop;
                        } else {
                            mem[byte16(current_offset)] = byte((typing * 16) + 2);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 12, ch);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, intToHex(typing, 1).c_str()[0]);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, COLS - 2 - (16-(current_offset % 0x0010)), makeChar(mem[byte16(current_offset)].to_ulong()));
                            typing = -1;
                            current_offset++;
                        }
                        break;
                    case '3':
                        if (typing == -1) {
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, ch | A_STANDOUT);
                            typing = 3;
                            refresh();
                            goto Loop;
                        } else {
                            mem[byte16(current_offset)] = byte((typing * 16) + 3);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 12, ch);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, intToHex(typing, 1).c_str()[0]);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, COLS - 2 - (16-(current_offset % 0x0010)), makeChar(mem[byte16(current_offset)].to_ulong()));
                            typing = -1;
                            current_offset++;
                        }
                        break;
                    case '4':
                        if (typing == -1) {
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, ch | A_STANDOUT);
                            typing = 4;
                            refresh();
                            goto Loop;
                        } else {
                            mem[byte16(current_offset)] = byte((typing * 16) + 4);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 12, ch);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, intToHex(typing, 1).c_str()[0]);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, COLS - 2 - (16-(current_offset % 0x0010)), makeChar(mem[byte16(current_offset)].to_ulong()));
                            typing = -1;
                            current_offset++;
                        }
                        break;
                    case '5':
                        if (typing == -1) {
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, ch | A_STANDOUT);
                            typing = 5;
                            refresh();
                            goto Loop;
                        } else {
                            mem[byte16(current_offset)] = byte((typing * 16) + 5);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 12, ch);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, intToHex(typing, 1).c_str()[0]);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, COLS - 2 - (16-(current_offset % 0x0010)), makeChar(mem[byte16(current_offset)].to_ulong()));
                            typing = -1;
                            current_offset++;
                        }
                        break;
                    case '6':
                        if (typing == -1) {
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, ch | A_STANDOUT);
                            typing = 6;
                            refresh();
                            goto Loop;
                        } else {
                            mem[byte16(current_offset)] = byte((typing * 16) + 6);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 12, ch);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, intToHex(typing, 1).c_str()[0]);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, COLS - 2 - (16-(current_offset % 0x0010)), makeChar(mem[byte16(current_offset)].to_ulong()));
                            typing = -1;
                            current_offset++;
                        }
                        break;
                    case '7':
                        if (typing == -1) {
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, ch | A_STANDOUT);
                            typing = 7;
                            refresh();
                            goto Loop;
                        } else {
                            mem[byte16(current_offset)] = byte((typing * 16) + 7);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 12, ch);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, intToHex(typing, 1).c_str()[0]);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, COLS - 2 - (16-(current_offset % 0x0010)), makeChar(mem[byte16(current_offset)].to_ulong()));
                            typing = -1;
                            current_offset++;
                        }
                        break;
                    case '8':
                        if (typing == -1) {
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, ch | A_STANDOUT);
                            typing = 8;
                            refresh();
                            goto Loop;
                        } else {
                            mem[byte16(current_offset)] = byte((typing * 16) + 8);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 12, ch);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, intToHex(typing, 1).c_str()[0]);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, COLS - 2 - (16-(current_offset % 0x0010)), makeChar(mem[byte16(current_offset)].to_ulong()));
                            typing = -1;
                            current_offset++;
                        }
                        break;
                    case '9':
                        if (typing == -1) {
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, ch | A_STANDOUT);
                            typing = 9;
                            refresh();
                            goto Loop;
                        } else {
                            mem[byte16(current_offset)] = byte((typing * 16) + 9);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 12, ch);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, intToHex(typing, 1).c_str()[0]);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, COLS - 2 - (16-(current_offset % 0x0010)), makeChar(mem[byte16(current_offset)].to_ulong()));
                            typing = -1;
                            current_offset++;
                        }
                        break;
                    case 'a':
                        if (typing == -1) {
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, 'a' | A_STANDOUT);
                            typing = 10;
                            refresh();
                            goto Loop;
                        } else {
                            mem[byte16(current_offset)] = byte((typing * 16) + 10);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 12, 'a');
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, intToHex(typing, 1).c_str()[0]);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, COLS - 2 - (16-(current_offset % 0x0010)), makeChar(mem[byte16(current_offset)].to_ulong()));
                            typing = -1;
                            current_offset++;
                        }
                        break;
                    case 'b':
                        if (typing == -1) {
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, 'b' | A_STANDOUT);
                            typing = 11;
                            refresh();
                            goto Loop;
                        } else {
                            mem[byte16(current_offset)] = byte((typing * 16) + 11);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 12, 'b');
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, intToHex(typing, 1).c_str()[0]);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, COLS - 2 - (16-(current_offset % 0x0010)), makeChar(mem[byte16(current_offset)].to_ulong()));
                            typing = -1;
                            current_offset++;
                        }
                        break;
                    case 'c':
                        if (typing == -1) {
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, 'c' | A_STANDOUT);
                            typing = 12;
                            refresh();
                            goto Loop;
                        } else {
                            mem[byte16(current_offset)] = byte((typing * 16) + 12);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 12, 'c');
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, intToHex(typing, 1).c_str()[0]);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, COLS - 2 - (16-(current_offset % 0x0010)), makeChar(mem[byte16(current_offset)].to_ulong()));
                            typing = -1;
                            current_offset++;
                        }
                        break;
                    case 'd':
                        if (typing == -1) {
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, 'd' | A_STANDOUT);
                            typing = 13;
                            refresh();
                            goto Loop;
                        } else {
                            mem[byte16(current_offset)] = byte((typing * 16) + 13);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 12, 'd');
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, intToHex(typing, 1).c_str()[0]);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, COLS - 2 - (16-(current_offset % 0x0010)), makeChar(mem[byte16(current_offset)].to_ulong()));
                            typing = -1;
                            current_offset++;
                        }
                        break;
                    case 'e':
                        if (typing == -1) {
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, 'e' | A_STANDOUT);
                            typing = 14;
                            refresh();
                            goto Loop;
                        } else {
                            mem[byte16(current_offset)] = byte((typing * 16) + 14);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 12, 'e');
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, intToHex(typing, 1).c_str()[0]);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, COLS - 2 - (16-(current_offset % 0x0010)), makeChar(mem[byte16(current_offset)].to_ulong()));
                            typing = -1;
                            current_offset++;
                        }
                        break;
                    case 'f':
                        if (typing == -1) {
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, 'f' | A_STANDOUT);
                            typing = 15;
                            refresh();
                            goto Loop;
                        } else {
                            mem[byte16(current_offset)] = byte((typing * 16) + 15);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 12, 'f');
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, ((current_offset % 0x0010) * 3) + 11, intToHex(typing, 1).c_str()[0]);
                            mvaddch(floor(current_offset / 0x0010) - (top_offset / 0x0010) + 1, COLS - 2 - (16-(current_offset % 0x0010)), makeChar(mem[byte16(current_offset)].to_ulong()));
                            typing = -1;
                            current_offset++;
                        }
                        break;
                    case 'q':
                        nodelay(stdscr, FALSE);
                        keypad(stdscr, FALSE);
                        echo();
                        nocbreak();
                        endwin();
                        return mem;
                    default:
                        goto Loop;
                }
                if (exit) break;
        }
    }
    nodelay(stdscr, FALSE);
    keypad(stdscr, FALSE);
    echo();
    nocbreak();
    endwin();
    return mem;
}

bool runloop() {
    std::getline(std::cin, data);
    if (data == "q") return true;
    if (data == "m") {
    	ascii = !ascii;
		std::cout << "Switched mode to " << (ascii ? "ascii" : "number") << "\n8H1A Input> ";
		return false;
	}
	if (data == "r") {
	    cpu.exec(byte(10));
	    cpu.exec(byte(0));
	    cpu.exec(byte(0));
	    if (cpu.output != ZERO) {
	        if (ascii) std::cout << (char)cpu.output.to_ulong() << "\n";
	        else std::cout << cpu.output.to_ulong() << "\n";
	        cpu.output = ZERO;
	    }
	    std::cout << "8H1A Input> ";
	    return false;
	}
	if (data == "h" || data == "help") {
		std::cout << "List of commands:\ned\t\tDrive editor\nem\t\tMemory editor\nh\t\tDisplay this help\nhi\t\tDisplay instructions\nl\t\tLoad text file (*.8s1) to drive\nlb\t\tLoad binary file (*.8b1) to drive\nm\t\tToggle display of numbers between ascii and number\nq\t\tQuit\nr\t\tRun program at 0x0000\n8H1A Input> ";
		return false;
	}
	if (data == "hi") {
		std::cout << "List of instructions:\n0x00 <addend1> <addend2>\t\tAdd addend1 and addend2\n0x01 <first> <second>\t\t\tSubtract second from first\n0x02 <position> <value>\t\t\tSet position to value\n0x03 <num1> <num2> <instructions...>\tRun instructions if num1 equals num2\n0x04 <num1> <num2> <instructions...>\tRun instructions if num1 does not equal num2\n0x05 <num1> <num2> <instructions...>\tRun instructions if num1 is greater than num2\n0x06 <num1> <num2> <instructions...>\tRun instructions if num1 is less than num2\n0x07 <number>\t\t\t\tPrint number as character or number depending on mode\n0x08 <b1> <b0> <pos>\t\t\tLoad value b from drive to pos in memory\n0x09 <pos> <b1> <b0>\t\t\tSave value of pos to drive position b\n0x0A <b1> <b0>\t\t\t\tRun instructions from memory starting at b\n0x0B\t\t\t\t\tStop execution (only in memory)\nVariables can be used by replacing the number with 0xFF + memory position\n8H1A Input> ";
		return false;
	}
	if (data == "ed") {
	    cpu.drive = memEdit(cpu.drive, size);
	    std::cout << "8H1A Input> ";
	    return false;
	}
	if (data == "em") {
	    //cpu.cpu.memory = memEdit(cpu.cpu.memory);
	    return false;
	}
	/*if (data == "c") {
	    for (std::vector<byte> c : cpu.cache) {
	        for (byte b : c) std::cout << b.to_ulong() << " ";
	        std::cout << "\n";
	    }
	    std::cout << "8H1A Input> ";
	    return false;
	}*/
	if (data[0] == 'l') {
		std::vector<std::string> argv = split(data, ' ');
		std::string file;
		std::ifstream in;
		cpu.eraseDrive();
		std::cout << "Loading file " << argv[1] << " to 0x0000...";
	    if (data[1] == 'b') {
	        //std::cout << "b";
    		in.open(argv[1].c_str(), std::ios_base::binary | std::ios_base::in);
			char ex;
    		for (int i = 0; in.good() && !in.eof(); i++) {
				in.read(&ex, 1);
				std::pair<byte, byte> currentnum = splitByte(byte16(i));
				//std::cout << (int)reinterpret_cast<unsigned char&>(ex) << " ";
				if (reinterpret_cast<unsigned long&>(ex) == 255) {
				    cpu.exec(byte(0));
				    cpu.exec(byte(128));
				    cpu.exec(byte(127));
				    cpu.exec(byte(2));
				    cpu.exec(byte(1));
				    cpu.exec(byte(255));
				    cpu.exec(byte(0));
				    //std::cout << "0 128 127\n2 1 255 0\n";
				} else if (reinterpret_cast<unsigned long&>(ex) == 254) {
				    cpu.exec(byte(0));
				    cpu.exec(byte(127));
				    cpu.exec(byte(127));
				    cpu.exec(byte(2));
				    cpu.exec(byte(1));
				    cpu.exec(byte(255));
				    cpu.exec(byte(0));
				    //std::cout << "0 127 127\n2 1 255 0\n";
				} else {
				    cpu.exec(byte(2));
				    cpu.exec(byte(1));
				    cpu.exec(byte(reinterpret_cast<unsigned long&>(ex)));
				    //std::cout << "2 1 " << reinterpret_cast<unsigned long&>(ex) << "\n";
				}
				cpu.exec(byte(9));
				cpu.exec(byte(1));
				cpu.exec(std::get<0>(currentnum));
				cpu.exec(std::get<1>(currentnum));
				//std::cout << "9 1 " << std::get<0>(currentnum).to_ulong() << " " << std::get<1>(currentnum).to_ulong() << "\n";
    		}
    		in.close();
		    std::cout << "\n8H1A Input> ";
		    return false;
    	} else {
    	    //std::cout << "t";
			in.open(argv[1].c_str());
			while (!in.eof() && in.good()) {
			    std::string line;
			    in >> line;
			    file += line + " ";
			}
			std::vector<std::string> sp = split(file, '\n');
			int number = 0;
			for (std::string s : sp) {
			    //std::cout << s << "\n";
				std::vector<std::string> insts = split(s, ' ');
				for (std::string i : insts) {
				    if (i != "") {
				        int val = -1;
				        try {
				            val = std::stoi(i, 0, 0);
				        } catch (const std::invalid_argument& e) {
				            std::cout << "1: Invalid command " << i << "\n";
				        }
				        //std::cout << val << " ";
				        std::pair<byte, byte> currentnum = splitByte(byte16(number));
				        if (val != -1) {
				            if (val == 255) {
				                cpu.exec(byte(0));
				                cpu.exec(byte(128));
				                cpu.exec(byte(127));
				                cpu.exec(byte(2));
				                cpu.exec(byte(1));
				                cpu.exec(byte(255));
				                cpu.exec(byte(0));
				            } else {
				                cpu.exec(byte(2));
				                cpu.exec(byte(1));
				                cpu.exec(byte(val));
				            }
				            cpu.exec(byte(9));
				            cpu.exec(byte(1));
				            cpu.exec(std::get<0>(currentnum));
				            cpu.exec(std::get<1>(currentnum));
				        }
				        number++;
				    }
				}
			}
			in.close();
		    std::cout << "\n8H1A Input> ";
		    return false;
    	}
    	std::cout << "e";
    	return false;
	}
	int val = -1;
	try {
	    val = std::stoi(data, 0, 0);
	} catch (const std::invalid_argument& e) {
	    std::cout << "0: Invalid command " << data << "\n";
	}
	if (val != -1) cpu.exec(byte(val));;
	if (cpu.output != ZERO) {
	    if (ascii) std::cout << (char)cpu.output.to_ulong() << "\n";
	    else std::cout << cpu.output.to_ulong() << "\n";
	    cpu.output = ZERO;
	}
	std::cout << "8H1A Input> ";
	return false;
}

int main(int argc, const char * argv[]) {
    if (argc < 2) {
        size = 1024;
    } else {
        size = std::stoi(std::string(argv[1]));
    }
    cpu = C8H1A(size);
    std::cout << "8H1A Input> ";
    while (!runloop()) ;
    return 0;
}
