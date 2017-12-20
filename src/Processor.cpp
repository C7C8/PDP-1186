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

