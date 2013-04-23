// natural loop related functions and data structures
#ifndef LOOP_H
#define LOOP_H

#include <list>
#include <set>

#include "common.h"
#include "block.h"
#include "cfg.h"
#include "dom.h"

// To store possibly many natural loops all sharing the same head.
class HeadLoop {
public:
	// Head of the loop.
	BlockIt head;
	
	// the preheader
	BlockIt preheader;
	// whether the preheader falls through or jumps
	bool fall_thru;
	
	// All tails of these loops.
	BlockItList tails;
	
	// All blocks of all these loops.
	BlockItList blocks;
};

typedef std::list<HeadLoop> HeadLoopList;
typedef HeadLoopList::iterator HeadLoopIt;

// Helper for sorting a list of HeadLoop.
// Requires the basic blocks to be numbered.
bool loop_cmp(HeadLoop const &a, HeadLoop const &b);

// Keep track of all natural loops.
// Requires blocks to be numbered.
class NaturalLoops {
private:
	// reference to the CFG
	CFG &cfg;
	
	// reference to the dominators
	Dominance &doms;
	
	// find all backedges
	// requires blocks to be numbered
	void find_loops();
	
	// Helper recursive function for visiting upwards towards head of loop.
	void visit_upwards(BlockIt curr, BitVecSet &visited, BlockItList &blks);
	
	// Find all blocks belonging to the loops.
	// Need blocks to be numbered.
	void populate_loops();
	
	// while finding/making preheaders, because of complexities, I will only
	// allow one preheader to be newly created at a time. This way, when creating
	// a new preheader, the CFG and everything is in a consistent state, which
	// makes reasoning much easier, at the cost of performance.
	
	// given a loop, try to find a preheader
	// assume we've called populate_loops already
	// Returns whether we can find one in the CFG already. Sets the preheader
	// field in h if it does find one.
	bool find_preheader(HeadLoopIt h);
	
	// make a new preheader, updating jumps and fall throughs as necessary
	// adding the special unconditional jump if necessary as mentioned above
	// may need to add a label to the loop head
	// after this is called, we should stop and redo CFG and Dominance and redo
	// loops, because we are only making one new preheader at a time.
	void make_preheader(HeadLoopIt h);
	
public:
	// List of all natural loops.
	HeadLoopList loop_list;
	
	// Find all natural loops in the given CFG.
	NaturalLoops(CFG &cfg_, Dominance &doms_);
	
	// find all natural loops
	// try to find preheaders for every natural loop
	// returns whether it changed anything i.e. made a preheader
	bool run();
	
	// Print the loops.
	// Need CFG to be numbered.
	void print(char const *proc_name);
};

#endif
