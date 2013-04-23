#include "copyprop.h"

void
CopyProp
::find_all
()
{
	copy_table.clear();
	copy_array.clear();
	
	FOREACH(InstrIt, i, cfg.instr_list) {
		if ((*i)->opcode == CPY_OP) {
			ASSERT((*i)->u.base.src1 != NO_REGISTER);
			ASSERT((*i)->u.base.src2 == NO_REGISTER);
			ASSERT((*i)->u.base.dst != NO_REGISTER);
			if ((*i)->u.base.src1->kind == PSEUDO_REG) {
				
				bool same_type = true;
				
				// also need both sides to have the same type to avoid casting errors
				simple_type *type = (*i)->type;
				DefLinkList &defs = reach_defs.use_def_chain[*i];
				FOREACH(DefLinkIt, d, defs) {
					if (d->def->type != type) {
						same_type = false;
					}
				}
				
				if (same_type) {
					int ix = copy_table.size();
					copy_table[*i] = ix;
					copy_array.push_back(*i);
					ASSERT(copy_array.size() == ix + 1);
				}
			}
		}
	}
}

void
CopyProp
::instr_step
(InstrIt i, BitVecSet &gen_set, BitVecSet &kill_set)
{
	// check if this a def of a pseudo reg
	// if this is a def of a temp reg, it cannot kill any of our copy instrs
	// because it can't be the src
	// and if it was the dst, well temp regs can only have one def
	simple_reg *dst = get_dst(*i);
	if (dst != NO_REGISTER && dst->kind == PSEUDO_REG) {
		// kills all defs of this reg
		for (int j = 0; j < copy_array.size(); ++j) {
			ASSERT(copy_array[j]->opcode == CPY_OP);
			ASSERT(copy_array[j]->u.base.src1 != NO_REGISTER);
			ASSERT(copy_array[j]->u.base.src1->kind == PSEUDO_REG);
			if (copy_array[j]->u.base.src1 == dst or copy_array[j]->u.base.dst == dst) {
				// if the dst is redefined or src is redefined
				kill_set.set(j, true);
				gen_set.set(j, false);
			}
		}
	}
	CopyTableIt lookup = copy_table.find(*i);
	if (lookup != copy_table.end()) {
		// new def
		ASSERT(lookup->second >= 0);
		gen_set.set(lookup->second, true);
	}
}

void
CopyProp
::setup_block
(BlockIt b, BitVecSet &gen_set, BitVecSet &kill_set)
{
	kill_set.clear_all();
	gen_set.clear_all();
	
	// go through instructions from begin to end
	FOREACH(InstrItIt, it, b->instrs) {
		instr_step(*it, gen_set, kill_set);
	}
}

bool
CopyProp
::try_prop
(InstrIt i, BitVecSet &gen_set)
{
	RegList args;
	get_srcs(*i, args);
	
	bool changed = false;
	
	int ix = 0;
	FOREACH(RegIt, r, args) {
		for (int j = 0; j < copy_table.size(); ++j) {
			if (gen_set.get(j)) {
				simple_instr *instr = copy_array[j];
				ASSERT(instr->u.base.dst != NO_REGISTER);
				if (*r == instr->u.base.dst) {
					// then this must be the only def that reaches by property
					// of copy expressions
					// propagate the copy
					set_src(*i, instr->u.base.src1, ix);
					changed = true;
				}
			}
		}
		++ix;
	}
	
	if (changed) {
		//printf("copyprop:\n");
		//print_instrs(*i, 1);
	}
	
	return changed;
}

CopyProp
::CopyProp
(CFG &cfg_, ReachDefs &reach_defs_)
: cfg(cfg_), reach_defs(reach_defs_), dfa(ALL_PATH, FORWARDS, cfg_)
{
}

bool
CopyProp
::run
()
{
	find_all();
	
	if (copy_table.size() == 0) {
		// nothing to do at all
		return false;
	}
	
	// start from scratch
	gen_sets.clear();
	kill_sets.clear();
	
	// Compute def and use sets for each basic block.
	FOREACH(BlockIt, b, cfg.block_list) {
		BitVecSet gen_set(copy_table.size());
		BitVecSet kill_set(copy_table.size());
		
		setup_block(b, gen_set, kill_set);
		
		gen_sets.push_back(gen_set);
		kill_sets.push_back(kill_set);
	}
	
	// Compute in and out sets using DFA.
	dfa.solve(copy_table.size(), gen_sets, kill_sets);
	
	// now try to propagate
	
	FOREACH(BlockIt, b, cfg.block_list) {
		BitVecSet gen_set = dfa.in_sets[b->ix];
		BitVecSet kill_set(copy_table.size()); /* unused */
		
		FOREACH(InstrItIt, it, b->instrs) {
			if (try_prop(*it, gen_set)) {
				return true;
			}
			instr_step(*it, gen_set, kill_set);
		}
	}
	
	return false;
}

void
CopyProp
::print
(char const *proc_name)
{
	printf("copy expressions %s %u\n", proc_name, copy_table.size());
	
	// first print out all the defs
	for (int i = 0; i < copy_table.size(); ++i) {
		print_instrs(copy_array[i], 1);
	}
	
	// now print out the def/kill/out sets
	for (int i = 0; i < gen_sets.size(); ++i) {
		printf("block %d\n", i);
		printf("\tgen  ");
		gen_sets[i].print(stdout);
		printf("\n");
		printf("\tkill ");
		kill_sets[i].print(stdout);
		printf("\n");
		printf("\tout  ");
		dfa.out_sets[i].print(stdout);
		printf("\n");
	}
}
