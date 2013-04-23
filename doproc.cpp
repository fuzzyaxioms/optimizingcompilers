#include <cstdio>
#include <cstdlib>
#include "common.h"
#include "instr.h"
#include "block.h"
#include "cfg.h"

#include "dfa.h"

#include "dom.h"
#include "reachdefs.h"
#include "loop.h"
#include "licm.h"
#include "constant.h"
#include "dce.h"
#include "cse.h"
#include "copyprop.h"
#include "temp.h"
#include "interpreter.h"

#define CONSTANT_OPTS_ENV "ECE540_DISABLE_CONSTANT_OPTS"
#define DCE_ENV "ECE540_DISABLE_DCE"
#define TEMP_ENV "ECE540_DISABLE_TEMP"
#define LICM_ENV "ECE540_DISABLE_LICM"
#define COPY_PROP_ENV "ECE540_DISABLE_COPY_PROP"
#define CSE_ENV "ECE540_DISABLE_CSE"
#define INTERPRETER_ENV "ECE540_DISABLE_INTERPRETER"

// checks if the given optimization has been disabled
bool opt_disabled(char const* env) {
	return std::getenv(env) != NULL;
}

// do constant propagation and constant folding
// returns whether anything was changed
bool do_constant_opts(CFG &cfg, ReachDefs &reach_defs, ConstantOpts &constant_opts) {
	
	if (opt_disabled(CONSTANT_OPTS_ENV)) {
		return false;
	}
	
	LOG("==== Constant Opts ====\n");
	
	int cutoff = 100;
	bool changed = false;
	
	for(int i = 0; i < cutoff; ++i) {
		
		LOG("-- Iteration %d --\n", i+1);
		
		cfg.run();
		reach_defs.run();
		if (constant_opts.run()) {
			changed = true;
		} else {
			// nothing else changes, so stop the loop
			break;
		}
	}
	
	return changed;
}

// deadcode elimination
// returns whether anything was changed
bool do_dce(CFG &cfg, ReachDefs &reach_defs, DCE &dce) {
	
	if (opt_disabled(DCE_ENV)) {
		return false;
	}
	
	LOG("==== Deadcode Elimination ====\n");
	
	int cutoff = 100;
	bool changed = false;
	
	for(int i = 0; i < cutoff; ++i) {
		
		LOG("-- Iteration %d --\n", i+1);
		
		cfg.run();
		reach_defs.run();
		if (dce.run()) {
			changed = true;
		} else {
			// nothing else changes, so stop the loop
			break;
		}
	}
	
	return changed;
}

// deadcode elimination
// returns whether anything was changed
bool do_temp(CFG &cfg, ReachDefs &reach_defs, Temp &temp) {
	
	if (opt_disabled(TEMP_ENV)) {
		return false;
	}
	
	LOG("==== Register Conversion ====\n");
	
	int cutoff = 100;
	bool changed = false;
	
	for(int i = 0; i < cutoff; ++i) {
		
		LOG("-- Iteration %d --\n", i+1);
		
		cfg.run();
		reach_defs.run();
		if (temp.run()) {
			changed = true;
		} else {
			// nothing else changes, so stop the loop
			break;
		}
	}
	
	return changed;
}

// loop invariant code motion
// returns whether anything was changed
bool do_licm(CFG &cfg, Dominance &doms, ReachDefs &reach_defs, NaturalLoops &loops, LICM &licm) {
	
	if (opt_disabled(LICM_ENV)) {
		return false;
	}
	
	LOG("==== Loop Invariant Code Motion ====\n");
	
	int cutoff = 100;
	bool changed = false;
	
	for(int i = 0; i < cutoff; ++i) {
		
		LOG("-- Iteration %d --\n", i+1);
		
		cfg.run();
		doms.run();
		reach_defs.run();
		
		// make sure loops doesn't change anymore before doing licm
		if (not loops.run()) {
			if (licm.run()) {
				changed = true;
			} else {
				break;
			}
		} else {
			changed = true;
		}
	}
	
	return changed;
}

// copy propagation
// returns whether anything was changed
bool do_copy_prop(CFG &cfg, ReachDefs &reach_defs, CopyProp &copy_prop) {
	
	if (opt_disabled(COPY_PROP_ENV)) {
		return false;
	}
	
	LOG("==== Copy Propagation ====\n");
	
	int cutoff = 100;
	bool changed = false;
	
	for(int i = 0; i < cutoff; ++i) {
		
		LOG("-- Iteration %d --\n", i+1);
		
		cfg.run();
		reach_defs.run();
		
		if (copy_prop.run()) {
			changed = true;
		} else {
			// nothing else changes, so stop the loop
			break;
		}
	}
	
	return changed;
}

// common subexpression elimination
// returns whether anything was changed
bool do_cse(CFG &cfg, CSE &cse) {
	
	if (opt_disabled(CSE_ENV)) {
		return false;
	}
	
	LOG("==== CSE ====\n");
	
	int cutoff = 100;
	bool changed = false;
	
	for(int i = 0; i < cutoff; ++i) {
		
		LOG("-- Iteration %d --\n", i+1);
		
		cfg.run();
		
		if (cse.run()) {
			changed = true;
		} else {
			// nothing else changes, so stop the loop
			break;
		}
	}
	
	return changed;
}

// compile time interpretation
bool do_interpreter(CFG &cfg, ReachDefs &reach_defs, Interpreter &interpreter) {
	
	if (opt_disabled(INTERPRETER_ENV)) {
		return false;
	}
	
	LOG("==== Compile Time Interpretation ====\n");
	
	// only do this once
	int cutoff = 1;
	bool changed = false;
	
	for(int i = 0; i < cutoff; ++i) {
		
		LOG("-- Iteration %d --\n", i+1);
		
		cfg.run();
		reach_defs.run();
		if (interpreter.run()) {
			changed = true;
		} else {
			// nothing else changes, so stop the loop
			break;
		}
	}
	
	return changed;
}

simple_instr* do_procedure (simple_instr *inlist, char *proc_name)
{
	// an empty procedure should still have a single ret instruction
	ASSERT(inlist != NULL);
	
	// Transform to a STL list of instructions
	InstrList instr_list;
	to_instr_list(inlist, instr_list);
	
	// Initialize some structures
	CFG cfg(instr_list);
	
	// Dominators
	Dominance doms(cfg, FORWARDS);
	
	// Post dominators
	//Dominance pdoms(cfg, BACKWARDS);
	
	// reaching definitions
	ReachDefs reach_defs(cfg);
	
	// find natural loops
	NaturalLoops loops(cfg, doms);
	
	// loop invariant code motion
	LICM licm(cfg, doms, loops, reach_defs);
	
	// constant propagation and folding
	ConstantOpts constant_opts(cfg, reach_defs);
	
	// deadcode elimination
	DCE dce(cfg, reach_defs);
	
	// converting pseudo regs to temp regs when possible
	Temp temp(cfg, reach_defs);
	
	// copy propagation
	CopyProp copy_prop(cfg, reach_defs);
	
	// common subexpression elimination
	CSE cse(cfg);
	
	Interpreter interpreter(cfg, reach_defs);
	
	// do optimizations in a loop
	printf("==== Optimizing ====\n");
	int cutoff = 10;
	for (int i = 0; i < cutoff; ++i) {
		
		printf("\nOuter Iteration %d\n\n", i);
		
		bool changed = false;
		changed |= do_licm(cfg, doms, reach_defs, loops, licm);
		changed |= do_constant_opts(cfg, reach_defs, constant_opts);
		changed |= do_cse(cfg, cse);
		changed |= do_copy_prop(cfg, reach_defs, copy_prop);
		changed |= do_temp(cfg, reach_defs, temp);
		changed |= do_dce(cfg, reach_defs, dce);
		changed |= do_interpreter(cfg, reach_defs, interpreter);
		
		if (not changed) {
			// reached a standstill
			break;
		}
	}
	
	//cfg.print(proc_name);
	//doms.print(proc_name);
	//pdoms.print(proc_name);
	//reach_defs.print(proc_name);
	//loops.print(proc_name);
	//cse.print(proc_name);
	
	// recreate the instruction list
	inlist = from_instr_list(instr_list);
	
	//printf("Optimized instructions: %s\n", proc_name);
	//print_instrs(inlist);
	
	return inlist;
}
