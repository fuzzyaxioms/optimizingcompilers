// constant propagation and folding
#ifndef DCE_H
#define DCE_H

#include <cstdio>
#include <vector>
#include <map>

#include "common.h"
#include "bitvecset.h"
#include "cfg.h"
#include "reachdefs.h"

// deadcode elimination
class DCE {
private:
	// keep references to useful stuff
	CFG &cfg;
	ReachDefs &reach_defs;
	
	// keep track of essential instructions
	InstrSet essential;
	
	// what's a basic essential instruction
	// i also consider control flow to be essential, as a simplification
	bool is_essential_base(simple_instr *i);
	
	// now iterate to find all essential instructions
	// assumes basic essentials have been found
	void mark_all_essential();
	
	// remove all instructions that are not essential
	bool clean_deadcode();
	
	
	
public:
	DCE(CFG &cfg_, ReachDefs &reach_defs_);
	
	// returns whether anything was changed
	bool run();
};

#endif /* DCE_H */
