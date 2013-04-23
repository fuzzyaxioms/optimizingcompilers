// constant propagation and folding
#ifndef CONSTANT_H
#define CONSTANT_H

#include <cstdio>
#include <vector>
#include <map>

#include "common.h"
#include "bitvecset.h"
#include "cfg.h"
#include "reachdefs.h"

// for storing which defs are constants
typedef std::map<simple_instr*,simple_immed*> ConstTable;
typedef ConstTable::iterator ConstTableIt;

// this just uses use def and def use chains to propagate constants and fold
// them
// this only propagates one iteration, because to propagate more, i need to
// keep the usedef and defuse chains updated, and it's a hassle, and so again
// i go with simplicity over efficiency
// After this runs, need to clean up the instructions:
// - may have dangling temp registers that are never used
// - may have unreachable code
// - may have nops
class ConstantOpts {
private:
	// keep references to useful stuff
	CFG &cfg;
	ReachDefs &reach_defs;
	
	// keep track of all constant defs
	ConstTable const_table;
	
	// populate the const table with constant base cases
	// a base case is either a temp reg in an ldc or a pseudo reg from a constant
	// temp reg
	void find_constant_base();
	
	// remove defs that appear more than once
	void filter_defs(DefLinkList &defs, DefLinkList &filtered);
	
	// get unique defs for the args of this instruction
	// return how many args this instruction has in total
	int get_unique_defs(simple_instr *i, DefLinkList &unique);
	
	// try to fold a bj form
	bool fold_bj(InstrIt instr, DefLinkList &defs);
	
	// try to fold base form
	bool fold_base(InstrIt instr, DefLinkList &defs);
	
	// try to propagate constants to the args
	// uses const_table
	// returns whether anything was changed
	bool prop_fold(InstrIt instr);
	
public:
	ConstantOpts(CFG &cfg_, ReachDefs &reach_defs_);
	
	// returns whether anything was changed
	bool run();
	
	//void print(char const *proc_name);
};

#endif /* CONSTANT_H */
