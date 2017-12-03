#pragma once
#include <cstring>
#include "defs.h"
#include "MemoryDevice.h"

/**
 * Core memory.
 */
class Core : public MemoryDevice {
public:
	explicit Core(PWORD size);
	virtual ~Core();
	Core(const Core& core);
	void operator=(const Core& core);

	virtual const PBYTE* read(PWORD loc, PWORD size) const override;
	virtual bool write(PWORD loc, PBYTE* data, PWORD size) override;
protected:
	PBYTE* mem;
	PWORD size;
};
