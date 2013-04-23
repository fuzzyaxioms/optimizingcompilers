// copy propagation
// like constant opts, this needs to be run repeatedly to get the full benefit
// of copy propagation
// This is necessary because my other optimizations create these kinds of copies.
#ifndef COPYPROP_H
#define COPYPROP_H

#include <map>
#include <vector>

#include "common.h"
#include "bitvecset.h"
#include "cfg.h"
#include "dfa.h"
#include "reachdefs.h"

// keep track of copies
// only care about copies from a pseudo register
// copies from a temp register cannot propagated, so we ignore those

// to keep track of the copy instructions we care about

// map copy instructions to their index in the bit vector
typedef std::map<simple_instr*, int> CopyTable;
typedef CopyTable::iterator CopyTableIt;

// and the inverse
typedef std::vector<simple_instr*> CopyArray;
typedef CopyArray::iterator CopyArrayIt;

class CopyProp {
private:
	// useful references
	CFG &cfg;
	ReachDefs &reach_defs;
	
	// the copy instructions
	CopyTable copy_table;
	CopyArray copy_array;
	
	// find all appropriate copy instructions
	void find_all();
	
	// updates the given gen and kill sets for a single instruction
	void instr_step(InstrIt i, BitVecSet &gen_set, BitVecSet &kill_set);
	
	// make gen and kill set for a single block
	void setup_block(BlockIt b, BitVecSet &gen_set, BitVecSet &kill_set);
	
	// try to propagate copies for an instruction
	bool try_prop(InstrIt i, BitVecSet &gen_set);
public:
	
	PropArray gen_sets;
	PropArray kill_sets;
	
	// Dataflow analysis
	DFA dfa;
	
	// Initialize
	CopyProp(CFG &cfg_, ReachDefs &reach_defs_);
	
	// returns whether anything changed i.e. any copies propagated
	bool run();
	
	// Prints out definitions and stuff.
	void print(char const *proc_name);
};


#endif /* COPYPROP_H */
