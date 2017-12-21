#include <cstring>
#include "Processor.h"

/**
 * Create a new CPU object with all zero data. Will create a 32KB core by default.
 */
Processor::Processor(){
	for (int i = 0; i < REGCOUNT; i++)
		registers[i] = 0;
	ps = 0;
	core = new PBYTE[1<<15];
	coreSize = 1<<15;
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
	core = new PBYTE[cpu.coreSize];
	memcpy(core, cpu.core, (size_t)cpu.coreSize);
	coreSize = cpu.coreSize;
	halted = false;
}

Processor::~Processor() {
	delete core;
}

void Processor::operator=(const Processor& cpu){
	for (int i = 0; i < REGCOUNT; i++)
		registers[i] = cpu.registers[i];
	ps = cpu.ps;
	core = new PBYTE[cpu.coreSize];
	memcpy(core, cpu.core, (size_t)cpu.coreSize);
	coreSize = cpu.coreSize;
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
// Todo: Redo all addressing
// Todo: Make sure all status flags get set as they should
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
void Processor::tst(PWORD *o1) {
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
void Processor::ror(PWORD *o1) {
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
void Processor::rol(PWORD *o1) {
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
void Processor::asr(PWORD *o1) {
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
void Processor::asl(PWORD *o1) {
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
void Processor::swab(PWORD *o1) {
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
	*o1 = (PWORD)(pstat_neg() ? 0xFFFF : 0x000);
	*o1 == 0 ? sez() : clz();
}

void Processor::mul(PWORD *o1, PWORD *o2) {

}

void Processor::div(PWORD *o1, PWORD *o2) {

}

void Processor::ash(PWORD *o1, PWORD *o2) {

}

void Processor::ashc(PWORD *o1, PWORD *o2) {

}

void Processor::kxor(PWORD *o1, PWORD *o2) {

}

void Processor::mov(PWORD *o1, PWORD *o2) {

}

void Processor::add(PWORD *o1, PWORD *o2) {

}

void Processor::sub(PWORD *o1, PWORD *o2) {

}

void Processor::cmp(PWORD *o1, PWORD *o2) {

}

void Processor::bis(PWORD *o1, PWORD *o2) {

}

void Processor::bic(PWORD *o1, PWORD *o2) {

}

void Processor::bit(PWORD *o1, PWORD *o2) {

}

void Processor::br(PWORD *ost) {

}

void Processor::bne(PWORD *ost) {

}

void Processor::beq(PWORD *ost) {

}

void Processor::bpl(PWORD *ost) {

}

void Processor::bmi(PWORD *ost) {

}

void Processor::bvc(PWORD *ost) {

}

void Processor::bvs(PWORD *ost) {

}

void Processor::bhis(PWORD *ost) {

}

void Processor::bcc(PWORD *ost) {

}

void Processor::blo(PWORD *ost) {

}

void Processor::bcs(PWORD *ost) {

}

void Processor::bge(PWORD *ost) {

}

void Processor::blt(PWORD *ost) {

}

void Processor::bgt(PWORD *ost) {

}

void Processor::ble(PWORD *ost) {

}

void Processor::bhi(PWORD *ost) {

}

void Processor::blos(PWORD ost) {

}

void Processor::jsr(PWORD *ost) {

}

void Processor::rts() {

}

void Processor::rti() {

}

void Processor::trap() {

}

void Processor::bpt() {

}

void Processor::iot() {

}

void Processor::emt() {

}

void Processor::rtt() {

}

void Processor::spl(PWORD *lvl) {

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
 * Use value scheme to set status word flags
 * @param o1 First argument
 * @param o2 Second argument
 * @param res Result of operation
 */
void Processor::valFlags(PWORD o1, PWORD o2, PWORD res) {
	res & NEG_BIT	? sen() : cln();
	res == 0		? sez() : clz();
	if (!(NEG_BIT & o1) && !(NEG_BIT & o2) && (NEG_BIT & res))
		sev();
	else if ((NEG_BIT & o1) && (NEG_BIT & o2) && !(NEG_BIT & res))
		sev();
	else
		clv();
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
