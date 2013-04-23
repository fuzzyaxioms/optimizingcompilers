// constant propagation and folding
#ifndef TEMP_H
#define TEMP_H

#include <cstdio>
#include <vector>
#include <map>

#include "common.h"
#include "bitvecset.h"
#include "cfg.h"
#include "reachdefs.h"

// convert pseudo reg to temp reg when possible
class Temp {
private:
	// keep references to useful stuff
	CFG &cfg;
	ReachDefs &reach_defs;
	
	// converts pseudo regs to temp reg when possible
	// returns whether anything changed
	bool clean_regs();
	
public:
	Temp(CFG &cfg_, ReachDefs &reach_defs_);
	
	// returns whether anything was changed
	bool run();
};

#endif /* TEMP_H */
