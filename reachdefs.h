// Reaching definitions analysis
// Used to build def use def chains later
// Only concern ourselves with pseudo defs
// Assume temp regs in addition to having a single def and use, the def
// must dominate the use.

#ifndef REACHDEFS_H
#define REACHDEFS_H

#include <map>
#include <vector>

#include "common.h"
#include "bitvecset.h"
#include "cfg.h"
#include "dfa.h"

// represents a definition being used somewhere
// for the def->use chain
struct UseLink {
	// which instruction is the usage
	simple_instr *usage;
	// which argument this is
	// 0 or 1 for src1 or src2
	// or the index in a call following the order of get_srcs
	int i;
	
	UseLink(simple_instr *usage_, int i_): usage(usage_), i(i_) {}
};

// represents a definition reaching a usage
// for the use->def chain
struct DefLink {
	// the definition
	simple_instr *def;
	// which argument this definition is reaching
	int i;
	
	DefLink(simple_instr *def_, int i_): def(def_), i(i_) {}
};

typedef std::list<UseLink> UseLinkList;
typedef UseLinkList::iterator UseLinkIt;

typedef std::list<DefLink> DefLinkList;
typedef DefLinkList::iterator DefLinkIt;

// the two chains
typedef std::map<simple_instr*, UseLinkList> DefUseChain;
typedef DefUseChain::iterator DefUseChainIt;

typedef std::map<simple_instr*, DefLinkList> UseDefChain;
typedef UseDefChain::iterator UseDefChainIt;




// Maps definition instruction to index
typedef std::map<simple_instr*, int> DefTable;
typedef DefTable::iterator DefTableIt;

// The inverse of DefTable
typedef std::vector<simple_instr*> DefArray;
typedef DefArray::iterator DefArrayIt;

class ReachDefs {
private:
	// keeps track of all definitions
	DefTable def_table;
	DefArray def_array;
	
	// Fake initial definitions for all pseudo registers.
	// This takes care of using variables before being defined.
	InstrList initial_defs;
	
	void make_fake_defs(InstrList &instr_list);
	
	// Assume fake defs have been made
	void find_all_defs(InstrList &instr_list);
	
	// Create def and kill set for a basic block.
	// Needs blocks to be numbered.
	// Needs num_instr to be correct.
	void block_set(BlockIt b, BitVecSet &def_set, BitVecSet &kill_set);
	
	// assume reaching definitions has been solved, so now make the chains
	void make_chains();
public:
	
	// the two chains
	DefUseChain def_use_chain;
	UseDefChain use_def_chain;
	
	// def/Gen set per basic block.
	PropArray def_sets;
	
	// Kill/def set per basic block.
	PropArray kill_sets;
	
	// Dataflow analysis
	DFA dfa;
	
	// reference to the CFG
	CFG &cfg;
	
	// Initialize cfg
	ReachDefs(CFG &cfg_);
	
	// Find all definitions and do the DFA.
	void run();
	
	// Prints out definitions and stuff.
	void print(char const *proc_name);
};


#endif /* REACHDEFS_H */
