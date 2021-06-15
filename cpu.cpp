#include "termi8.h"

CPURegisters regs;
unsigned char* memory;
bool NMIDisabled = 0;

unsigned char getMemory(int address) {
	return (unsigned char) memory[address];
}
int setMemory(int address, unsigned char data) {
	memory[address] = data;
	return 0;
}

int pushToStack8(unsigned char data) {
	setMemory(0x100 + regs.SP, data);
	regs.SP = (regs.SP - 1) & 0xff;
	return 0;
}
int pushToStack16(int data) {
	setMemory(0x100 + regs.SP, (data & 0x00ff));
	regs.SP = (regs.SP - 1) & 0xff;
	setMemory(0x100 + regs.SP, (data & 0xff00) >> 8);
	regs.SP = (regs.SP - 1) & 0xff;
	return 0;
}
int pushToStackF(CPUFlags data) {
	unsigned char flags = 0;
	if (data.Carry) flags |= 0x01;
	if (data.Zero) flags |= 0x02;
	if (data.InterruptDis) flags |= 0x04;
	if (data.Decimal) flags |= 0x08;
	if (data.Break) flags |= 0x10;
	if (data.Overflow) flags |= 0x20;
	if (data.Negative) flags |= 0x40;
	pushToStack8(flags);
	return 0;
}
unsigned char popFromStack8() {
	regs.SP = (regs.SP + 1) & 0xff;
	return getMemory(0x100 + regs.SP);
}
int popFromStack16() {
	regs.SP = (regs.SP + 1) & 0xff;
	int ret = getMemory(0x100 + regs.SP) << 8;
	regs.SP = (regs.SP + 1) & 0xff;
	ret = ret | getMemory(0x100 + regs.SP);
	return ret;
}
CPUFlags popFromStackF() {
	regs.SP = (regs.SP + 1) & 0xff;
	unsigned char flagData = getMemory(0x100 + regs.SP);
	CPUFlags ret;
	if (flagData & 0x01) ret.Carry = 1;
	if (flagData & 0x02) ret.Zero = 1;
	if (flagData & 0x04) ret.InterruptDis = 1;
	if (flagData & 0x08) ret.Decimal = 1;
	if (flagData & 0x10) ret.Break = 1;
	if (flagData & 0x20) ret.Overflow = 1;
	if (flagData & 0x40) ret.Negative = 1;
	return ret;
}

int setMemoryPointer(unsigned char* memPointer) {
	memory = memPointer;
	return 0;
}
int execInstruction() {
	unsigned char instruction = getMemory(regs.PC);
	regs.PC = (regs.PC + 1) & 0xffff;
	switch(instruction & 0x0f) {
		case 0x00: {
			switch((instruction & 0xf0) >> 4) {
				case 0x00: { //BRK impl
					pushToStack16((regs.PC + 1) & 0xffff);
					regs.FLAGS.Break = 1;
					pushToStackF(regs.FLAGS);
					regs.PC = getMemory(0xfffe) | (getMemory(0xffff) << 8); //load data in 0xfffe to pc
					regs.FLAGS.InterruptDis = 1; //disable interrupts
					break;
				}
				case 0x01: { //BPL rel
					signed char jumpOffset = (signed char) getMemory(regs.PC);
					regs.PC = (regs.PC + 1) & 0xffff;
					if (regs.FLAGS.Negative == 0) regs.PC = (regs.PC + jumpOffset) & 0xffff;
					break;
				}
				case 0x02: { //JSR abs
					pushToStack16((regs.PC + 1) & 0xffff);
					regs.PC = getMemory(regs.PC) | (getMemory(regs.PC + 1) << 8);
					break;
				}
				case 0x03: { //BMI rel
					signed char jumpOffset = (signed char) getMemory(regs.PC);
					regs.PC = (regs.PC + 1) & 0xffff;
					if (regs.FLAGS.Negative == 1) regs.PC = (regs.PC + jumpOffset) & 0xffff;
					break;
				}
				case 0x04: { //RTI impl
					bool breakFlag = regs.FLAGS.Break;
					regs.FLAGS = popFromStackF();
					regs.FLAGS.Break = breakFlag; //preserve break flag
					regs.PC = popFromStack16();
					break;
				}
				case 0x05: { //BVC rel
					signed char jumpOffset = (signed char) getMemory(regs.PC);
					regs.PC = (regs.PC + 1) & 0xffff;
					if (regs.FLAGS.Overflow == 0) regs.PC = (regs.PC + jumpOffset) & 0xffff;
					break;
				}
				case 0x06: { //RTS impl
					regs.PC = popFromStack16();
					regs.PC = (regs.PC + 1) & 0xffff; //add one because jsr pushes one byte to early on the stack (instructions can only add 0, 1 or 2 to the PC)
					break;
				}
				case 0x07: { //BVS rel
					signed char jumpOffset = (signed char) getMemory(regs.PC);
					regs.PC = (regs.PC + 1) & 0xffff;
					if (regs.FLAGS.Overflow == 1) regs.PC = (regs.PC + jumpOffset) & 0xffff;
					break;
				}
				case 0x08: { //undef
					break;
				}
				case 0x09: { //BCC rel
					signed char jumpOffset = (signed char) getMemory(regs.PC);
					regs.PC = (regs.PC + 1) & 0xffff;
					if (regs.FLAGS.Carry == 0) regs.PC = (regs.PC + jumpOffset) & 0xffff;
					break;
				}
				case 0x0a: { //LDY #
					regs.Y = getMemory(regs.PC);
					regs.PC = (regs.PC + 1) & 0xffff;
					regs.FLAGS.Negative = ((regs.Y & 0x80) > 0);
					regs.FLAGS.Zero = (regs.Y == 0);
					break;
				}
				case 0x0b: { //BCS rel
					signed char jumpOffset = (signed char) getMemory(regs.PC);
					regs.PC = (regs.PC + 1) & 0xffff;
					if (regs.FLAGS.Carry == 1) regs.PC = (regs.PC + jumpOffset) & 0xffff;
					break;
				}
				case 0x0c: { //CPY #
					int tmp = (int) (regs.Y - getMemory(regs.PC));
					unsigned char tmpC = regs.Y - getMemory(regs.PC);
					regs.PC = (regs.PC + 1) & 0xffff;
					regs.FLAGS.Zero = (tmp == 0);
					regs.FLAGS.Negative = (tmpC & 0x80) > 0;
					regs.FLAGS.Carry = (tmp & 0x0100) > 0;
					break;
				}
				case 0x0d: { //BNE rel
					signed char jumpOffset = (signed char) getMemory(regs.PC);
					regs.PC = (regs.PC + 1) & 0xffff;
					if (regs.FLAGS.Zero == 0) regs.PC = (regs.PC + jumpOffset) & 0xffff;
					break;
				}
				case 0x0e: { //CPX #
					int tmp = (int) (regs.X - getMemory(regs.PC));
					unsigned char tmpC = regs.X - getMemory(regs.PC);
					regs.PC = (regs.PC + 1) & 0xffff;
					regs.FLAGS.Zero = (tmp == 0);
					regs.FLAGS.Negative = (tmpC & 0x80) > 0;
					regs.FLAGS.Carry = (tmp & 0x0100) > 0;
					break;
				}
				case 0x0f: { //BEQ rel
					signed char jumpOffset = (signed char) getMemory(regs.PC);
					regs.PC = (regs.PC + 1) & 0xffff;
					if (regs.FLAGS.Zero == 1) regs.PC = (regs.PC + jumpOffset) & 0xffff;
					break;
				}
			}
			break;
		}
		case 0x01: {
			switch((instruction & 0xf0) >> 4) {
				case 0x00: { //ORA X, ind
					unsigned char indirectAddressPointer = (getMemory(regs.PC) + regs.X) & 0xff;
					regs.PC = (regs.PC + 1) & 0xffff;
					int indirectAddress = getMemory(indirectAddressPointer) | ((getMemory(indirectAddressPointer + 1) << 8) & 0xff00);
					regs.A |= getMemory(indirectAddress);
					regs.FLAGS.Zero = (regs.A == 0x00);
					regs.FLAGS.Negative = ((regs.A & 0x80) > 0);
					break;
				}
				case 0x01: { //ORA ind, Y
					unsigned char indirectAddressPointer = getMemory(regs.PC);
					regs.PC = (regs.PC + 1) & 0xffff;
					int indirectAddress = ((getMemory(indirectAddressPointer) + regs.Y) & 0xff) | ((getMemory(indirectAddressPointer + 1) << 8) & 0xff00);
					regs.A |= getMemory(indirectAddress);
					regs.FLAGS.Zero = (regs.A == 0x00);
					regs.FLAGS.Negative = ((regs.A & 0x80) > 0);
					break;
				}
				case 0x02: { //AND X, ind
					unsigned char indirectAddressPointer = (getMemory(regs.PC) + regs.X) & 0xff;
					regs.PC = (regs.PC + 1) & 0xffff;
					int indirectAddress = getMemory(indirectAddressPointer) | ((getMemory(indirectAddressPointer + 1) << 8) & 0xff00);
					regs.A &= getMemory(indirectAddress);
					regs.FLAGS.Zero = (regs.A == 0x00);
					regs.FLAGS.Negative = ((regs.A & 0x80) > 0);
					break;
				}
				case 0x03: { //AND ind, Y
					unsigned char indirectAddressPointer = getMemory(regs.PC);
					regs.PC = (regs.PC + 1) & 0xffff;
					int indirectAddress = ((getMemory(indirectAddressPointer) + regs.Y) & 0xff) | ((getMemory(indirectAddressPointer + 1) << 8) & 0xff00);
					regs.A &= getMemory(indirectAddress);
					regs.FLAGS.Zero = (regs.A == 0x00);
					regs.FLAGS.Negative = ((regs.A & 0x80) > 0);
					break;
				}
				case 0x04: { //EOR X, ind
					unsigned char indirectAddressPointer = (getMemory(regs.PC) + regs.X) & 0xff;
					regs.PC = (regs.PC + 1) & 0xffff;
					int indirectAddress = getMemory(indirectAddressPointer) | ((getMemory(indirectAddressPointer + 1) << 8) & 0xff00);
					regs.A ^= getMemory(indirectAddress);
					regs.FLAGS.Zero = (regs.A == 0x00);
					regs.FLAGS.Negative = ((regs.A & 0x80) > 0);
					break;
				}
				case 0x05: { //EOR ind, Y
					unsigned char indirectAddressPointer = getMemory(regs.PC);
					regs.PC = (regs.PC + 1) & 0xffff;
					int indirectAddress = ((getMemory(indirectAddressPointer) + regs.Y) & 0xff) | ((getMemory(indirectAddressPointer + 1) << 8) & 0xff00);
					regs.A ^= getMemory(indirectAddress);
					regs.FLAGS.Zero = (regs.A == 0x00);
					regs.FLAGS.Negative = ((regs.A & 0x80) > 0);
					break;
				}
				case 0x06: { //ADC X, ind
					unsigned char indirectAddressPointer = (getMemory(regs.PC) + regs.X) & 0xff;
					regs.PC = (regs.PC + 1) & 0xffff;
					int indirectAddress = getMemory(indirectAddressPointer) | ((getMemory(indirectAddressPointer + 1) << 8) & 0xff00);
					int tmp = regs.A + getMemory(indirectAddress);
					if (regs.FLAGS.Carry) tmp++;
					regs.A = tmp & 0x00ff;
					regs.FLAGS.Zero = (regs.A == 0x00);
					regs.FLAGS.Negative = ((regs.A & 0x80) > 0);
					regs.FLAGS.Carry = ((tmp & 0x0100) > 0);
					break;
				}
				case 0x07: { //ADC ind, Y
					unsigned char indirectAddressPointer = getMemory(regs.PC);
					regs.PC = (regs.PC + 1) & 0xffff;
					int indirectAddress = ((getMemory(indirectAddressPointer) + regs.Y) & 0xff) | ((getMemory(indirectAddressPointer + 1) << 8) & 0xff00);
					int tmp = regs.A + getMemory(indirectAddress);
					if (regs.FLAGS.Carry) tmp++;
					regs.A = tmp & 0x00ff;
					regs.FLAGS.Zero = (regs.A == 0x00);
					regs.FLAGS.Negative = ((regs.A & 0x80) > 0);
					regs.FLAGS.Carry = ((tmp & 0x0100) > 0);
					break;
				}
				case 0x08: { //STA X, ind
					unsigned char indirectAddressPointer = (getMemory(regs.PC) + regs.X) & 0xff;
					regs.PC = (regs.PC + 1) & 0xffff;
					int indirectAddress = getMemory(indirectAddressPointer) | ((getMemory(indirectAddressPointer + 1) << 8) & 0xff00);
					setMemory(indirectAddress, regs.A);
					break;
				}
				case 0x09: { //STA ind, Y
					unsigned char indirectAddressPointer = getMemory(regs.PC);
					regs.PC = (regs.PC + 1) & 0xffff;
					int indirectAddress = ((getMemory(indirectAddressPointer) + regs.Y) & 0xff) | ((getMemory(indirectAddressPointer + 1) << 8) & 0xff00);
					setMemory(indirectAddress, regs.A);
					break;
				}
				case 0x0a: { //LDA X, ind
					unsigned char indirectAddressPointer = (getMemory(regs.PC) + regs.X) & 0xff;
					regs.PC = (regs.PC + 1) & 0xffff;
					int indirectAddress = getMemory(indirectAddressPointer) | ((getMemory(indirectAddressPointer + 1) << 8) & 0xff00);
					regs.A = getMemory(indirectAddress);
					regs.FLAGS.Zero = (regs.A == 0x00);
					regs.FLAGS.Negative = ((regs.A & 0x80) > 0);
					break;
				}
				case 0x0b: { //LDA ind, Y
					unsigned char indirectAddressPointer = getMemory(regs.PC);
					regs.PC = (regs.PC + 1) & 0xffff;
					int indirectAddress = ((getMemory(indirectAddressPointer) + regs.Y) & 0xff) | ((getMemory(indirectAddressPointer + 1) << 8) & 0xff00);
					regs.A = getMemory(indirectAddress);
					regs.FLAGS.Zero = (regs.A == 0x00);
					regs.FLAGS.Negative = ((regs.A & 0x80) > 0);
					break;
				}
				case 0x0c: { //CMP X, ind
					unsigned char indirectAddressPointer = (getMemory(regs.PC) + regs.X) & 0xff;
					regs.PC = (regs.PC + 1) & 0xffff;
					int indirectAddress = getMemory(indirectAddressPointer) | ((getMemory(indirectAddressPointer + 1) << 8) & 0xff00);
					unsigned int tmp = regs.A - getMemory(indirectAddress);
					regs.FLAGS.Carry = ((tmp & 0x0100) > 0);
					regs.FLAGS.Negative = ((tmp & 0x80) > 0);
					regs.FLAGS.Zero = ((tmp & 0x00ff) == 0);
					break;
				}
				case 0x0d: { //CMP ind, Y
					unsigned char indirectAddressPointer = getMemory(regs.PC);
					regs.PC = (regs.PC + 1) & 0xffff;
					int indirectAddress = ((getMemory(indirectAddressPointer) + regs.Y) & 0xff) | ((getMemory(indirectAddressPointer + 1) << 8) & 0xff00);
					unsigned int tmp = regs.A - getMemory(indirectAddress);
					regs.FLAGS.Carry = ((tmp & 0x0100) > 0);
					regs.FLAGS.Negative = ((tmp & 0x80) > 0);
					regs.FLAGS.Zero = ((tmp & 0x00ff) == 0);
					break;
				}
				case 0x0e: { //SBC X, ind
					unsigned char indirectAddressPointer = (getMemory(regs.PC) + regs.X) & 0xff;
					regs.PC = (regs.PC + 1) & 0xffff;
					int indirectAddress = getMemory(indirectAddressPointer) | ((getMemory(indirectAddressPointer + 1) << 8) & 0xff00);
					unsigned int tmp = regs.A - getMemory(indirectAddress);
					if (regs.FLAGS.Carry) tmp = tmp - 1;
					regs.FLAGS.Carry = ((tmp & 0x0100) > 0);
					regs.FLAGS.Negative = ((tmp & 0x80) > 0);
					regs.FLAGS.Zero = ((tmp & 0x00ff) == 0);
					regs.A = (tmp & 0xff);
					break;
				}
				case 0x0f: { //SBC ind, Y
					unsigned char indirectAddressPointer = getMemory(regs.PC);
					regs.PC = (regs.PC + 1) & 0xffff;
					int indirectAddress = ((getMemory(indirectAddressPointer) + regs.Y) & 0xff) | ((getMemory(indirectAddressPointer + 1) << 8) & 0xff00);
					unsigned int tmp = regs.A - getMemory(indirectAddress);
					if (regs.FLAGS.Carry) tmp = tmp - 1;
					regs.FLAGS.Carry = ((tmp & 0x0100) > 0);
					regs.FLAGS.Negative = ((tmp & 0x80) > 0);
					regs.FLAGS.Zero = ((tmp & 0x00ff) == 0);
					regs.A = (tmp & 0xff);
					break;
				}
			}
			break;
		}
		case 0x02: //only one instruction in this group (LDX #)
			regs.X = getMemory(regs.PC);
			regs.PC = (regs.PC + 1) & 0xffff;
			regs.FLAGS.Negative = ((regs.X & 0x80) > 0);
			regs.FLAGS.Zero = (regs.X == 0);
			break;
		case 0x03:
			break; //no instructions in this group
		case 0x04:
			break;
		case 0x05:
			break;
		case 0x06:
			break;
		case 0x07:
			break; //no instructions in this group
		case 0x08:
			break;
		case 0x09:
			break;
		case 0x0a:
			break;
		case 0x0b:
			break; //no instructions in this group
		case 0x0c:
			break;
		case 0x0d:
			break;
		case 0x0e:
			break;
		case 0x0f:
			break; //no instructions in this group
	}
	return 0;
}
int triggerInterrupt(bool maskable) {
	if (maskable) {
		if (regs.FLAGS.InterruptDis) return 1; //interrupt disable bit is set. Interrupt gets ignored

		pushToStack16(regs.PC); //push PC to stack
		pushToStackF(regs.FLAGS); //push flags to stack
		regs.PC = getMemory(0xfffe) | (getMemory(0xffff) << 8); //load data in 0xfffe to pc
		regs.FLAGS.InterruptDis = 1; //disable interrupts
		return 0;
	} else {
		if (!NMIDisabled) {
			pushToStack16(regs.PC); //push PC to stack
			pushToStackF(regs.FLAGS); //push flags to stack
			regs.PC = getMemory(0xfffa) | (getMemory(0xfffb) << 8); //load data in 0xfffa to pc
			regs.FLAGS.InterruptDis = 1; //disable interrupts
			NMIDisabled = 1;
			return 0;
		} else {
			return 1;
		}
	}
}
