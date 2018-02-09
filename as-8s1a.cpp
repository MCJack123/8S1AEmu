#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <bitset>
#include <utility>

typedef std::bitset<8> byte;
typedef std::bitset<16> byte16;

std::string trim(const std::string& str,
                 const std::string& whitespace = " \t")
{
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

std::string reduce(const std::string& str,
                   const std::string& fill = " ",
                   const std::string& whitespace = " \t")
{
    // trim first
    auto result = trim(str, whitespace);

    // replace sub ranges
    auto beginSpace = result.find_first_of(whitespace);
    while (beginSpace != std::string::npos)
    {
        const auto endSpace = result.find_first_not_of(whitespace, beginSpace);
        const auto range = endSpace - beginSpace;

        result.replace(beginSpace, range, fill);

        const auto newStart = beginSpace + fill.length();
        beginSpace = result.find_first_of(whitespace, newStart);
    }

    return result;
}

std::vector<std::string> split(const std::string &s, char delim) {
  std::stringstream ss(s);
  std::string item;
  std::vector<std::string> elems;
  if (s.find(delim) == std::string::npos) {
      elems.push_back(s);
      return elems;
  }
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

int main(int argc, const char * argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <file.s> <output>\n";
        return 1;
    }
    std::ifstream in;
    in.open(argv[1]);
    std::string line;
    std::vector<char> outfile;
    while (in.good() && !in.eof()) {
        std::getline(in, line);
        line = reduce(line);
        std::string instr = line.substr(0, 4);
        int twonum = -1;
        if (instr == "addn") {
            outfile.push_back(0);
        } else if (instr == "subt") {
            outfile.push_back(1);
        } else if (instr == "setv") {
            outfile.push_back(2);
        } else if (instr == "ifeq") {
            outfile.push_back(3);
        } else if (instr == "ifne") {
            outfile.push_back(4);
        } else if (instr == "ifgt") {
            outfile.push_back(5);
        } else if (instr == "iflt") {
            outfile.push_back(6);
        } else if (instr == "outp") {
            outfile.push_back(7);
        } else if (instr == "load") {
            outfile.push_back(8);
            twonum = 0;
        } else if (instr == "save") {
            outfile.push_back(9);
            twonum = 1;
        } else if (instr == "jump") {
            outfile.push_back(10);
            twonum = 0;
        } else if (instr == "stop") {
            outfile.push_back(11);
            goto write;
        } else if (instr == "done") {
            outfile.push_back(254);
            continue;
        } else {
            std::cerr << "Error: Invalid instruction " << instr << "\n";
            return 2;
        }
        std::string argstr = line.substr(5, line.find("#") - 6);
        std::vector<std::string> args = split(argstr, ',');
        int i = 0;
        for (std::string arg : args) {
            arg = reduce(arg);
            if (arg[0] == '$') {
                arg = arg.substr(1);
                outfile.push_back(255);
            }
            int argnum = std::stoi(arg);
            if (twonum != -1 && twonum == i) {
                std::pair<byte, byte> newnum = splitByte(byte16(argnum));
                outfile.push_back((char)std::get<0>(newnum).to_ulong());
                argnum = (int)std::get<1>(newnum).to_ulong();
            } else {
                i++;
            }
            if (argnum > 255) argnum %= 256;
            outfile.push_back((char)argnum);
        }
    }
write:
    if (outfile[outfile.size() - 1] != 11) {
        std::cout << "Warning: Missing stop at end of program\n";
    }
    in.close();
    std::ofstream out;
    out.open(argv[2], std::ios_base::binary | std::ios_base::out);
    out.write(&outfile[0], outfile.size());
    out.close();
    return 0;
}