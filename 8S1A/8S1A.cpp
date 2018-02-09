#include <algorithm> // for std::fill
//#include <iostream> // debugging
#include "8S1A.hpp"

// Internal Commands

void C8S1A::clearAllCache() { // Clear entire cache
    if (cache[0].size() < 3) for (int i = cache[0].size(); i < 4; i++) cache[0].push_back(ZERO);
    if (cache[1].size() < 2) for (int i = cache[1].size(); i < 4; i++) cache[1].push_back(ZERO);
    if (cache[2].size() < 2) for (int i = cache[2].size(); i < 4; i++) cache[2].push_back(ZERO);
    if (cache[3].size() < 2) for (int i = cache[3].size(); i < 4; i++) cache[3].push_back(ZERO);
    if (cache[4].size() < 3) for (int i = cache[4].size(); i < 4; i++) cache[4].push_back(ZERO);
    if (cache[5].size() < 3) for (int i = cache[5].size(); i < 4; i++) cache[5].push_back(ZERO);
    if (cache[6].size() < 3) for (int i = cache[6].size(); i < 4; i++) cache[6].push_back(ZERO);
    if (cache[7].size() < 3) for (int i = cache[7].size(); i < 4; i++) cache[7].push_back(ZERO);
    std::fill(cache[0].begin(), cache[0].begin()+3, ZERO); // executor
    std::fill(cache[1].begin(), cache[1].begin()+2, ZERO); // add
    std::fill(cache[2].begin(), cache[2].begin()+2, ZERO); // sub
    std::fill(cache[3].begin(), cache[3].begin()+2, ZERO); // set
    std::fill(cache[4].begin(), cache[4].begin()+3, ZERO); // ifeq
    std::fill(cache[5].begin(), cache[5].begin()+3, ZERO); // ifne
    std::fill(cache[6].begin(), cache[6].begin()+3, ZERO); // ifgt
    std::fill(cache[7].begin(), cache[7].begin()+3, ZERO); // iflt
}

void C8S1A::done(int c) { // Clear cache of one command
    std::fill(cache[c].begin(), cache[c].end(), ZERO);
    std::fill(cache[0].begin(), cache[0].end(), ZERO);
    running = cache[0][0][0];
    //std::cout << "Ran command " << c - 1 << "\n";
}

void C8S1A::clearMem() { // Zero memory
    for (int i = 0; i < 256; i++) memory[byte(i)] = ZERO;
}

// CPU Instructions

void C8S1A::add(byte b) { // add <addend0> <addend1> -> memory[0x00] = addend0 + addend1
    if (cache[1][0] == ZERO) {
        cache[1][1] = b;
        cache[1][0] = byte(1);
    }
    else {
        byte sum = ZERO;
        byte a = cache[1][1];
        // Add a to b using xor
        sum[0] = (a[0] != b[0]);
        sum[1] = (((a[0] && b[0]) != a[1]) != b[1]);
        sum[2] = (((a[1] && b[1]) != a[2]) != b[2]);
        sum[3] = (((a[2] && b[2]) != a[3]) != b[3]);
        sum[4] = (((a[3] && b[3]) != a[4]) != b[4]);
        sum[5] = (((a[4] && b[4]) != a[5]) != b[5]);
        sum[6] = (((a[5] && b[5]) != a[6]) != b[6]);
        sum[7] = (((a[6] && b[6]) != a[7]) != b[7]);
        memory[ZERO] = sum;
        done(1);
    }
}

void C8S1A::sub(byte b) { // sub <minuend> <subtractend> -> memory[0x00] = minuend - subtractend
    if (cache[2][0] == ZERO) {
        cache[2][1] = b;
        cache[2][0] = byte(1);
    }
    else {
        byte a = cache[2][1];
        // Flip bits in b
        b[0] = !b[0];
        b[1] = !b[1];
        b[2] = !b[2];
        b[3] = !b[3];
        b[4] = !b[4];
        b[5] = !b[5];
        b[6] = !b[6];
        b[7] = !b[7];
        // Add 0b00000001 to b
        byte newb = ZERO; //temp for cpp
        newb[0] = !b[0];
        newb[1] = (b[0] != b[1]);
        newb[2] = (!b[1] != b[2]);
        newb[3] = (!b[2] != b[3]);
        newb[4] = (!b[3] != b[4]);
        newb[5] = (!b[4] != b[5]);
        newb[6] = (!b[5] != b[6]);
        newb[7] = (!b[6] != b[7]);
        b = newb;
        // Add a to 2's b
        byte sum = ZERO;
        sum[0] = (a[0] != b[0]);
        sum[1] = (((a[0] && b[0]) != a[1]) != b[1]);
        sum[2] = (((a[1] && b[1]) != a[2]) != b[2]);
        sum[3] = (((a[2] && b[2]) != a[3]) != b[3]);
        sum[4] = (((a[3] && b[3]) != a[4]) != b[4]);
        sum[5] = (((a[4] && b[4]) != a[5]) != b[5]);
        sum[6] = (((a[5] && b[5]) != a[6]) != b[6]);
        sum[7] = (((a[6] && b[6]) != a[7]) != b[7]);
        memory[ZERO] = sum;
        done(2);
    }
}

void C8S1A::set(byte a) { // set <position> <value> -> memory[position] = value
    if (cache[3][0] == ZERO) {
        cache[3][1] = a;
        cache[3][0] = byte(1);
    } else {
        if (memory.empty()) clearMem();
        memory[cache[3][1]] = a;
        done(3);
    }
}

void C8S1A::ifeq(byte b) { // ifeq <a> <b> <command...> 0b11111110 -> if (a == b) exec(command...)
    if (cache[4][0] == ZERO) {
        cache[4][0] = byte(1);
        cache[4][2] = b;
    } else if (cache[4][0] == byte(1)) {
        byte a = cache[4][2];
        cache[4][1][0] = (a[0] == b[0]); // using XNOR
        cache[4][1][0] = cache[4][1][0] && (a[1] == b[1]);
        cache[4][1][0] = cache[4][1][0] && (a[2] == b[2]);
        cache[4][1][0] = cache[4][1][0] && (a[3] == b[3]);
        cache[4][1][0] = cache[4][1][0] && (a[4] == b[4]);
        cache[4][1][0] = cache[4][1][0] && (a[5] == b[5]);
        cache[4][1][0] = cache[4][1][0] && (a[6] == b[6]);
        cache[4][1][0] = cache[4][1][0] && (a[7] == b[7]);
        cache[4][0] = byte(2);
        if (cache[4][1][0]) done(0);
        //std::cout << a.to_string() << "==" << b.to_string() << ": " << cache[4][1][0] << "\n";
    } else if (cache[4][0] == byte(2)) {
        if (!b[0] && b[1] && b[2] && b[3] && b[4] && b[5] && b[6] && b[7]) done(4);
        if (cache[4][1][0]) exec(b);
        else return;
    }
}

void C8S1A::ifne(byte b) { // ifne <a> <b> <command...> 0b11111110 -> if (a != b) exec(command...)
    if (cache[5][0] == ZERO) {
        cache[5][0] = byte(1);
        cache[5][2] = b;
    } else if (cache[5][0] == byte(1)) {
        byte a = cache[5][2];
        cache[5][1][0] = (a[0] != b[0]); // using XOR
        cache[5][1][0] = cache[5][1][0] || (a[1] != b[1]);
        cache[5][1][0] = cache[5][1][0] || (a[2] != b[2]);
        cache[5][1][0] = cache[5][1][0] || (a[3] != b[3]);
        cache[5][1][0] = cache[5][1][0] || (a[4] != b[4]);
        cache[5][1][0] = cache[5][1][0] || (a[5] != b[5]);
        cache[5][1][0] = cache[5][1][0] || (a[6] != b[6]);
        cache[5][1][0] = cache[5][1][0] || (a[7] != b[7]);
        cache[5][0] = byte(2);
        if (cache[5][1][0]) done(0);
    } else if (cache[5][0] == byte(2)) {
        if (!b[0] && b[1] && b[2] && b[3] && b[4] && b[5] && b[6] && b[7]) done(5);
        else if (cache[5][1][0]) exec(b);
    }
}

void C8S1A::ifgt(byte b) { // ifgt <a> <b> <command...> 0b11111110 -> if (a > b) exec(command...)
    if (cache[6][0] == ZERO) {
        cache[6][0] = byte(1);
        cache[6][2] = b;
    } else if (cache[6][0] == byte(1)) {
        byte a = cache[6][2];
        cache[6][1][0] = (!a[7] && b[7]);
        cache[6][1][0] = cache[6][1][0] || ((a[7] == b[7]) && (!a[6] && b[6]));
        cache[6][1][0] = cache[6][1][0] || ((a[7] == b[7]) && (a[6] == b[6]) && (!a[5] && b[5]));
        cache[6][1][0] = cache[6][1][0] || ((a[7] == b[7]) && (a[6] == b[6]) && (a[5] == b[5]) && (!a[4] && b[4]));
        cache[6][1][0] = cache[6][1][0] || ((a[7] == b[7]) && (a[6] == b[6]) && (a[5] == b[5]) && (a[4] == b[4]) && (!a[3] && b[3]));
        cache[6][1][0] = cache[6][1][0] || ((a[7] == b[7]) && (a[6] == b[6]) && (a[5] == b[5]) && (a[4] == b[4]) && (a[3] == b[3]) && (!a[2] && b[2]));
        cache[6][1][0] = cache[6][1][0] || ((a[7] == b[7]) && (a[6] == b[6]) && (a[5] == b[5]) && (a[4] == b[4]) && (a[3] == b[3]) && (a[2] == b[2]) && (!a[1] && b[1]));
        cache[6][1][0] = cache[6][1][0] || ((a[7] == b[7]) && (a[6] == b[6]) && (a[5] == b[5]) && (a[4] == b[4]) && (a[3] == b[3]) && (a[2] == b[2]) && (a[1] == b[1]) && (!a[0] && b[0]));
        if (cache[6][1][0]) done(0);
        cache[6][0] = byte(2);
    } else if (cache[6][0] == byte(2)) {
        if (!b[0] && b[1] && b[2] && b[3] && b[4] && b[5] && b[6] && b[7]) done(6);
        else if (cache[6][1][0]) exec(b);
    }
}

void C8S1A::iflt(byte b) { // iflt <a> <b> <command...> 0b11111110 -> if (a < b) exec(command...)
    if (cache[7][0] == ZERO) {
        cache[7][0] = byte(1);
        cache[7][2] = b;
    } else if (cache[7][0] == byte(1)) {
        byte a = cache[7][2];
        cache[7][1][0] = (a[7] && !b[7]);
        cache[7][1][0] = cache[7][1][0] || ((a[7] == b[7]) && (a[6] && !b[6]));
        cache[7][1][0] = cache[7][1][0] || ((a[7] == b[7]) && (a[6] == b[6]) && (a[5] && !b[5]));
        cache[7][1][0] = cache[7][1][0] || ((a[7] == b[7]) && (a[6] == b[6]) && (a[5] == b[5]) && (a[4] && !b[4]));
        cache[7][1][0] = cache[7][1][0] || ((a[7] == b[7]) && (a[6] == b[6]) && (a[5] == b[5]) && (a[4] == b[4]) && (a[3] && !b[3]));
        cache[7][1][0] = cache[7][1][0] || ((a[7] == b[7]) && (a[6] == b[6]) && (a[5] == b[5]) && (a[4] == b[4]) && (a[3] == b[3]) && (a[2] && !b[2]));
        cache[7][1][0] = cache[7][1][0] || ((a[7] == b[7]) && (a[6] == b[6]) && (a[5] == b[5]) && (a[4] == b[4]) && (a[3] == b[3]) && (a[2] == b[2]) && (a[1] && !b[1]));
        cache[7][1][0] = cache[7][1][0] || ((a[7] == b[7]) && (a[6] == b[6]) && (a[5] == b[5]) && (a[4] == b[4]) && (a[3] == b[3]) && (a[2] == b[2]) && (a[1] == b[1]) && (a[0] && !b[0]));
        cache[7][0] = byte(2);
        if (cache[7][1][0]) done(0);
    } else if (cache[7][0] == byte(2)) {
        if (!b[0] && b[1] && b[2] && b[3] && b[4] && b[5] && b[6] && b[7]) done(7);
        else if (cache[7][1][0]) exec(b);
    }
}

void C8S1A::out(byte b) { // out <number> -> printf("%d", number)
    output = b;
    done(0);
}

void C8S1A::exec(byte b) { // the main function!
    if (!b[0] && b[1] && b[2] && b[3] && b[4] && b[5] && b[6] && b[7]) {
        done(4); // would actually tell the ifs to finish via redstone
        done(5);
        done(6);
        done(7);
        return;
    }
    if (cache[0][0] == ZERO) {
        cache[0][1][0] = !b[0] && !b[1] && !b[2] && !b[3] && !b[4] && !b[5] && !b[6] && !b[7];
        cache[0][1][1] = b[0] && !b[1] && !b[2] && !b[3] && !b[4] && !b[5] && !b[6] && !b[7];
        cache[0][1][2] = !b[0] && b[1] && !b[2] && !b[3] && !b[4] && !b[5] && !b[6] && !b[7];
        cache[0][1][3] = b[0] && b[1] && !b[2] && !b[3] && !b[4] && !b[5] && !b[6] && !b[7];
        cache[0][1][4] = !b[0] && !b[1] && b[2] && !b[3] && !b[4] && !b[5] && !b[6] && !b[7];
        cache[0][1][5] = b[0] && !b[1] && b[2] && !b[3] && !b[4] && !b[5] && !b[6] && !b[7];
        cache[0][1][6] = !b[0] && b[1] && b[2] && !b[3] && !b[4] && !b[5] && !b[6] && !b[7];
        cache[0][1][7] = b[0] && b[1] && b[2] && !b[3] && !b[4] && !b[5] && !b[6] && !b[7];
        cache[0][0] = byte(1);
    } else {
        if (b[0] && b[1] && b[2] && b[3] && b[4] && b[5] && b[6] && b[7]) {
            cache[0][2] = byte(1);
            return;
        }
        if (cache[0][2] == byte(1)) {
            b = memory[b];
            cache[0][2] = ZERO;
        }
        if (cache[0][1][0]) add(b);
        if (cache[0][1][1]) sub(b);
        if (cache[0][1][2]) set(b);
        if (cache[0][1][3]) ifeq(b);
        if (cache[0][1][4]) ifne(b);
        if (cache[0][1][5]) ifgt(b);
        if (cache[0][1][6]) iflt(b);
        if (cache[0][1][7]) out(b);
    }
    running = cache[0][0][0];
}
