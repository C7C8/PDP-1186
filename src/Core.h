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
	virtual PBYTE read(PWORD loc) const;
	virtual bool write(PWORD loc, PBYTE* data, PWORD size) override;
	virtual bool write(PWORD loc, PBYTE data);
protected:
	PBYTE* mem;
	PWORD size;
};
