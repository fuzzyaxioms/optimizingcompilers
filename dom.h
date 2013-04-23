// dominators
#ifndef DOM_H
#define DOM_H

#include <cstdio>
#include <vector>

#include "common.h"
#include "bitvecset.h"
#include "cfg.h"
#include "dfa.h"

#define NO_IDOM (-1)

// Can calculate either dominance or post dominance
// Requires blocks to be numbered.
class Dominance {
private:
	// keep a reference to the CFG
	CFG &cfg;
public:
	
	PropArray gen_sets;
	PropArray kill_sets;
	
	// Block number of the Immediate (post) dominator of each block.
	// Or the constant NO_IDOM
	std::vector<int> idoms;
	
	// Dataflow analysis
	DFA dfa;
	
	// Initialize the cfg reference and direction of DFA.
	Dominance(CFG &cfg_, DataDirection dir);
	
	// Compute dominators
	// Needs blocks to be numbered.
	void run();
	
	// Prints out variables, def and use sets.
	void print(char const *proc_name);
};

#endif
