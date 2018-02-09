#include "8S1A.hpp"

class byte16: public std::bitset<16> {
    public:
        using std::bitset<16>::bitset;
        bool operator <(const byte16 &rhs) const {
            return to_ulong() < rhs.to_ulong();
        }
        bool operator >(const byte16 &rhs) const {
            return to_ulong() > rhs.to_ulong();
        }
        
};

extern byte16 combineBytes(byte a, byte b);

class C8H1A {
    public:
        byte output = ZERO;
        void exec(byte b);
        void load(byte b);
        void save(byte b);
        void jump(byte b);
        void eraseDrive();
        void clearCache();
        
        C8H1A(int size) : drivesize(size) {
            eraseDrive();
            clearCache();
        }
        std::map<byte16, byte> drive;
        C8S1A cpu;
        
    private:
        int drivesize;
        std::vector<byte> cache[4];
        void done(int c);
};
