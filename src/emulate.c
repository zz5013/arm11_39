#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

#define PC reg[15] // or regs.r_15
#define CPSR reg[16]
#define r0 reg[0]
#define r1 reg[1]
#define r2 reg[2]
#define r3 reg[3]
#define r4 reg[4]
#define r5 reg[5]
#define r6 reg[6]
#define r7 reg[7]
#define r8 reg[8]
#define r9 reg[9]
#define r10 reg[10]
#define r11 reg[11]
#define r12 reg[12]
#define r13 reg[13]
#define r14 reg[14]
#define r15 reg[15]
#define r16 reg[16]

struct registers {
	int r_0;
	int r_1;
	int r_2;
	int r_3;
	int r_4;
	int r_5;
	int r_6;
	int r_7;
	int r_8;
	int r_9;
	int r_10;
	int r_11;
	int r_12;
	int r_13;
	int r_14;
	int r_15; // PC
	int r_16; // CPSR
};

char memory[64 * 1024]; //memory, every char means a byte
int reg[17]; // registers
struct registers regs;
_Bool sflag;
_Bool carryout;
_Bool clearpipe;

_Bool readbit();
_Bool checkCnd(int);
_Bool halt(int);
int memToInstrc(int);
void datapro(int);
void boot();
int power(int, int);
_Bool checkCnd(int);
void printBits(uint32_t);
int takeint(int, int, int);
int ror(int, int);
int asr(int, int);
int lsr(int, int);
int lsl(int, int);
int rotat(int, int);
void setZ();
void setN();
void setC();
void setV();
_Bool checkN();
_Bool checkV();
_Bool checkC();
_Bool checkZ();
unsigned int add(unsigned long, unsigned long);
unsigned int sub(unsigned int, unsigned int);
int op2reg(int);
void finalprint();
void branch(int);
void sdTrans(int);
void mul(int);
int typeOfInstrc(int);
_Bool checkaddr(int);
void decodexecute(int);
void store(int, int);
void reader(char *);
int checkGPIO(int);

int main(int argc, char **argv) { // Jiahao Lin
    return 0;
	boot(); // initialise
	char *pf = argv[1];

	reader(pf); // read file

	// start execute
	printf("PC current: %d\n", PC); //
	int current = memToInstrc(PC);
	PC+= 4;
	printf("PC NEXT: %d\n", PC); //
	int next = memToInstrc(PC);
	PC+= 4;
	printf("PC NEXT2: %d\n", PC); //
	int next2 = memToInstrc(PC);

	clearpipe = false;

	while (!halt(current)) {

		decodexecute(current); // decode and execute current
		if (clearpipe) {
			current = memToInstrc(PC);
			PC+= 4;
			next = memToInstrc(PC);
			PC+= 4;
			next2 = memToInstrc(PC);
			clearpipe = false;
		} else {
			printf("PC current = NEXT\n"); //
			current = next;
			printf("PC NEXT = NEXT2\n");//
			next = next2;
			PC+= 4;
			printf("PC NEXT2: %d\n", PC);//
			next2 = memToInstrc(PC);
		}

	}

	finalprint(); // print out register contents and non zero memory

	return EXIT_SUCCESS; // end program
}

void reader(char *point) {
	FILE *pfile = fopen(point, "rb"); // read binary file
	if (pfile == NULL) {
		perror("Unable to open file!\n");
		exit(EXIT_FAILURE);
	}
	fread(memory, sizeof(char), sizeof(memory) / sizeof(char), pfile);
	if (fclose(pfile) != 0) {
		perror("Unable to close file!\n");
		exit(EXIT_FAILURE);
	}
}

void decodexecute(int current) { // decode and execute current instruction

	printf("current instruction = ");
	printBits(current);
	if (checkCnd(current)) {
		int typ = typeOfInstrc(current);
		switch (typ) {
		case 1:
			printf("datapro\n");
			datapro(current);
			break;
		case 2:
			printf("mul\n");
			mul(current);
			break;
		case 3:
			printf("sdtrans\n");
			sdTrans(current);
			break;
		case 4:
			printf("branch\n");
			branch(current);
			break;
		default:
			printf("%s", "Impossible instruction\n");
		}
	}
}

void datapro(int instrc) { // Jiahao Lin

	int o2;
	sflag = false;
	carryout = false;
	if (readbit(instrc, 20)) {
		sflag = true;
		printf("SET Flag");
	}
	if (readbit(instrc, 25)) { // I = 1 immediate
		printf("I=1\n");
		unsigned int imm = takeint(instrc, 0, 8);
		unsigned int rotate = takeint(instrc, 8, 4);
		rotate *= 2;
		// now we have rotate and imm
		o2 = rotat(imm, rotate);  // have o2 if I is 1
		printf("o2=%d\n", o2);
	} else { // I = 0     shifted register
		printf("I=0\n");
		int rm = takeint(instrc, 0, 4);
		int vrm = reg[rm]; // value to be shifted
		int shifttype = takeint(instrc, 5, 2);
		int shiftvalue = 0;
		if (readbit(instrc, 4)) { // if bit 4 is 1
			int rs = takeint(instrc, 8, 4);
			int vrs = reg[rs];
			char lastbyte = vrs;
			int value = takeint(lastbyte, 0, 8);
			shiftvalue = value;
		} else { // bit 4 is 0
			unsigned int inte = takeint(instrc, 7, 5);
			shiftvalue = inte;
		}
		switch (shifttype) {
		case (0):
			o2 = lsl(vrm, shiftvalue);
			break;
		case (1):
			o2 = lsr(vrm, shiftvalue);
			break;
		case (2):
			o2 = asr(vrm, shiftvalue);
			break;
		case (3):
			o2 = ror(vrm, shiftvalue);
			break;
		default:
			printf("%s", "Impossible");
		}
		// now we have o2 if I = 0
	}
	// now we have o2
	int rn = takeint(instrc, 16, 4);
	printf("rn=%d\n", rn);
	int rd = takeint(instrc, 12, 4);
	printf("rd=%d\n", rd);
	int vrn = reg[rn];
	int opcode = takeint(instrc, 21, 4);
	printf("opcode=%d\n", opcode);
	int re = 0;
	switch (opcode) {
	case (0):
		printf("AND\n");
		printf("o2=%d\n", o2);
		reg[rd] = vrn & o2;
		if (sflag && carryout) {
			setC();
		}
		break;
	case (1):
		printf("EOR\n");
		printf("o2=%d\n", o2);
		reg[rd] = vrn ^ o2;
		if (sflag && carryout) {
			setC();
		}
		break;
	case (2):
		printf("SUB\n");
		printf("o2=%d\n", o2);
		reg[rd] = sub(vrn, o2);
		break;
	case (3):
		printf("RSB\n");
		printf("o2=%d\n", o2);
		reg[rd] = sub(o2, vrn);
		break;
	case (4):
		printf("ADD\n");
		printf("o2=%d\n", o2);
		reg[rd] = add(vrn, o2);
		break;
	case (8):
		CPSR= 0;
		printf("AND, no result\n");
		printf("o2=%d\n", o2);
		re = vrn & o2; // no result
		if (sflag && carryout) {
			setC();
		}
		if (sflag && re==0) {
			setZ();
		}
		break;
		case (9) : CPSR = 0;
		printf("EOR, no result\n");
		printf("o2=%d\n", o2);
		re = vrn ^ o2; // no result
		if (sflag && carryout) {
			setC();
		}
		break;
		case (10) : CPSR = 0;
		printf("SUB, no result\n");
		printf("o2=%d\n", o2);re = sub(vrn, o2); // no result
		break;
		case (12) :printf("OR\n");
		printf("o2=%d\n", o2);
		reg[rd] = vrn | o2;
		if (sflag && carryout) {
			setC();
		}
		break;
		case (13) : printf("MOV\n");
		printf("o2=%d\n", o2);
		reg[rd] = o2;
		if (sflag && carryout) {
			setC();
		}
		break;
		default : printf("%s\n", "Impossible opcode!");
	}
	re += 1;
}


unsigned int add(unsigned long x, unsigned long y) {
	unsigned long re = 0;
	re = x + y;
	unsigned int re2 = re >> 32;
	if (readbit(re2, 0) == 1) {
		carryout = true;
      if (sflag & carryout) {
    	  setC();
      }
	}
    return re;
}

unsigned int sub(unsigned int x, unsigned int y) {
	printf("sub\n");
	int re = x-y;
	if (sflag && (re==0)) {
	  setZ();

	}
	if (sflag && (re<0)) {
		setN();
    }
	if (sflag && re>=0) {
        setC();
	}
    return re;
}

void mul(int instrc) {//Shuang Xia
	    int rm = reg[takeint(instrc, 0, 4)];
	    printf("rm = %d\n", rm);
	    int rs = reg[takeint(instrc, 8, 4)];
	    printf("rs = %d\n", rs);
	    int rn = reg[takeint(instrc, 12, 4)];
	    printf("rn = %d\n", rn);
	    int rd = takeint(instrc, 16, 4);
	    printf("rd = %d\n", rd);
	    int64_t result;
	    int32_t number;


	    if (readbit(instrc, 21)) { //accumulate
	    	printf("Accumulate\n");
		    result = rm * rs + rn;
		    number = result;
		    printf("number = %d\n", number);
		    reg[rd] = number;
	    } else {
	    	printf("Not Accumulate\n");
	    	result = rm * rs;
	    	number = result;
	    	printf("number = %d\n", number);
	    	reg[rd] = number;
	    }

	    if (readbit(instrc, 20)) { //set flags
	        if (readbit(result, 31)) {
	        	setN();
	        }
	        if (number == 0) {
	        	setZ();
	        }
	    }
}

int op2reg(int instrc) {
	int o2 = 0;
	int rm = takeint(instrc, 0, 4);
	int vrm = reg[rm]; // value to be shifted
	int shifttype = takeint(instrc, 5, 2);
	int shiftvalue = 0;
	if (!readbit(instrc, 4)) { // if bit 4 is 1
		unsigned int inte = takeint(instrc, 7, 5);
		shiftvalue = inte;
	} else { // bit 4 is 0
		int rs = takeint(instrc, 8, 4);
		int vrs = reg[rs];
		char lastbyte = vrs;
		int value = takeint(lastbyte, 0, 8);
		shiftvalue = value;
	}
	switch (shifttype) {
	case (0):
		o2 = lsl(vrm, shiftvalue);
		break;
	case (1):
		o2 = lsr(vrm, shiftvalue);
		break;
	case (2):
		o2 = asr(vrm, shiftvalue);
		break;
	case (3):
		o2 = ror(vrm, shiftvalue);
		break;
	default:
		printf("%s", "Impossible");
	}
	return o2;
}

void sdTrans(int instrc) { //wenke yang(Alice)
	int rn = takeint(instrc, 16, 4);
	printf("rn = %d\n", rn);
	int rd = takeint(instrc, 12, 4);
	printf("rd = %d\n", rd);
	uint32_t imOffset;
	int shiftedR;
	_Bool addORsub; //true for add, false for sub
	_Bool indexAddr; //true for pre-indexing addressing; false for post
	_Bool loadorstroe = readbit(instrc, 20);
	_Bool I = readbit(instrc, 25);
	if (I) {
		printf("I=1\n");
	} else {
		printf("I=0\n");
	}
	if (loadorstroe) {
		printf("load\n");
	} else {
		printf("store\n");
	}
	addORsub = readbit(instrc, 23);
	indexAddr = readbit(instrc, 24);
	if (addORsub) {
		printf("U = 1\n");
	} else {
		printf("U = 0\n");
	}
	if (indexAddr) {
		printf("pre indexing\n");
	} else {
		printf("post indexing\n");
	}
	int addr = reg[rn];
	if (I) { //I is set
		shiftedR = op2reg(instrc);
		if (loadorstroe) { //load from memory
			if (indexAddr) { //pre
				if (addORsub) { //add
					addr += shiftedR;
				} else {
					addr -= shiftedR;
				}
				printf("address = %d\n", addr);
				if (checkGPIO(addr) == -1) {
					if (checkaddr(addr)) {
						reg[rd] = memToInstrc(addr);
					}
				} else {
					reg[rd] = addr;
					switch (checkGPIO(addr)) {
					case 0:
						printf("%s",
								"One GPIO pin from 0 to 9 has been accessed");
						break;
					case 1:
						printf("%s",
								"One GPIO pin from 10 to 19 has been accessed");
						break;
					case 2:
						printf("%s",
								"One GPIO pin from 20 to 29 has been accessed");
						break;
					}
				}
			} else { //post
				printf("address = %d\n", addr);
				if (checkGPIO(addr) == -1) {
					if (checkaddr(addr)) {
						reg[rd] = memToInstrc(addr);
					}
				} else {
					reg[rd] = addr;
					switch (checkGPIO(addr)) {
					case 0:
						printf("%s",
								"One GPIO pin from 0 to 9 has been accessed");
						break;
					case 1:
						printf("%s",
								"One GPIO pin from 10 to 19 has been accessed");
						break;
					case 2:
						printf("%s",
								"One GPIO pin from 20 to 29 has been accessed");
						break;
					}
				}
				if (addORsub) { //add
					reg[rn] += shiftedR;
				} else {
					reg[rn] -= shiftedR;
				}
			}
		} else { //store into memory
			if (indexAddr) { //pre
				if (addORsub) { //add
					addr += shiftedR;
				} else {
					addr -= shiftedR;
				}
				if (addr == 538968104 || addr == 538968092) {
					printf("%s", "PIN OFF");
				}
				if (addr == 538968092) {
					printf("%s", "PIN ON");
				}
				printf("address = %d\n", addr);
				printf("rd = %d\n", rd);
				if (checkaddr(addr)) {
					store(addr, reg[rd]);
				}
			} else { //post
				printf("address = %d\n", addr);
				printf("vrd = %d\n", reg[rd]);
				if (addr == 538968104 || addr == 538968092) {
									printf("%s", "PIN OFF");
								}
								if (addr == 538968092) {
									printf("%s", "PIN ON");
								}
				if (checkaddr(addr)) {
					store(addr, reg[rd]);
					if (addORsub) { //add
						reg[rn] += shiftedR;
					} else {
						reg[rn] -= shiftedR;
					}
				}
			}
		}
	} else { // I is not set
		imOffset = takeint(instrc, 0, 12);
		printf("offset = %d\n", imOffset);
		if (loadorstroe) { //load from memory
			if (indexAddr) { //pre
				if (addORsub) { //add
					addr += imOffset;
				} else {
					addr -= imOffset;
				}
				printf("address = %d\n", addr);

				if (checkGPIO(addr) == -1) {
					if (checkaddr(addr)) {
						reg[rd] = memToInstrc(addr);
					}
				} else {
					reg[rd] = addr;
					switch (checkGPIO(addr)) {
					case 0:
						printf("%s",
								"One GPIO pin from 0 to 9 has been accessed");
						break;
					case 1:
						printf("%s",
								"One GPIO pin from 10 to 19 has been accessed");
						break;
					case 2:
						printf("%s",
								"One GPIO pin from 20 to 29 has been accessed");
						break;
					}
				}

			} else { //post
				printf("address = %d\n", addr);

				if (checkGPIO(addr) == -1) {
					if (checkaddr(addr)) {
						reg[rd] = memToInstrc(addr);
					}
				} else {
					reg[rd] = addr;
					switch (checkGPIO(addr)) {
					case 0:
						printf("%s",
								"One GPIO pin from 0 to 9 has been accessed");
						break;
					case 1:
						printf("%s",
								"One GPIO pin from 10 to 19 has been accessed");
						break;
					case 2:
						printf("%s",
								"One GPIO pin from 20 to 29 has been accessed");
						break;
					}
				}
				if (addORsub) { //add
					reg[rn] += imOffset;
				} else {
					reg[rn] -= imOffset;
				}
			}

		} else { //store into memory
			if (indexAddr) { //pre
				int addr = reg[rn];
				if (addORsub) { //add
					addr += imOffset;
				} else {
					addr -= imOffset;
				}
				printf("address = %d\n", addr);
				printf("vrd = %d\n", reg[rd]);
				if (addr == 538968104 || addr == 538968092) {
									printf("%s", "PIN OFF");
								}
								if (addr == 538968092) {
									printf("%s", "PIN ON");
								}
				if (checkaddr(addr)) {
					store(addr, reg[rd]);
				}
				printf("%d\n", memToInstrc(addr));
			} else { //post
				printf("address = %d\n", addr);
				if (addr == 538968104 || addr == 538968092) {
									printf("%s", "PIN OFF");
								}
								if (addr == 538968092) {
									printf("%s", "PIN ON");
								}
				if (checkaddr(addr)) {
					store(addr, reg[rd]);
					if (addORsub) { //add
						reg[rn] += imOffset;
					} else {
						reg[rn] -= imOffset;
					}
				}
			}

		}
	}
}

int checkGPIO(int add) {
	if (add == 538968064) {
		return 0;
	} else if(add == 538968068) {
		return 1;
	} else if(add == 538968072) {
      return 2;
	} else {
	  return -1;
	}
}

void store(int add, int rdd) {
	for (int i = 0; i < 4; i++) {
		memory[add + i] = takeint(rdd, 8 * i, 8);
	}
}

_Bool checkaddr(int addre) {
	if (addre >= 64 * 1024) {
		printf("Error: Out of bounds memory access at address %d\n", addre);
		return false;
	} else {
		return true;
	}
}

void branch(int instrc) { // Zhuofan Zhang
    int address = 0;
    address = lsl(takeint(instrc, 0, 24), 2);
    if (readbit(instrc, 23)){
       address |= ((~0) << 26);
    }
    printf("           addresssssss = %d\n", address);
    PC += address;
    printf("branch to \n");
    printBits(memToInstrc(PC));
    clearpipe = true;
}

int typeOfInstrc(int instrc) { //Zhuofan Zhang
	if (readbit(instrc, 27)) {
		return 4;
	} else if (readbit(instrc, 26)) {
		return 3;
	} else if (!readbit(instrc, 25) && !readbit(instrc, 24)
			&& !readbit(instrc, 23) && !readbit(instrc, 22)
			&& readbit(instrc, 7) && !readbit(instrc, 6) && !readbit(instrc, 5)
			&& readbit(instrc, 4)) {
		return 2;
	} else {
		return 1;
	}
}


_Bool readbit(int instrc, int bitadd) { //wenke yang; reading start from right to left. begin with index 0
	return takeint(instrc, bitadd, 1) == 1;
}

void boot() {
	for (int i = 0; i < 17; reg[i++] = 0);

	for (int j = 0; j < 64 * 1024; memory[j++] = 0);
}

_Bool checkCnd(int i) { //wenke yang; condition check: input i is the instruction
	if (readbit(i, 31)) {
		if (readbit(i, 30)) {
			if (readbit(i, 29)) {
				//always
				return true;
			} else if (readbit(i, 28)) {
				//less than or equal
				return checkZ() || (checkN() != checkV());
			}

			//greater than
			return !checkZ() && (checkN() == checkV());

		} else if (readbit(i, 28)) {
			//less than
			return checkN() != checkV();
		}

		//greater or equal
		return checkN() == checkV();

	} else if (readbit(i, 28)) {
		//not equal
		return !checkZ();

	}
	//equal
	return checkZ();
}

_Bool checkN() { //Shuang Xia
  return readbit(reg[16], 31);
}

_Bool checkZ() {
	return readbit(reg[16], 30);
}

_Bool checkC() {
	return readbit(reg[16], 29);
}

_Bool checkV() {
	return readbit(reg[16], 28);
}

void setZ() {
	if (!readbit(r16, 30)) {
		r16 += power(2, 30);
	}
	 printf("SET Z!\n");
	 printBits(r16);
}

void setN() {
	if (!readbit(r16, 31)) {
		r16 += -power(2, 31);
	}
	 printf("SET N!\n");
	 printBits(r16);
}

void setC() {
	if (!readbit(r16, 29)) {
		r16 += power(2, 29);
	}
	printf("SET C!\n");
	printBits(r16);
}

void setV() {
	if (!readbit(r16, 28)) {
		r16 += power(2, 28);
}
	printf("SET V!\n");
	printBits(r16);
}

_Bool halt(int i) { //wenke yang; all bit in instruction i is 0.
	return i == 0;
}

int memToInstrc(int add) { //wenke yang; add is the value of PC now, read 4 bytes from PC to be an instruction
	if (add >=0 && add < 64*1024) {
		int res=0;
			for (int i = 0; i < 4; i++) {
			  if (readbit(memory[add + i], 7)) {
		        int mask = 255 << (8 * i);
		       int shifted = memory[add + i] << (8 * i);
		       res += mask & shifted;
		      } else {
			    int new = memory[add + i] << (8 * i);
		        res += new;
		      }
		    }
		    return res;
	} else {
		printf("Out of bounds memory access at address %d\n", add);
		return 0;
	}
}

int memToInstrc2(int add){ //a flipped instruction
	int res = 0;
		for (int i = 0; i < 4; i++) {
		  if (readbit(memory[add + 3 - i], 7)) {
	        int mask = 255 << (8 * i);
	       int shifted = memory[add + 3 - i] << (8 * i);
	       res += mask & shifted;
	      } else {
		    int new = memory[add + 3 - i] << (8 * i);
	        res += new;
	      }
	    }
	    return res;
}

int power(int x, int y) { // Jiahao Lin
  if (x == 0) {
	  return 0;
  } else if (y == 0){
	  return 1;
  } else {
	  return x * power(x, y-1);
  }
}

int rotat(int x,int y) {
   if (y != 0) {
	   carryout = readbit(x, y-1);
	     int z = x << (32 - y);
	     int v = x >> y;
	     return (z | v);
   } else {
	  return x;
   }
}

int lsl(int x, int y) {
	if (y != 0) {
		 carryout = readbit(x, 32-y);
		   return x << y;
	} else {
		return x;
	}
}

int lsr(int x, int y) {
	if (y != 0) {
		carryout = readbit(x, y-1);
		   return x >> y;
	} else {
		return x;
	}

}

int asr(int x, int y) {
	if (y != 0) {
		carryout = readbit(x, y-1);
		   int z = lsr(x, y);
		   int v = lsr(x, 31);
		   if (v == 1) {
		     v = lsl((power(2, y)-1), 32-y);
		     return y | z;
		   } else {
			 return z;
		   }
	} else {
		return x;
	}

}

int ror(int x, int y) {
	if (y != 0) {
			int z = x << (32 - y);
			int v = x >> y;
			return (z | v);
		} else {
			return x;
		}
}


int takeint(int instruc, int start, int length) { // start --- start+length-1
  if (start > 31 || start < 0) {
	  printf("%s\n", "take int error");
	  printf("start  =   %d    length = %d\n", start, length);
	  return 0;
  } else {
      instruc >>= start;
      int temp = ~(~0 << length);
      return instruc & temp;
  }
}

void printBits(uint32_t x) {
	int i;
	uint32_t mask = 1 << 31;
	for(i=0; i<32; ++i) {
	printf("%i", (x & mask) != 0);
	x <<= 1;
	}
	printf("\n");
}

void finalprint() {  // Zhuofan Zhang
    printf("%s\n","Registers:");

    printf("$0  :%10d(0x%.8x)\n",r0,r0);
    printf("$1  :%10d(0x%.8x)\n",r1,r1);
    printf("$2  :%10d(0x%.8x)\n",r2,r2);
    printf("$3  :%10d(0x%.8x)\n",r3,r3);
    printf("$4  :%10d(0x%.8x)\n",r4,r4);
    printf("$5  :%10d(0x%.8x)\n",r5,r5);
    printf("$6  :%10d(0x%.8x)\n",r6,r6);
    printf("$7  :%10d(0x%.8x)\n",r7,r7);
    printf("$8  :%10d(0x%.8x)\n",r8,r8);
    printf("$9  :%10d(0x%.8x)\n",r9,r9);
    printf("$10 :%10d(0x%.8x)\n",r10,r10);
    printf("$11 :%10d(0x%.8x)\n",r11,r11);
    printf("$12 :%10d(0x%.8x)\n",r12,r12);
    printf("PC  :%10d(0x%.8x)\n",r15,r15);
    printf("CPSR:%10d(0x%.8x)\n",r16,r16);



    printf("%s\n","Non-zero memory:");

    for (int i = 0; i < (64 * 1024); i = i + 4){

    	if (memToInstrc(i) != 0){
    		printf("%s","0x");
    		printf("%.8x",i);
    		printf("%s",": 0x");

    		printf("%.8x",memToInstrc2(i));

    		printf("\n");
    	}
    }
}
