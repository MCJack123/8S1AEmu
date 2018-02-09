#include <algorithm>
//#include <iostream>
#include "8H1A.hpp"

byte16 combineBytes(byte a, byte b) {
    byte16 retval;
    retval[0] = b[0];
    retval[1] = b[1];
    retval[2] = b[2];
    retval[3] = b[3];
    retval[4] = b[4];
    retval[5] = b[5];
    retval[6] = b[6];
    retval[7] = b[7];
    retval[8] = a[0];
    retval[9] = a[1];
    retval[10] = a[2];
    retval[11] = a[3];
    retval[12] = a[4];
    retval[13] = a[5];
    retval[14] = a[6];
    retval[15] = a[7];
    return retval;
}

void C8H1A::eraseDrive() {
    for (int i = 0; i < drivesize; i++) drive[byte16(i)] = ZERO;
}

void C8H1A::clearCache() {
    if (cache[0].size() < 3) for (int i = cache[0].size(); i < 4; i++) cache[0].push_back(ZERO);
    if (cache[1].size() < 2) for (int i = cache[1].size(); i < 4; i++) cache[1].push_back(ZERO);
    if (cache[2].size() < 2) for (int i = cache[2].size(); i < 4; i++) cache[2].push_back(ZERO);
    if (cache[3].size() < 2) for (int i = cache[3].size(); i < 4; i++) cache[3].push_back(ZERO);
    std::fill(cache[0].begin(), cache[0].begin()+3, ZERO); // executor
    std::fill(cache[1].begin(), cache[1].begin()+3, ZERO); // load
    std::fill(cache[2].begin(), cache[2].begin()+3, ZERO); // save
    std::fill(cache[3].begin(), cache[3].begin()+3, ZERO); // jump
}

void C8H1A::done(int c) {
    std::fill(cache[c].begin(), cache[c].end(), ZERO);
    std::fill(cache[0].begin(), cache[0].end(), ZERO);
    output = byte((output | cpu.output).to_ulong());
    cpu.output = ZERO;
    //std::cout << "Ran extended command " << c - 1 << "\n";
}

void C8H1A::load(byte b) { // load <drive byte 1> <drive byte 0> <memory position>
    if (cache[1][0] == ZERO) {
        cache[1][1] = b;
        cache[1][0] = byte(1);
    } else if (cache[1][0] == byte(1)) {
        cache[1][2] = b;
        cache[1][0] = byte(2);
    } else if (cache[1][0] == byte(2)) {
        cpu.exec(byte(2));
        cpu.exec(b);
        cpu.exec(drive[combineBytes(cache[1][1], cache[1][2])]);
        done(1);
    }
}

void C8H1A::save(byte b) { // save <memory position> <drive byte 1> <drive byte 0>
     if (cache[2][0] == ZERO) {
        cache[2][1] = b;
        cache[2][0] = byte(1);
    } else if (cache[2][0] == byte(1)) {
        cache[2][2] = b;
        cache[2][0] = byte(2);
    } else if (cache[2][0] == byte(2)) {
        cpu.exec(byte(7));
        cpu.exec(byte(255));
        cpu.exec(cache[2][1]);
        drive[combineBytes(cache[2][2], b)] = cpu.output;
        cpu.output = ZERO;
        done(2);
    }
}

void C8H1A::jump(byte b) { // jump <drive byte 1> <drive byte 0>
    if (cache[3][0] == ZERO) {
        cache[3][1] = b;
        cache[3][0] = byte(1);
    } else {
        done(0);
        cache[3][2] = drive[combineBytes(cache[3][1], b)];
        if (!cache[3][2][7] && !cache[3][2][6] && !cache[3][2][5] && !cache[3][2][4] && cache[3][2][3] && !cache[3][2][2] && cache[3][2][1] && cache[3][2][0]) {
            done(3);
            return;
        }
        //std::cout << "Running " << cache[3][2].to_ulong() << " at " << combineBytes(cache[3][1], b).to_ulong() << "\n";
        exec(cache[3][2]);
        if (b[0] && b[1] && b[2] && b[3] && b[4] && b[5] && b[6] && b[7]) {
            cache[3][1] = byte(cache[3][1].to_ulong() + 1);
        }
        //std::cout << "Jumping to " << cache[3][1].to_ulong() << " " << b.to_ulong() + 1 << "\n";
        jump(byte(b.to_ulong() + 1));
    }
}

void C8H1A::exec(byte b) {
    if (cache[0][0] == byte(1)) {
        load(b);
        return;
    }
    if (cache[0][0] == byte(2)) {
        save(b);
        return;
    }
    if (cache[0][0] == byte(3)) {
        jump(b);
        return;
    }
    if (cpu.running && cache[0][0] == ZERO) {
        cpu.exec(b);
    } else {
        if (!b[7] && !b[6] && !b[5] && !b[4] && b[3] && !b[2] && !b[1] && !b[0]) {
            //load(b);
            cache[0][0] = byte(1);
        }
        if (!b[7] && !b[6] && !b[5] && !b[4] && b[3] && !b[2] && !b[1] && b[0]) {
            //save(b);
            cache[0][0] = byte(2);
        }
        if (!b[7] && !b[6] && !b[5] && !b[4] && b[3] && !b[2] && b[1] && !b[0]) {
            //jump(b);
            cache[0][0] = byte(3);
        }
        if (cache[0][0] == ZERO) cpu.exec(b);
    }
    output = byte((output | cpu.output).to_ulong());
    cpu.output = ZERO;
}
