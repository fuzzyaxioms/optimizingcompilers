#include "licm.h"
#include "bitvecset.h"

void
LICM
::find_instrs
(HeadLoopIt h, InstrSet &instrs)
{
	instrs.clear();
	
	for (BlockItIt b = h->blocks.begin(); b != h->blocks.end(); ++b) {
		for (InstrItIt i = (*b)->instrs.begin(); i != (*b)->instrs.end(); ++i) {
			instrs.insert(**i);
		}
	}
}

bool
LICM
::is_invariant_base
(InstrSet &instrs, simple_instr* i)
{
	//printf("is_invariant_base: in\n");
	
	simple_instr* cur_instr = i;
	switch(cur_instr->opcode) {
		case JMP_OP:
		case BTRUE_OP:
		case BFALSE_OP:
		case MBR_OP:
		case LABEL_OP:
		case CALL_OP:
		case RET_OP:
		case STR_OP:
		case LOAD_OP:
		case MCPY_OP:
			// these kinds of instructions cannot be moved, so skip them
			// we don't do aliasing analysis, so that's why we can't move
			// memory instructions
			//printf("is_invariant_base: out\n");
			return false;
	}
	// otherwise we check the operands
	bool good = true;
	DefLinkList &defs = reach_defs.use_def_chain[cur_instr];
	for (DefLinkIt d = defs.begin(); d != defs.end(); ++d) {
		if (instrs.count(d->def) != 0) {
			// there is a local def, so not invariant
			good = false;
		}
	}
	
	//printf("is_invariant_base: out\n");
	return good;
}

void
LICM
::find_exits
(HeadLoopIt h, BlockItList &exits)
{
	exits.clear();
	
	BitVecSet blks(cfg.block_list.size());
	blks.clear_all();
	
	// first make the set of loop blocks for fast testing
	for (BlockItIt b = h->blocks.begin(); b != h->blocks.end(); ++b) {
		blks.set((*b)->ix, true);
	}
	
	// now look for all the exit blocks
	for (BlockItIt b = h->blocks.begin(); b != h->blocks.end(); ++b) {
		BlockItList &succs = (*b)->succs;
		bool is_exit = false;
		for (BlockItIt s = succs.begin(); s != succs.end(); ++s) {
			if (not blks.get((*s)->ix)) {
				// find successors that lead outside the loop
				is_exit = true;
			}
		}
		if (is_exit) {
			exits.push_back(*b);
		}
	}
}

bool
LICM
::doms_exits
(BlockIt b, BlockItList &exits)
{
	for (BlockItIt e = exits.begin(); e != exits.end(); ++e) {
		
		if (not doms.dfa.out_sets[(*e)->ix].get(b->ix)) {
			// does not dominate one of the exits
			return false;
		}
	}
	return true;
}

bool
LICM
::leaks_out
(InstrSet &instrs, simple_instr* i)
{
	UseLinkList &uses = reach_defs.def_use_chain[i];
	FOREACH(UseLinkIt, u, uses) {
		InstrSetIt lookup = instrs.find(u->usage);
		if (lookup == instrs.end()) {
			// it leaks
			return true;
		}
	}
	return false;
}

bool
LICM
::only_def
(InstrSet &instrs, simple_instr* i)
{
	//printf("only_def: in\n");
	
	simple_reg *dst = get_dst(i);
	if (dst == NO_REGISTER) {
		// not a definition, so trivially satisfies this condition
		//printf("only_def: out\n");
		return true;
	}
	
	// check if there are any other defs
	for (InstrSetIt j = instrs.begin(); j != instrs.end(); ++j) {
		simple_instr *k = *j;
		if (k != i and dst == get_dst(k)) {
			// there are, so no go
			//printf("only_def: out\n");
			return false;
		}
	}
	// no other ones
	//printf("only_def: out\n");
	return true;
}

bool
LICM
::doms_uses
(InstrSet &instrs, simple_instr* i)
{
	//printf("doms_uses: in\n");
	
	simple_reg *dst = get_dst(i);
	if (dst == NO_REGISTER) {
		// not a definition, so trivially satisfies this condition
		//printf("doms_uses: out\n");
		return true;
	}
	
	// check all other uses to see if their only def that reaches is this one
	for (InstrSetIt j = instrs.begin(); j != instrs.end(); ++j) {
		simple_instr *k = *j;
		// check all reaching defs
		DefLinkList &defs = reach_defs.use_def_chain[k];
		for (DefLinkIt d = defs.begin(); d != defs.end(); ++d) {
			simple_instr* try_def = d->def;
			if (try_def != i and dst == get_dst(try_def)) {
				// another definition of the variable reaches, so not dominated
				//printf("doms_uses: out\n");
				return false;
			}
		}
	}
	
	// good
	//printf("doms_uses: out\n");
	return true;
}

void
LICM
::to_pseudo
(InstrIt instr)
{
	simple_instr *i = *instr;
	
	simple_reg* dst = get_dst(i);
	if (dst != NO_REGISTER and dst->kind == TEMP_REG) {
		// make a new pseudo register and copy the value into it
		simple_reg* pseudo = new_register(i->type, PSEUDO_REG);
		simple_instr* cpy = new_instr(CPY_OP, i->type);
		cpy->u.base.src1 = dst;
		cpy->u.base.src2 = NO_REGISTER;
		cpy->u.base.dst = pseudo;
		// insert the new instruction
		cfg.instr_list.insert(++instr, cpy);
		
		// update the use
		UseLinkList &use = reach_defs.def_use_chain[i];
		ASSERT(use.size() == 1);
		set_src(use.front().usage, pseudo, use.front().i);
	}
}

LICM
::LICM
(CFG &cfg_, Dominance &doms_, NaturalLoops &loops_, ReachDefs &reach_defs_)
: cfg(cfg_), doms(doms_), loops(loops_), reach_defs(reach_defs_)
{
}

bool
LICM
::run
()
{
	bool changed = false;
	
	for (HeadLoopIt h = loops.loop_list.begin(); h != loops.loop_list.end(); ++h) {
		
		//printf("\nProcessing loop %d\n", h->head->ix);
		
		InstrSet instrs;
		find_instrs(h, instrs);
		
		BlockItList exits;
		find_exits(h, exits);
		
		// now go through instructions block by block
		for (BlockItIt b = h->blocks.begin(); b != h->blocks.end(); ++b) {
			
			//printf("Processing block %d\n", (*b)->ix);
			
			BlockIt blk = *b;
			bool does_doms_exits = doms_exits(blk, exits);
			
			for (InstrItIt i = blk->instrs.begin(); i != blk->instrs.end(); ++i) {
				InstrIt cur_instr = *i;
				
				//print_instrs(*cur_instr, 1);
				
				bool toggle = false;
				
				if (is_invariant_base(instrs, *cur_instr)) {
					if (does_doms_exits or (not leaks_out(instrs, *cur_instr))) {
						if (only_def(instrs, *cur_instr)) {
							
							if (doms_uses(instrs, *cur_instr)) {
								// then move it
								simple_instr *raw_instr = *cur_instr;
								// remove from the instructions
								cfg.instr_list.erase(cur_instr);
								// add to preheader
								
								InstrIt last_instr = h->preheader->instrs.back();
								
								if (h->fall_thru) {
									// insert at the very end if it's just a fall through
									++last_instr;
								}
								InstrIt moved = cfg.instr_list.insert(last_instr, raw_instr);
								
								to_pseudo(moved);
								
								changed = true;
								
								//printf("moved:\n");
								//print_instrs(raw_instr, 1);
							}
						}
					}
				}
			}
		}
		
		if (changed) {
			// stop after moving code in one loop
			return true;
		}
	}
	return false;
}

#if 0
void
LICM
::print
(char const *proc_name)
{
	for (HeadLoopIt h = loops.loop_list.begin(); h != loops.loop_list.end(); ++h) {
		InstrSet instrs;
		InstrSet invars;
		find_instrs(h, instrs);
		mark_invariant_base(instrs, invars);
		
		printf("loop head %d\n", h->head->ix);
		for (InstrSetIt i = invars.begin(); i != invars.end(); ++i) {
			printf("\t");
			print_instrs(*i, 1);
		}
		
	}
}
#endif
