#pragma once
#include "defs.h"

/**
 * Interface for a generic memory device. Since the PDP-11 supports DMA, this could technically be anything.
 */
class MemoryDevice {
public:
	MemoryDevice() {stat = true;}
	virtual const PBYTE* read(PWORD loc, PWORD size) const =0;
	virtual bool write(PWORD loc, PBYTE * data, PWORD size)=0;
	virtual bool status() {return stat;}
protected:
	bool stat;
};
