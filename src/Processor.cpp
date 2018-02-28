#include <cstring>
#include "Processor.h"


/**
 * Create a new CPU object with all zero data. Will create a 32KB core by default.
 */
Processor::Processor(){
	for (int i = 0; i < REGCOUNT; i++) // NOLINT
		registers[i] = 0;
	ps = 0;
	core.byte = new PBYTE[1<<15];
	coreSizeBytes = 1<<15;
	halted = false;
}

/**
 * Copy constructor
 * @param cpu CPU to copy from
 */
Processor::Processor(const Processor& cpu){
	for (int i = 0; i < REGCOUNT; i++)
		registers[i] = cpu.registers[i];
	ps = cpu.ps;
	core.byte = new PBYTE[cpu.coreSizeBytes];
	memcpy(core.byte, cpu.core.byte, (size_t)cpu.coreSizeBytes);
	coreSizeBytes = cpu.coreSizeBytes;
	halted = false;
}

Processor::~Processor() {
	delete core.byte;
}

void Processor::operator=(const Processor& cpu){ // NOLINT
	for (int i = 0; i < REGCOUNT; i++)
		registers[i] = cpu.registers[i];
	ps = cpu.ps;
	core.byte = new PBYTE[cpu.coreSizeBytes];
	memcpy(core.byte, cpu.core.byte, (size_t)cpu.coreSizeBytes);
	coreSizeBytes = cpu.coreSizeBytes;
	halted = false;
}

/**
 * Get contents of register
 * @param reg Register to access
 * @return Word in register
 */
PWORD Processor::reg(RegCode reg) const {
	return registers[reg];
}

/**
 * Set contents of register
 * @param reg Register to access
 * @param val Word to set
 */
void Processor::reg(RegCode reg, PWORD val) {
	registers[reg] = val;
}

/**
 * Get processor status word
 * @return Processor status word
 */
PWORD Processor::pstat() const{
	return ps;
}

/**
 * Whether the operation caused a carry from the most significant bit
 */
bool Processor::pstat_carry() const {
	return (bool)(ps & SC);
}

/**
 * Whether the operation caused an arithmetic overflow
 */
bool Processor::pstat_overf() const {
	return (bool)(ps & SV);
}

/**
 * Whether the result of the last operation was zero
 */
bool Processor::pstat_zero() const {
	return (bool)(ps & SZ);
}

/**
 * Whether the result of the last operation was negative
 */
bool Processor::pstat_neg() const {
	return (bool)(ps & SN);
}

/**
 * Whether the trap flag is set or not
 */
bool Processor::pstat_trap() const {
	return (bool)(ps & ST);
}

/**
 * Get the priority of the processor as defined by bits 5-7 of the status word
 * @return Priority number
 */
PWORD Processor::priority() const {
	return (ps & (PWORD)0xe0) >> 5;
}

void Processor::priority(PWORD prty) {
	if (prty > 7)
		return;
	prty <<= 5;
	ps = (ps & (PWORD)~0xe0) | prty;
}

//
// INSTRUCTIONS
// TODO Redo all addressing
// TODO Make sure all status flags get set as they should
//

/**
 * Halt the CPU until restarted
 */
void Processor::halt() {
	halted = true;
}

/**
 * Halt the CPU until restarted or interrupted
 */
void Processor::wait() {
	//TODO Allow processor to be restarted by interrupt
	halted = true;
}

/**
 * Reset all IO devices
 */
void Processor::reset() {
	//Unimplemented
}

/**
 * Execute no operation
 */
void Processor::nop() {
}

/**
 * Clear value. Sets Z, clears NVC
 */
void Processor::clr(PWORD *o1) {
	*o1 = 0;
	sez();
	cln();
	clv();
	clc();
}

/**
 * Increment value by one. Flags set by value scheme
 */
void Processor::inc(PWORD *o1) {
	PWORD prev = *o1;
	(*o1)++;
	valFlags(prev, prev, *o1);
}

/**
 * Decrement value by one. Flags set by value scheme
 */
void Processor::dec(PWORD *o1) {
	PWORD prev = *o1;
	(*o1)--;
	valFlags(prev, prev, *o1);
}

/**
 * Add 1 if carry flag set. Flags set by value scheme
 */
void Processor::adc(PWORD *o1) {
	PWORD prev = *o1;
	if (pstat_carry())
		(*o1)++;
	valFlags(prev, prev, *o1);
}

/**
 * Subtract 1 if carry flag set. Flags set by value scheme
 */
void Processor::sbc(PWORD *o1) {
	PWORD prev = *o1;
	if (pstat_carry())
		(*o1)--;
	valFlags(prev, prev, *o1);
}

/**
 * Set condition codes by value.
 */
void Processor::tst(const PWORD *o1) {
	*o1 & NEG_BIT	? sen() : cln();
	*o1 == 0		? sez() : clz();
	clv();
	clc();
}

/**
 * Invert value. Flags set by value scheme; if result is zero, reset C, else set it
 */
void Processor::neg(PWORD *o1) {
	PWORD prev = *o1;
	*o1 = ~*o1 + (PWORD)1;
	valFlags(prev, prev, *o1);
	*o1 == 0 ? clc() : sec();
}

/**
 * Take one's complement of input. NZ set by result, V reset, C set
 */
void Processor::com(PWORD *o1) {
	*o1 = ~*o1;
	*o1 == 0 ? sez() : clz();
	*o1 && NEG_BIT ? sen() : sez();
	clv();
	sec();
}

/**
 * Rotate value right. NZ set by result, C set (???), V=N^C
 */
void Processor::ror(const PWORD *o1) {
	uint64_t flags;
	asm("rorw $1, (%0)\n\t"
		"pushf\n\t"
		"popq %1"
		: "=r" (o1), "=r" (flags)
		: "r" (o1)
		);
	x86Flags(flags);
	sec(); //What?
	pstat_neg() ^ pstat_carry() ? sev() : clv();
}

/**
 * Rotate value left. NZ set by result, C set (???), V=N^C
 */
void Processor::rol(const PWORD *o1) {
	uint64_t flags;
	asm("rolw $1, (%0)\n\t"
		"pushf\n\t"
		"popq %1"
		: "=r" (o1), "=r" (flags)
		: "r" (o1)
		);
	x86Flags(flags);
	sec(); //What?
	pstat_neg() ^ pstat_carry() ? sev() : clv(); //The specs say that V=N^C. I happen to disagree, and so does x86. But, whatver.
}

/**
 * Arithmetic right shift. NZ set by result, C set by old low bit, V=N^C
 */
void Processor::asr(const PWORD *o1) {
	//<<= doesn't work and I'm too lazy to figure out why
	uint64_t flags;
	asm("sarw $1, (%0)\n\t"
		"pushf\n\t"
		"popq %1"
		: "=r" (o1), "=r" (flags)
		: "r" (o1)
		);
	x86Flags(flags);
	pstat_neg() ^ pstat_carry() ? sev() : clv();
}

/**
 * Left shift. NZ set by result, C set by old high bit, V=N^C
 */
void Processor::asl(const PWORD *o1) {
	uint64_t flags;
	asm("salw $1, (%0)\n\t"
		"pushf\n\t"
		"popq %1"
		: "=r" (o1), "=r" (flags)
		: "r" (o1)
		);
	x86Flags(flags);
	pstat_neg() ^ pstat_carry() ? sev() : clv();
}

/**
 * Swap bytes in a word. CV reset, NZ set by low byte of result.
 */
void Processor::swab(const PWORD *o1) {
	//TODO spec unclear, claims "CV reset, NV set by low byte". Corrected to NZ?
	asm("movw (%0), %%ax\n\t"
		"xchgb %%ah, %%al\n\t"
		"movw %%ax, (%0)\n\t"
		: "=r" (o1)
		: "r" (o1)
		: "%ax"
		);

	clc();
	clv();
	(*o1 & (PWORD)0x00FF) == 0 ? sez() : clz();
	*o1 & NEG_BIT>>8 ? sen() : cln();
}

/**
 * Sign extend; sets all bits to be the same as the code N. Z set by result.
 */
void Processor::sxt(PWORD *o1) {
	*o1 = (PWORD)(pstat_neg() ? ~0x0000 : 0x000);
	*o1 == 0 ? sez() : clz();
}

/**
 * Multiply register by address
 * TODO implement spec "C set if low word overflows"
 */
void Processor::mul(const RegCode reg, const PWORD *o2) {
	registers[reg] *= *o2;
	registers[reg] == 0			? sez() : clz();
	registers[reg] & NEG_BIT 	? sen() : cln();
	clv();
}

/**
 * Divide register by address
 */
void Processor::div(const RegCode reg, const PWORD *o2) {
	if (*o2 == 0){
		sec();
		sev();
	}
	else {
		PWORD orig = registers[reg];
		registers[reg] /= *o2;
		overflow(orig, *o2, registers[reg]) ? sev() : clv();
	}

}

/**
 * Shift register by low 6 bits of address. Shift value interpreted as signed,
 * so negative means right shift.
 */
void Processor::ash(const RegCode reg, const PWORD *o2) {
	//Convert this into something we can work with, then shift by it
	PWORD prev = registers[reg];
	SPWORD conv = (*o2 & (SPWORD)0x1F) | ((*o2 & 0x20) ? (SPWORD)0xFFE0 : (SPWORD)0); //extend sign bit, lazily
	if (conv < 0)
		registers[reg] >>= -conv;
	else
		registers[reg] <<= conv;
	if (registers[reg] & NEG_BIT != prev & NEG_BIT)
		sev();
	//TODO FOR THE LOVE OF CTHULHU TEST THIS
	prev <<= (conv - (conv < 0 ? -1 : 1));
	if (conv < 0 ? prev & 1 : (prev & NEG_BIT) != 0)
		sec();
	else
		clc();
}

/**
 * Not implemented, has absolutely no effect
 */
void Processor::ashc(const RegCode reg, PWORD *o2) {
	//TODO Implement, whenever this instruction becomes relevant (I still don't understand the ref manual)
}

/**
 * XOR register with value
 */
void Processor::xor_(const RegCode reg, const PWORD *o2) {
	const PWORD prev = registers[reg];
	registers[reg] ^= *o2;
	bitFlags(prev, *o2, registers[reg]);
}

/**
 * Move src into dst
 * @param o1 src
 * @param o2 dst
 */
void Processor::mov(const PWORD *o1, PWORD *o2) {
	valFlags(*o1, *o2, *o1);
	*o2 = *o1;
	clv();
}

/**
 * dst = dst + src
 * @param o1 src
 * @param o2 dst
 */
void Processor::add(const PWORD *o1, PWORD *o2) {
	//TODO Double check that flags are set appropriately, esp. overflow
	valFlags(*o1, *o2, *o2 + *o1);
	*o2 += *o1;
}

/**
 * dst = dst - src;
 * @param o1 src
 * @param o2 dst
 */
void Processor::sub(const PWORD *o1, PWORD *o2) {
	valFlags(*o1, *o2, *o2 - *o1);
	*o2 -= *o1;
}

/**
 * Compare; Perform src - dst but don't store the results, just alter flags
 * @note The order of subtraction here is not the same as in sub, it's reversed!
 * @param o1 src
 * @param o2 dst
 */
void Processor::cmp(const PWORD *o1, const PWORD *o2) {
	valFlags(*o1, *o2, *o1 - *o2);
}

/**
 * OR (bit set); dst = dst | src
 * @param o1 src
 * @param o2 dst
 */
void Processor::bis(const PWORD *o1, PWORD *o2) {
	bitFlags(*o1, *o2, *o1 | *o2);
	*o2 |= *o1;
}

/**
 * AND; dst = dst & src
 * @param o1 src
 * @param o2 dst
 */
void Processor::bic(const PWORD *o1, PWORD *o2) {
	bitFlags(*o1, *o2, *o1 & *o2);
	*o2 &= *o1;
}

/**
 * Bit test; perform dst & src but don't store the results, just alter flags
 * @param o1 src
 * @param o2 dst
 */
void Processor::bit(const PWORD *o1, const PWORD *o2) {
	bitFlags(*o1, *o2, *o1 & *o2);
}

/**
 * Unconditional branch
 * @param ost Offset
 */
void Processor::br(const PWORD *ost) {
	branch(*ost);
}

/**
 * Branch on not equal
 * @param ost Offset
 */
void Processor::bne(const PWORD *ost) {
	if (!pstat_zero())
		branch(*ost);
}

/**
 * Brnach on equal
 * @param ost Offset
 */
void Processor::beq(const PWORD *ost) {
	if (pstat_zero())
		branch(*ost);
}

/**
 * Branch on positive
 * @param ost Offset
 */
void Processor::bpl(const PWORD *ost) {
	if (!pstat_neg())
		branch(*ost);
}

/**
 * Branch on negative
 * @param ost Offset
 */
void Processor::bmi(const PWORD *ost) {
	if (pstat_neg())
		branch(*ost);
}

/**
 * Branch on overflow clear
 * @param ost Offset
 */
void Processor::bvc(const PWORD *ost) {
	if (!pstat_overf())
		branch(*ost);
}

/**
 * Branch on overflow set
 * @param ost Offset
 */
void Processor::bvs(const PWORD *ost) {
	if (pstat_overf())
		branch(*ost);
}

/**
 * Branch on higher than or same as
 * @param ost Offset
 */
void Processor::bhis(const PWORD *ost) {
	if (!pstat_carry())
		branch(*ost);
}

/**
 * BGranch on carry clear
 * @param ost Offset
 */
void Processor::bcc(const PWORD *ost) {
	if (!pstat_carry())
		branch(*ost);
}

/**
 * Branch on lower
 * @param ost Offset
 */
void Processor::blo(const PWORD *ost) {
	if (pstat_carry())
		branch(*ost);
}

/**
 * Branch on carry set
 * @param ost Offset
 */
void Processor::bcs(const PWORD *ost) {
	if (pstat_carry())
		branch(*ost);
}

/**
 * Branch on greater than or equal to
 * @param ost Offset
 */
void Processor::bge(const PWORD *ost) {
	if ((pstat_neg() ^ pstat_overf()) == 0)
		branch(*ost);
}

/**
 * Branch on less than
 * @param ost Offset
 */
void Processor::blt(const PWORD *ost) {
	if (pstat_neg() ^ pstat_overf())
		branch(*ost);
}

/**
 * Branch on greater than
 * @param ost Offset
 */
void Processor::bgt(const PWORD *ost) {
	if (!(pstat_zero() || (pstat_neg() ^ pstat_overf())))
		branch(*ost);
}

/**
 * Branch on less than or equal to
 * @param ost Offset
 */
void Processor::ble(const PWORD *ost) {
	if (pstat_zero() || (pstat_neg() ^ pstat_overf()))
		branch(*ost);
}

/**
 * Branch on higher than
 * @param ost Offset
 */
void Processor::bhi(const PWORD *ost) {
	if (!(pstat_carry() || pstat_zero()))
		branch(*ost);
}

/**
 * Branch on lower than or same as
 * @param ost Offset
 */
void Processor::blos(const PWORD *ost) {
	if (pstat_carry() || pstat_zero())
		branch(*ost);
}

/**
 * Jump to address
 * @param ost dst
 */
void Processor::jmp(const PWORD *ost) {
	registers[PC] = *ost;
}

/**
 * Subtract one and branch
 * @param reg Register to subtract from
 * @param ost dst
 */
void Processor::sob(const RegCode reg, const PWORD *ost) {
	registers[reg] -= 1;
	if (registers[reg] != 0)
		registers[PC] = *ost;
}

/**
 * Jump to subroutine; the contents of reg will be pushed to the stack and reg
 * will take on the previous value of the pc register
 * @param reg Register to swap pc value into
 * @param ost Address of subroutine
 */
void Processor::jsr(const RegCode reg, const PWORD *ost) {
	//Push contents of reg to stack
	registers[SP] -= 2;
	core.byte[registers[SP]] = LOWER_W(registers[reg]);
	core.byte[registers[SP] + 1] = UPPER_W(registers[reg]);
	//Copy PC register to reg
	registers[reg] = registers[PC];
	//Transfer control
	registers[PC] = *ost;
}

/**
 * Return from subroutine; copies contents of reg into pc and pops top of stack into reg
 */
void Processor::rts(const RegCode reg) {
	registers[PC] = reg;
	registers[reg] = COMPOSE(core.byte[registers[SP] + 1], core.byte[registers[SP]]);
	registers[SP] += 2;
}

/**
 * Return from interrupt (or trap)
 */
void Processor::rti() {
	registers[PC] = core.word[SP/2]; //TODO: check if this works... might be awkward if it doesn't, and it probably doesn't.
	registers[SP] += 2;
	ps = core.byte[SP];
	registers[SP] += 2;
}

/**
 * Trap
 * @param n From
 */
void Processor::trap(const PWORD n) {
	registers[PC] -= 2;
	registers[PC] = ps;
	registers[PC] -= 2;
	registers[SP] = registers[PC];
	registers[PC] = n; //TODO: Check, this trap routine can't possibly be right
	registers[PC] = n + (PWORD)2;
}

/**
 * Breakpoint trap. Used by debuggers, not sure why it's supported here, but why not.
 * Oh, by the way, it's identical to trap.
 */
void Processor::bpt(const PWORD n) {
	trap(n);
}

/**
 * I/O trap, used by OS for I/O calls. Really just trap(020) in disguise.
 */
void Processor::iot() {
	trap(020);
}

/**
 * Emulator trap, used by the OS to implement fake operations. Really just trap(030) in disguise.
 */
void Processor::emt() {
	trap(030);
}

/**
 * Return from trace trap. Same as RTI, but suppresses the trace trap (?!) that follows. Really just
 * RTI in disguise, at this point I'm pretty sure the instruction set reference manual is playing
 * tricks on me.
 */
void Processor::rtt() {
	rti();
}

/**
 * Set priority level. Sets bits 7-5 of the psw to lvl
 * @param lvl Level to set
 */
void Processor::spl(const PBYTE* lvl) {
	ps = (*lvl << 4) | (ps * (PBYTE)0b11111);
}

/**
 * Clear the carry flag;
 */
void Processor::clc() {
	ps &= ~SC;
}

/**
 * Clear the overflow flag
 */
void Processor::clv() {
	ps &= ~SV;
}

/**
 * Clear the zero flag
 */
void Processor::clz() {
	ps &= ~SZ;
}

/**
 * Clear the negative flag
 */
void Processor::cln() {
	ps &= ~SN;
}

/**
 * Set the carry flag
 */
void Processor::sec() {
	ps |= SC;
}

/**
 * Set the overflow flag
 */
void Processor::sev() {
	ps |= SV;
}

/**
 * Set the zero flag
 */
void Processor::sez() {
	ps |= SZ;
}

/**
 * Set the negative flag
 */
void Processor::sen() {
	ps |= SN;
}

/**
 * Clear all condition codes
 */
void Processor::ccc() {
	ps = (ps & (PWORD)~(SZ | SV | SC | SN));
}

/**
 * Set all condition codes
 */
void Processor::scc() {
	ps |= (SZ & SV & SC & SN);
}

/**
 * Determine if an overflow happened using two operands and the result of the operation
 * @param o1 Operand 1
 * @param o2 Operand 2
 * @param res Result of operation using given operands
 * @return True if overflow, false otherwise
 */
bool Processor::overflow(PWORD o1, PWORD o2, PWORD res) {
	if (!(NEG_BIT & o1) && !(NEG_BIT & o2) && (NEG_BIT & res))
		return true;
	else if ((NEG_BIT & o1) && (NEG_BIT & o2) && !(NEG_BIT & res))
		return true;
	return false;
}

/**
 * Use value scheme to set status word flags
 * @param o1 First argument
 * @param o2 Second argument
 * @param res Result of operation
 */
void Processor::valFlags(PWORD o1, PWORD o2, PWORD res) {
	res & NEG_BIT			? sen() : cln();
	res == 0				? sez() : clz();
	overflow(o1, o2, res)	? sev() : clv();
}

/**
 * Use bit scheme to set status word flags
 * @param o1 First argument
 * @param o2 Second argument
 * @param res Result of operation
 */
void Processor::bitFlags(PWORD o1, PWORD o2, PWORD res) {
	res & NEG_BIT	? sen() : cln();
	res == 0		? sez() : clz();
	clv();
}

/**
 * Convert an x86 processor's status word into a PDP 11's status word.
 * @param flags x86 processor status word
 */
void Processor::x86Flags(uint64_t flags) {
	flags & SC_86	?	sec() : clc();
	flags & SV_86	?	sev() : clv();
	flags & SZ_86	?	sez() : clz();
	flags & SN_86	?	sen() : cln();
}
