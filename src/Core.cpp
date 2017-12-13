#include "Core.h"

/**
 * Construct a core memory object of given size
 * @param size Size in bytes (NOT number of words)
 */
Core::Core(const PWORD size){
	mem = new PBYTE[size];
	memset(mem, 0, size);
	this->size = size;
}

/**
 * Free core memory
 */
Core::~Core(){
	delete mem;
}

/**
 * Copy constructor, will duplicate core
 * @param core Core to copy from.
 */
Core::Core(const Core& core){
	mem = new PBYTE[core.size];
	memcpy(mem, core.mem, core.size);
	this->size = core.size;
}

/**
 * Overload, will duplicate core
 * @param core Core to copy from
 */
void Core::operator=(const Core &core)	{
	mem = new PBYTE[core.size];
	memcpy(mem, core.mem, core.size);
	this->size = core.size;
}

/**
 * Read from a location in memory
 * @param loc Location to read from
 * @param size Amount of data to read
 * @return Pointer to data (don't modify it!). Will return nullptr in the event of an invalid read (out of bounds...)
 */
const PBYTE* Core::read(PWORD loc, PWORD size) const {
	if (loc >= this->size || size == 0 || loc + size >= this->size)
		return nullptr;
	return (const PBYTE*)&mem[loc];
}

PBYTE Core::read(PWORD loc) const {
	if (loc >= this->size || size == 0 || loc + size >= this->size)
		return nullptr;
	return (PBYTE)mem[loc];
}

/**
 * Write to a location in memory
 * @param loc Location to write to
 * @param data Data to write
 * @param size Amount of data to be written
 * @return Success status of write operation; will be false in event of invalid write (out of bounds...)
 */
bool Core::write(PWORD loc, PBYTE *data, PWORD size) {
	if (loc >= this->size || size == 0 || loc + size >= this->size)
		return false;
	memcpy(mem, data, size);
	return true;
}

/**
 * Write a single value to a location in memory
 * @param loc Location to read from
 * @param data Data to set
 * @return Whether the write succeeded
 */
bool Core::write(PWORD loc, PBYTE data){
	if (loc >= this->size || size == 0 || loc + size >= this->size)
		return false;
	mem[loc] = data;
	return true;
}
