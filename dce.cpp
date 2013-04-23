#include <deque>

#include "dce.h"

bool
DCE
::is_essential_base
(simple_instr *i)
{
	switch(i->opcode) {
		case LOAD_OP:
		case STR_OP:
		case MCPY_OP:
		case JMP_OP:
		case BTRUE_OP:
		case BFALSE_OP:
		case MBR_OP:
		case LABEL_OP:
		case CALL_OP:
		case RET_OP:
			return true;
		default:
			return false;
	}
}

void
DCE::
mark_all_essential
()
{
	essential.clear();
	
	typedef std::deque<simple_instr*> WorkList;
	typedef WorkList::iterator WorkListIt;
	
	// add all base essential instructions to the work list
	WorkList work_list;
	
	FOREACH(InstrIt, i, cfg.instr_list) {
		if (is_essential_base(*i)) {
			work_list.push_back(*i);
		}
	}
	
	// start iterating to find all essential instructions
	// use use-def chain to follow dependency
	while (not work_list.empty()) {
		simple_instr *i = work_list.front();
		work_list.pop_front();
		
		if (essential.count(i) > 0) {
			// already found this to be essential, so skip
			continue;
		}
		
		// mark as essential
		essential.insert(i);
		
		// propagate to all reaching defs
		DefLinkList &defs = reach_defs.use_def_chain[i];
		FOREACH(DefLinkIt, d, defs) {
			work_list.push_back(d->def);
		}
	}
}

bool
DCE
::clean_deadcode
()
{
	bool changed = false;
	
	for (InstrIt i = cfg.instr_list.begin(); i != cfg.instr_list.end();) {
		if (essential.count(*i) <= 0) {
			// remove it
			
			//printf("removed:\n");
			//print_instrs(*i, 1);
			
			i = cfg.instr_list.erase(i);
			changed = true;
		} else {
			// keep going
			++i;
		}
	}
	return changed;
}

DCE
::DCE
(CFG &cfg_, ReachDefs &reach_defs_)
: cfg(cfg_), reach_defs(reach_defs_)
{
}

bool
DCE
::run
()
{
	mark_all_essential();
	return clean_deadcode();
}