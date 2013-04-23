#include <deque>
#include <map>

#include "constant.h"

void
ConstantOpts
::find_constant_base
()
{
	const_table.clear();
	
	// first find all ldc temp regs
	FOREACH(InstrIt, i, cfg.instr_list) {
		if ((*i)->opcode == LDC_OP) {
			const_table[*i] = &(*i)->u.ldc.value;
			
			// debug info
			//printf("temp const:\n");
			//print_instrs(*i, 1);
		}
	}
	
	// now find all cpy instructions of constant temp regs
	FOREACH(InstrIt, i, cfg.instr_list) {
		if ((*i)->opcode == CPY_OP && (*i)->u.base.src1->kind == TEMP_REG) {
			DefLinkList &defs = reach_defs.use_def_chain[*i];
			// since this is using a temp reg, there should only be one def
			ASSERT(defs.size() == 1);
			
			// see if it's constant
			ConstTableIt lookup = const_table.find(defs.front().def);
			if (lookup != const_table.end()) {
				// add this to the const table as well
				const_table[*i] = lookup->second;
				
				// debug info
				//printf("reg const:\n");
				//print_instrs(lookup->first, 1);
				//print_instrs(*i, 1);
			}
		}
	}
}

void
ConstantOpts
::filter_defs
(DefLinkList &defs, DefLinkList &filtered)
{
	typedef std::set<int> IntSet;
	typedef IntSet::iterator IntSetIt;
	IntSet seen;
	IntSet dups;
	
	// find duplicates
	FOREACH(DefLinkIt, d, defs) {
		IntSetIt lookup = seen.find(d->i);
		if (lookup == seen.end()) {
			// haven't seen before
			seen.insert(d->i);
		} else {
			// yes duplicate
			dups.insert(d->i);
		}
	}
	
	filtered.clear();
	// only put in uniques
	FOREACH(DefLinkIt, d, defs) {
		if (dups.find(d->i) == dups.end()) {
			filtered.push_back(*d);
		}
	}
}

int
ConstantOpts
::get_unique_defs
(simple_instr *i, DefLinkList &unique)
{
	RegList args;
	get_srcs(i, args);
	// count how many args there are
	int nargs = 0;
	FOREACH(RegIt, r, args) {
		if (*r != NO_REGISTER) {
			++nargs;
		}
	}
	
	// get the unique defs
	filter_defs(reach_defs.use_def_chain[i], unique);
	
	return nargs;
}

bool
ConstantOpts
::fold_bj
(InstrIt instr, DefLinkList &defs)
{
	ASSERT(defs.size() <= 1);
	simple_immed* val = NULL;
	if (defs.size() == 1) {
		DefLinkIt d = defs.begin();
		ASSERT(d->i == 0);
		ASSERT(d->def->opcode == LDC_OP);
		val = &d->def->u.ldc.value;
		ASSERT(val->format == IMMED_INT);
	}
	
	bool folded = false;
	int which = -1;
	
	// TODO
	switch((*instr)->opcode) {
		case BTRUE_OP:
			//printf("MISSING FOLD BTRUE_OP\n");
			folded = true;
			which = 1;
			break;
		case BFALSE_OP:
			//printf("MISSING FOLD BFALSE_OP\n");
			folded = true;
			which = 0;
			break;
	}
	
	if (folded) {
		ASSERT(which == 0 or which == 1);
		if (val->u.ival == which or (which == 1 and val->u.ival != 0)) {
			// then this will take the branch, so make it a jump
			simple_instr *jmp = new_instr(JMP_OP, simple_type_void);
			jmp->u.bj.src = NO_REGISTER;
			jmp->u.bj.target = (*instr)->u.bj.target;
			*instr = jmp;
		} else {
			// then this will fall through, so make it a NOP
			// removing it here will cause problems, so i don't do it
			simple_instr *nop = new_instr(NOP_OP, simple_type_void);
			nop->u.base.dst = NO_REGISTER;
			nop->u.base.src1 = NO_REGISTER;
			nop->u.base.src2 = NO_REGISTER;
			*instr = nop;
		}
		//printf("folded:\n");
		//print_instrs(*instr, 1);
	}
	
	return folded;
}

bool
ConstantOpts
::fold_base
(InstrIt instr, DefLinkList &defs)
{
	// get the zero or one or two values
	simple_immed* src1 = NULL;
	simple_immed* src2 = NULL;
	simple_immed result;
	
	FOREACH(DefLinkIt, d, defs) {
		ASSERT(d->i == 0 or d->i == 1);
		ASSERT(d->def->opcode == LDC_OP);
		if (d->i == 0) {
			src1 = &d->def->u.ldc.value;
		} else {
			src2 = &d->def->u.ldc.value;
		}
	}
	
	bool folded = false;
	
	switch((*instr)->opcode) {
		case SLE_OP:
			folded = true;
			ASSERT(src1->format == src2->format);
			result.format = IMMED_INT;
			if (src1->format == IMMED_INT) {
				result.u.ival = src1->u.ival <= src2->u.ival;
			} else if (src1->format == IMMED_FLOAT) {
				result.u.ival = src1->u.fval <= src2->u.fval;
			}
			break;
		case SL_OP:
			folded = true;
			ASSERT(src1->format == src2->format);
			result.format = IMMED_INT;
			if (src1->format == IMMED_INT) {
				result.u.ival = src1->u.ival < src2->u.ival;
			} else if (src1->format == IMMED_FLOAT) {
				result.u.ival = src1->u.fval < src2->u.fval;
			}
			break;
		case SNE_OP:
			folded = true;
			ASSERT(src1->format == src2->format);
			result.format = IMMED_INT;
			if (src1->format == IMMED_INT) {
				result.u.ival = src1->u.ival != src2->u.ival;
			} else if (src1->format == IMMED_FLOAT) {
				result.u.ival = src1->u.fval != src2->u.fval;
			}
			break;
		case SEQ_OP:
			folded = true;
			ASSERT(src1->format == src2->format);
			result.format = IMMED_INT;
			if (src1->format == IMMED_INT) {
				result.u.ival = src1->u.ival == src2->u.ival;
			} else if (src1->format == IMMED_FLOAT) {
				result.u.ival = src1->u.fval == src2->u.fval;
			}
			break;
		case ROT_OP:
			// TODO
			//printf("MISSING FOLD ROT_OP\n");
			//folded = true;
			break;
		case LSL_OP:
			// TODO
			//printf("MISSING FOLD LSL_OP\n");
			//folded = true;
			break;
		case LSR_OP:
			// TODO
			//printf("MISSING FOLD LSR_OP\n");
			//folded = true;
			break;
		case ASR_OP:
			// TODO
			//printf("MISSING FOLD ASR_OP\n");
			//folded = true;
			break;
		case XOR_OP:
			folded = true;
			ASSERT(src1->format == src2->format);
			ASSERT(src1->format == IMMED_INT);
			result.format = IMMED_INT;
			result.u.ival = src1->u.ival ^ src2->u.ival;
			break;
		case IOR_OP:
			folded = true;
			ASSERT(src1->format == src2->format);
			ASSERT(src1->format == IMMED_INT);
			result.format = IMMED_INT;
			result.u.ival = src1->u.ival | src2->u.ival;
			break;
		case AND_OP:
			folded = true;
			ASSERT(src1->format == src2->format);
			ASSERT(src1->format == IMMED_INT);
			result.format = IMMED_INT;
			result.u.ival = src1->u.ival & src2->u.ival;
			break;
		case NOT_OP:
			folded = true;
			ASSERT(src1->format == IMMED_INT);
			result.format = IMMED_INT;
			result.u.ival = ~src1->u.ival;
			break;
		case MOD_OP:
			// TODO
			//printf("MISSING FOLD MOD_OP\n");
			//folded = true;
			break;
		case REM_OP:
			// TODO
			//printf("MISSING FOLD REM_OP\n");
			//folded = true;
			break;
		case DIV_OP:
			folded = true;
			ASSERT(src1->format == src2->format);
			result.format = src1->format;
			if (src1->format == IMMED_INT) {
				result.u.ival = src1->u.ival / src2->u.ival;
			} else if (src1->format == IMMED_FLOAT) {
				result.u.fval = src1->u.fval / src2->u.fval;
			}
			break;
		case MUL_OP:
			folded = true;
			ASSERT(src1->format == src2->format);
			result.format = src1->format;
			if (src1->format == IMMED_INT) {
				result.u.ival = src1->u.ival * src2->u.ival;
			} else if (src1->format == IMMED_FLOAT) {
				result.u.fval = src1->u.fval * src2->u.fval;
			}
			break;
		case SUB_OP:
			folded = true;
			if (src1->format == IMMED_FLOAT) {
				result.format = IMMED_FLOAT;
				result.u.fval = src1->u.fval - src2->u.fval;
			} else if (src1->format == IMMED_SYMBOL) {
				if (src2->format == IMMED_SYMBOL) {
					folded = false;
					// can't handle this because we don't know addresses
				} else {
					result.format = IMMED_SYMBOL;
					result.u.s.symbol = src1->u.s.symbol;
					result.u.s.offset = src1->u.s.offset - src2->u.ival;
				}
			} else {
				ASSERT(src1->format == src2->format);
				result.format = IMMED_INT;
				result.u.ival = src1->u.ival - src2->u.ival;
			}
			break;
		case ADD_OP:
			folded = true;
			if (src1->format == IMMED_FLOAT) {
				result.format = IMMED_FLOAT;
				result.u.fval = src1->u.fval + src2->u.fval;
			} else if (src1->format == IMMED_SYMBOL) {
				result.format = IMMED_SYMBOL;
				result.u.s.symbol = src1->u.s.symbol;
				result.u.s.offset = src1->u.s.offset + src2->u.ival;
				
				// this seems to mess up for some reason
				// probably because the offset is in bits and this addition isn't
				folded = false;
			} else if (src2->format == IMMED_SYMBOL) {
				result.format = IMMED_SYMBOL;
				result.u.s.symbol = src2->u.s.symbol;
				result.u.s.offset = src2->u.s.offset + src1->u.ival;
				
				// this seems to mess up for some reason, so don't do it
				// probably because the offset is in bits and this addition isn't
				folded = false;
			} else {
				ASSERT(src1->format == src2->format);
				result.format = IMMED_INT;
				result.u.ival = src1->u.ival + src2->u.ival;
			}
			break;
		case NEG_OP:
			folded = true;
			result.format = src1->format;
			if (src1->format == IMMED_INT) {
				result.u.ival = -src1->u.ival;
			} else if (src1->format == IMMED_FLOAT) {
				result.u.fval = -src1->u.fval;
			}
			break;
		case CVT_OP:
			// TODO
			//printf("MISSING FOLD CVT_OP\n");
			 //folded = true;
			break;
		default:
			// can't fold anything else
			// CPY_OP are folded by constant propagation
			break;
	}
	
	simple_reg *dst = get_dst(*instr);
	
	if (folded and dst != NO_REGISTER) {
		// change the instructions now
		if (dst->kind == TEMP_REG) {
			// simple, just change to a ldc instruction
			*instr = new_instr(LDC_OP, (*instr)->type);
			(*instr)->u.ldc.dst = dst;
			(*instr)->u.ldc.value = result;
			
			//printf("folded:\n");
			//print_instrs(*instr, 1);
			
		} else if (dst->kind == PSEUDO_REG) {
			// need to make a new temporary
			// make a new temp reg and ldc instruction
			simple_reg *new_temp = new_register((*instr)->type, TEMP_REG);
			simple_instr *new_ldc = new_instr(LDC_OP, (*instr)->type);
			new_ldc->u.ldc.dst = new_temp;
			// copy the constant value over
			new_ldc->u.ldc.value = result;
			
			// insert the new instruction right before
			cfg.instr_list.insert(instr, new_ldc);
			
			// turn current instruction into a cpy instruction
			*instr = new_instr(CPY_OP, (*instr)->type);
			(*instr)->u.base.src1 = new_temp;
			(*instr)->u.base.dst = dst;
			
			//printf("folded:\n");
			//print_instrs(new_ldc, 1);
			//print_instrs(*instr, 1);
		}
	}
	
	return folded;
}

bool
ConstantOpts
::prop_fold
(InstrIt instr)
{
	if (const_table.find(*instr) != const_table.end()) {
		// ignore if it's a constant base case
		return false;
	}
	
	bool changed = false;
	
	// get the arguments
	DefLinkList unique;
	int nargs = get_unique_defs(*instr, unique);
	
	// try to propagate if possible
	int nconst = 0;
	FOREACH(DefLinkIt, d, unique) {
		ConstTableIt lookup = const_table.find(d->def);
		if (lookup != const_table.end()) {
			simple_reg *dst = get_dst(d->def);
			if (dst->kind == PSEUDO_REG) {
				// we can only propagate if the constant comes from a pseudo reg
				// no point if the constant comes from a temp reg
				
				// make a new temp reg and ldc instruction
				simple_reg *new_temp = new_register(d->def->type, TEMP_REG);
				simple_instr *new_ldc = new_instr(LDC_OP, d->def->type);
				new_ldc->u.ldc.dst = new_temp;
				// copy the constant value over
				new_ldc->u.ldc.value = *lookup->second;
				
				// insert the new instruction right before
				cfg.instr_list.insert(instr, new_ldc);
				
				// update to use the new reg
				set_src(*instr, new_temp, d->i);
				
				// update the def link as well for use in folding later
				d->def = new_ldc;
				
				changed = true;
				
				// one more arg is now constant
				
				// debug info
				//printf("propagated:\n");
				//print_instrs(new_ldc, 1);
				//print_instrs(*instr, 1);
			}
			
			++nconst;
		}
	}
	//printf("nargs %d nconst %d\n", nargs, nconst);
	
	// try to fold
	if (nargs == nconst) {
		// all arguments must be constants in order to fold
		switch((*instr)->opcode) {
			case JMP_OP:
				// already fully folded
				break;
			case BTRUE_OP:
			case BFALSE_OP:
				// BJ_FORM
				changed |= fold_bj(instr, unique);
				break;
			case MBR_OP:
				// MBR_FORM
				// TODO fold this
				break;
			case LABEL_OP:
				// LABEL_FORM
				// can't fold
				break;
			case LDC_OP:
				// LDC_FORM
				// can't fold
				break;
			case CALL_OP:
				// CALL_FORM
				// can't fold
				break;
			default:
				// BASE_FORM
				changed |= fold_base(instr, unique);
				break;
		}
	}
	
	return changed;
}

ConstantOpts
::ConstantOpts
(CFG &cfg_, ReachDefs &reach_defs_)
: cfg(cfg_), reach_defs(reach_defs_)
{
}

bool
ConstantOpts
::run
()
{
	bool changed = false;
	
	find_constant_base();
	
	FOREACH(InstrIt, i, cfg.instr_list) {
		changed |= prop_fold(i);
	}
	return changed;
}