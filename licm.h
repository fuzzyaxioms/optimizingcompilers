// loop invariant code motion
#ifndef LICM_H
#define LICM_H

#include <cstdio>
#include <vector>

#include "common.h"
#include "bitvecset.h"
#include "cfg.h"
#include "dfa.h"
#include "loop.h"
#include "reachdefs.h"

// This will only do one sweep and only mark the base invariant instructions
// so it won't mark invariant instructions that depend on other invariant
// instructions. Also only does one loop, because I don't handle nested loops.
// To get the full benefit, this needs to be run many times.
// Running this multiple times seems to be equivalent to running the full version
// which also checks for invariant instructions dependent on other invariant
// instructions.
// Brief justification: suppose the full version moves instruction s, then it
// must have also been able to move the ones s depends on, and so on and so forth
// until you get to base invariant instructions. This should be the same as moving
// out the base invariant instructions, then calculating new base invariant
// instructions, and moving those out, and so on and so forth.
// Not doing the full version saves some complexity at the cost of speed.
//
// Right now it's doing more harm than good, because of the temporary registers.
// It moves temporary registers outside for constants, which then get converted
// to pseudo registers, and are only used once still. So constant propagation
// is needed to fix this. Then deadcode elimination is necessary to remove the
// useless instructions outside now. Hopefully those two will be enough to get
// no slowdown at all when LICM doesn't help.
class LICM {
private:
	// keep references to useful stuff
	CFG &cfg;
	Dominance &doms;
	NaturalLoops &loops;
	ReachDefs &reach_defs;
	
	// find all instructions of a loop
	void find_instrs(HeadLoopIt h, InstrSet &instrs);
	
	// checks if basic invariant instruction i.e. constant or all defs outside
	// and also is suitable to be moved i.e. not a memory instruction or control flow
	bool is_invariant_base(InstrSet &instrs, simple_instr* i);
	
	// find all the basic blocks that can exit the loop
	// needs succs to be correct
	void find_exits(HeadLoopIt h, BlockItList &exits);
	
	// the first condition for being able to move out
	// has to dominate all exits of the loop
	bool doms_exits(BlockIt b, BlockItList &exits);
	
	// an alternative to the first condition that's slightly more lax
	// may move instructions outside loop if they're not used outside the loop
	// result in a little bit of extra computation but it's worth it if the
	// instruction executes more often than not
	bool leaks_out(InstrSet &instrs, simple_instr* i);
	
	// the second condition
	// has to be the only definition in the loop
	bool only_def(InstrSet &instrs, simple_instr* i);
	
	// the third condition
	// has to dominate all other uses in the loop
	bool doms_uses(InstrSet &instrs, simple_instr* i);
	
	// converts a temp register to a pseudo register, if applicable
	void to_pseudo(InstrIt instr);
	
public:
	LICM(CFG &cfg_, Dominance &doms_, NaturalLoops &loops_, ReachDefs &reach_defs_);
	
	// performs one sweep of loop invariant code motion for at most one loop
	// returns whether anything was changed
	bool run();
	
	//void print(char const *proc_name);
};

#endif /* LICM_H */
