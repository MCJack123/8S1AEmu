#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "8S1A.hpp"

C8S1A cpu = C8S1A();
std::string data;
bool ascii;

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

bool runloop() {
    std::getline(std::cin, data);
    if (data == "q") return true;
    if (data == "m") {
    	ascii = !ascii;
		std::cout << "Switched mode to " << (ascii ? "ascii" : "number") << "\n8S1A Input> ";
		return false;
	}
	if (data == "h") {
		std::cout << "List of commands:\ne\t\tExecute text file (*.8s1)\neb\t\tExecute binary file (*.8b1)\nh\t\tDisplay this help\nhi\t\tDisplay instructions\nm\t\tToggle display of numbers between ascii and number\nq\t\tQuit\n8S1A Input> ";
		return false;
	}
	if (data == "hi") {
		std::cout << "List of instructions:\n0x00 <addend1> <addend2>\t\tAdd addend1 and addend2\n0x01 <first> <second>\t\t\tSubtract second from first\n0x02 <position> <value>\t\t\tSet position to value\n0x03 <num1> <num2> <instructions...>\tRun instructions if num1 equals num2\n0x04 <num1> <num2> <instructions...>\tRun instructions if num1 does not equal num2\n0x05 <num1> <num2> <instructions...>\tRun instructions if num1 is greater than num2\n0x06 <num1> <num2> <instructions...>\tRun instructions if num1 is less than num2\n0x07 <number>\t\t\t\tPrint number as character or number depending on mode\nVariables can be used by replacing the number with 0xFF + memory position\n8S1A Input> ";
		return false;
	}
	/*if (data == "c") {
	    for (std::vector<byte> c : cpu.cache) {
	        for (byte b : c) std::cout << b.to_ulong() << " ";
	        std::cout << "\n";
	    }
	    std::cout << "8S1A Input> ";
	    return false;
	}*/
	if (data[0] == 'e') {
		std::vector<std::string> argv = split(data, ' ');
		std::string file;
		std::ifstream in;
	    if (data[1] == 'b') {
	        //std::cout << "b";
    		in.open(argv[1].c_str(), std::ios_base::binary | std::ios_base::in);
			char ex;
    		while (in.good() && !in.eof()) {
				in.read(&ex, 1);
				//std::cout << (int)reinterpret_cast<unsigned char&>(ex) << " ";
				cpu.exec(byte(reinterpret_cast<unsigned long&>(ex)));
				if (cpu.output != ZERO) {
	                if (ascii) std::cout << (char)cpu.output.to_ulong() << "\n";
	                else std::cout << cpu.output.to_ulong() << "\n";
	                cpu.output = ZERO;
	            }
    		}
    		in.close();
		    std::cout << "8S1A Input> ";
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
				        if (val != -1) cpu.exec(byte(val));
				        if (cpu.output != ZERO) {
	                        if (ascii) std::cout << (char)cpu.output.to_ulong() << "\n";
	                        else std::cout << cpu.output.to_ulong() << "\n";
	                        cpu.output = ZERO;
	                    }
				    }
				}
			}
			in.close();
		    std::cout << "8S1A Input> ";
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
	std::cout << "8S1A Input> ";
	return false;
}

int main() {
    std::cout << "8S1A Input> ";
    while (!runloop()) ;
    return 0;
}
