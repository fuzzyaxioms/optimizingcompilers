#include "temp.h"

bool
Temp
::clean_regs
()
{
	FOREACH(BlockIt, b, cfg.block_list) {
		// make a set of this block's instructions for fast lookup
		InstrSet instrs;
		FOREACH(InstrItIt, it, b->instrs) {
			instrs.insert(**it);
		}
		
		// find pseudo defs with single uses in the same block, and turn into temp regs
		FOREACH(InstrItIt, it, b->instrs) {
			
			// keep instrs be the set of instructions after the current instruction
			// to avoid backward reaching defs
			instrs.erase(**it);
			
			simple_reg *dst = get_dst(**it);
			if (dst != NO_REGISTER and dst->kind == PSEUDO_REG) {
				// check if it has a single use inside this basic block
				UseLinkList &uses = reach_defs.def_use_chain[**it];
				if (uses.size() == 1) {
					UseLink &use = uses.front();
					InstrSetIt lookup = instrs.find(use.usage);
					if (lookup != instrs.end()) {
						
						// have to make sure there is no call instruction between
						// because apparently simple suif also thinks call instructions
						// are breaks in basic blocks
						bool call_blocked = false;
						for (InstrIt j = *it; *j != use.usage; ++j) {
							if ((*j)->opcode == CALL_OP) {
								call_blocked = true;
								break;
							}
						}
						if (call_blocked) {
							// skip if there's a call in between
							continue;
						}
						
						//printf("before tempified:\n");
						//print_instrs(**it, 1);
						//print_instrs(use.usage, 1);
						
						// special case for a CPY instruction
						if ((**it)->opcode == CPY_OP) {
							// in this case, we just do copy propagation
							// which is fine even if the src is a temp reg
							// because there is only one use
							
							simple_reg* orig_reg = (**it)->u.base.src1;
							ASSERT(orig_reg != NO_REGISTER);
							
							// update the single usage
							set_src(use.usage, orig_reg, use.i);
							
							// remove the unused def
							cfg.instr_list.erase(*it);
							
							//printf("after tempified:\n");
							//print_instrs(use.usage, 1);
							//printf("\t%s\n", orig_reg->var);
						} else {
							// normal case
							
							// new temp register
							simple_reg *new_temp = new_register((**it)->type, TEMP_REG);
							
							// store to the new temp register
							set_dst(**it, new_temp);
							
							// update the single usage
							set_src(use.usage, new_temp, use.i);
							
							//printf("after tempified:\n");
							//print_instrs(**it, 1);
							//print_instrs(use.usage, 1);
							//printf("\t%s\n", new_temp->var);
						}
						
						return true;
					}
				}
			}
		}
	}
	
	return false;
}

Temp
::Temp
(CFG &cfg_, ReachDefs &reach_defs_)
: cfg(cfg_), reach_defs(reach_defs_)
{
}

bool
Temp
::run
()
{
	return clean_regs();
}