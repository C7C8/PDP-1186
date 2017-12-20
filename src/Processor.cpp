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
		(*o1);
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
	*o1 ^= NEG_BIT;
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
	PWORD lsd = *o1 & (PWORD)1;
	*o1 >>= 1;
	*o1 = (*o1 & ~NEG_BIT) | (lsd << 15);
	*o1 == 0 ? sez() : clz();
	*o1 & NEG_BIT ? sen() : cln();
	sec(); //TODO spec unclear, determine how C should be set
	pstat_neg() ^ pstat_carry() ? sev() : clv();
}

/**
 * Rotate value left. NZ set by result, C set (???), V=N^C
 */
void Processor::rol(PWORD *o1) {
	PWORD msd = *o1 & NEG_BIT;
	*o1 <<= 1;
	*o1 &= msd>>15;
	*o1 == 0 ? sez() : clz();
	*o1 & NEG_BIT ? sen() : cln();
	sec(); //TODO spec unclear, determine how C should be set
	pstat_neg() ^ pstat_carry() ? sev() : clv();
}

/**
 * Arithmetic right shift. NZ set by result, C set by old low bit, V=N^C
 * @note This is PLATFORM/IMPLEMENTATION DEPENDENT CODE, or I think it is anyways.
 * This is because on x86 with GCC, the default implementation of right shift is
 * arithmetic right shift, but this might not hold for other platforms.
 */
void Processor::asr(PWORD *o1) {
	*o1 & 1 ? sec() : clc();
	*o1 >>= 1;
	*o1 == 0 ? sez() : clz();
	*o1 & NEG_BIT ? sen() : cln();
	pstat_neg() ^ pstat_carry() ? sev() : clv();
}

/**
 * Left shift. NZ set by result, C set by old high bit, V=N^C
 */
void Processor::asl(PWORD *o1) {
	*o1 & NEG_BIT ? sec() : clc();
	*o1 <<= 1;
	*o1 == 0 ? sez() : clz();
	*o1 & NEG_BIT ? sen() : cln();
	pstat_neg() ^ pstat_carry() ? sev() : clv();
}

/**
 * Swap bytes in a word. CV reset, NZ set by low byte of result.
 */
void Processor::swab(PWORD *o1) {
	//TODO spec unclear, claims "CV reset, NV set by low byte". Corrected to NZ?
	*o1 = ((*o1 & (PWORD)0xFF00) >> 8) | ((*o1 & (PWORD)0x00FF) << 8);
	clc();
	clv();
	(*o1 & (PWORD)0x00FF) == 0 ? sez() : clz();
	(*o1 & (PWORD)0xFF00) & NEG_BIT ? sen() : cln();
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
	ps = (ps & (PWORD)~SC);
}

/**
 * Clear the overflow flag
 */
void Processor::clv() {
	ps = (ps & (PWORD)~SV);
}

/**
 * Clear the zero flag
 */
void Processor::clz() {
	ps = (ps & (PWORD)~SZ);
}

/**
 * Clear the negative flag
 */
void Processor::cln() {
	ps = (ps & (PWORD)~SN);
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
