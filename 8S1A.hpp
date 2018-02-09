#define ZERO byte(0) // quick 0x00 access
#include <vector> // for cache vector
#include <bitset> // for byte type
#include <map> // for memory map

class byte: public std::bitset<8> {
    public:
        using std::bitset<8>::bitset;
        bool operator <(const byte &rhs) const {
            return to_ulong() < rhs.to_ulong();
        }
        bool operator >(const byte &rhs) const {
            return to_ulong() > rhs.to_ulong();
        }
};

//typedef bitsetm<8> byte; // byte type

/*
CPU pinout (not actual layout, but approximation):

          ----
   INS0 --|  |-- OUT0
   INS1 --|  |-- OUT1
   INS2 --|  |-- OUT2
   INS3 --|  |-- OUT3
   INS4 --|  |-- OUT4
   INS5 --|  |-- OUT5
   INS6 --|  |-- OUT6
   INS7 --|  |-- OUT7
   CLKI --|  |-- CLKO
          ----
INS0-7 = byte b
CLKI   = running exec(b)
OUT0-7 = out property
CLKO   = return of exec(b)

*/

class C8S1A {
    public:
        byte output = ZERO; // output of cpu
        bool running = false;
        void clearMem(); // clears all memory
        void clearAllCache(); // clears all cache
        void exec(byte b); // execute command (main function)
        // Instructions
        void add(byte b); // add
        void sub(byte b); // subtract
        void set(byte b); // set
        void ifeq(byte b); // if equal
        void ifne(byte b); // if not equal
        void ifgt(byte b); // if greater than
        void iflt(byte b); // if less than
        void out(byte b); // output
        
        C8S1A() { // initializer
            clearMem();
            clearAllCache();
        }
        std::map<byte, byte> memory; // main memory
        
    private:
        std::vector<byte> cache[8]; // extra memory for commands (not in actual machine)
        void done(int c); // clears cache 0 + cache c
        
};
