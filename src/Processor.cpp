#include "Processor.h"

/**
 * Create a new CPU object with all zero data. Will create a 32KB core by default.
 */
Processor::Processor(){
	for (int i = 0; i < REGCOUNT; i++)
		registers[i] = 0;
	ps = 0;
	core = new Core(1<<15);
	halted = false;
}

/**
 * Create new CPU object with supplied memory device
 * @param core
 */
Processor::Processor(Core* core) {
	for (int i = 0; i < REGCOUNT; i++)
		registers[i] = 0;
	ps = 0;
	this->core = core;
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
	core = new Core(*cpu.core);
	halted = false;
}

Processor::~Processor() {
	delete core;
}

void Processor::operator=(const Processor& cpu){
	for (int i = 0; i < REGCOUNT; i++)
		registers[i] = cpu.registers[i];
	ps = cpu.ps;
	core = new Core(*cpu.core);
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
 * Set to zero. Clears NVC, sets Z
 * @param o1 Address/register to zero
 */
void Processor::clr(PWORD o1) {
	core->write(o1, 0);
}

/**
 * Increment by one.
 * @param o1 Address/register to increment
 */
void Processor::inc(PWORD o1) {
	core->write(o1, core->read(o1) + 1);
}

/**
 * Decrement by one.
 * @param o1 Address/register to decrement
 */
void Processor::dec(PWORD o1) {
	core->write(o1, core->read(o1) - 1);
}

/**
 * Add one if carry flag set
 * @param o1 Address/register to operate on
 */
void Processor::adc(PWORD o1) {
	core->write(o1, core->read(o1) + pstat_carry() ? 1 : 0);
}

/**
 * Subtract one if carry flag set
 * @param o1 Address/register to operate on
 */
void Processor::sbc(PWORD o1) {
	core->write(o1, core->read(o1) - pstat_carry() ? 1 : 0);
}

/**
 * Set condition codes NZ using input, clears VC
 * @param o1 Address/register to test
 */
void Processor::tst(PWORD o1) {
	ccc();
	if (NEG_BIT & o1)
		sen();
	if (o1 == 0)
		sez();
}

/**
 * Invert value at address
 * @param o1 Register/address to operate on
 */
void Processor::neg(PWORD o1) {

}

void Processor::com(PWORD o1) {

}

void Processor::ror(PWORD o1) {

}

void Processor::rol(PWORD o1) {

}

void Processor::asr(PWORD o1) {

}

void Processor::asl(PWORD o1) {

}

void Processor::swab(PWORD o1) {

}

void Processor::sxt(PWORD o1) {

}

void Processor::mul(PWORD o1, PWORD o2) {

}

void Processor::div(PWORD o1, PWORD o2) {

}

void Processor::ash(PWORD o1, PWORD o2) {

}

void Processor::ashc(PWORD o1, PWORD o2) {

}

void Processor::kxor(PWORD o1, PWORD o2) {

}

void Processor::mov(PWORD o1, PWORD o2) {

}

void Processor::add(PWORD o1, PWORD o2) {

}

void Processor::sub(PWORD o1, PWORD o2) {

}

void Processor::cmp(PWORD o1, PWORD o2) {

}

void Processor::bis(PWORD o1, PWORD o2) {

}

void Processor::bic(PWORD o1, PWORD o2) {

}

void Processor::bit(PWORD o1, PWORD o2) {

}

void Processor::br(PWORD ost) {

}

void Processor::bne(PWORD ost) {

}

void Processor::beq(PWORD ost) {

}

void Processor::bpl(PWORD ost) {

}

void Processor::bmi(PWORD ost) {

}

void Processor::bvc(PWORD ost) {

}

void Processor::bvs(PWORD ost) {

}

void Processor::bhis(PWORD ost) {

}

void Processor::bcc(PWORD ost) {

}

void Processor::blo(PWORD ost) {

}

void Processor::bcs(PWORD ost) {

}

void Processor::bge(PWORD ost) {

}

void Processor::blt(PWORD ost) {

}

void Processor::bgt(PWORD ost) {

}

void Processor::ble(PWORD ost) {

}

void Processor::bhi(PWORD ost) {

}

void Processor::blos(PWORD ost) {

}

void Processor::jsr(PWORD ost) {

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

void Processor::spl(PWORD lvl) {

}

void Processor::clc() {

}

void Processor::clv() {

}

void Processor::clz() {

}

void Processor::cln() {

}

void Processor::sec() {

}

void Processor::sev() {

}

void Processor::sez() {

}

void Processor::sen() {

}

void Processor::ccc() {

}

void Processor::scc() {

}
