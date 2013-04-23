#include "cse.h"

void
print_arg
(ArgType const& type, ConstOrVar const& src)
{
	switch(type) {
		case REG_ARG: {
			fprint_reg(stdout, src.reg);
			break;
		}
		case CONST_ARG: {
			fprint_immed(stdout, &src.immed);
			break;
		}
		default: {
			ASSERT(false);
			break;
		}
	}
}

// helper function for comparison
template<typename T>
int
simple_cmp
(T const& a, T const& b)
{
	if (a < b) {
		return -1;
	} else if (a > b) {
		return 1;
	} else {
		return 0;
	}
}

int
ExpKeyLT
::cmp_arg
(ArgType const& type1, ConstOrVar const& src1, ArgType const& type2, ConstOrVar const& src2)
const
{
	if (type1 != type2) {
		// constant is smaller than reg
		// use the enum as the ordering
		return type1 < type2 ? -1 : 1;
	}
	// otherwise both are the same type
	if (type1 == REG_ARG) {
		return simple_cmp(src1.reg, src2.reg);
	} else {
		// need to compare the actual constants now
		// int < float < symbol, at least for how it's defined in simple.h
		if (src1.immed.format != src2.immed.format) {
			// use the enum as the ordering
			return simple_cmp(src1.immed.format, src2.immed.format);
		}
		// otherwise both are same type
		switch(src1.immed.format) {
			case IMMED_INT: {
				return simple_cmp(src1.immed.u.ival, src2.immed.u.ival);
			}
			case IMMED_FLOAT: {
				return simple_cmp(src1.immed.u.fval, src2.immed.u.fval);
			}
			case IMMED_SYMBOL: {
				if (src1.immed.u.s.symbol != src2.immed.u.s.symbol) {
					return simple_cmp(src1.immed.u.s.symbol, src2.immed.u.s.symbol);
				} else {
					// compare offsets
					return simple_cmp(src1.immed.u.s.offset, src2.immed.u.s.offset);
				}
			}
			default: {
				ASSERT(false);
				break;
			}
		}
	}
}

bool
ExpKeyLT
::operator()
(ExpKey const& exp1, ExpKey const& exp2)
const
{
	if (exp1.opcode != exp2.opcode) {
		// use the opcode enum for ordering
		return exp1.opcode < exp2.opcode;
	}
	
	if (exp1.type != exp2.type) {
		// use pointer address for ordering
		return exp1.type < exp2.type;
	}
	
	int cmp_arg1 = cmp_arg(exp1.type1, exp1.src1, exp2.type1, exp2.src1);
	if (cmp_arg1 != 0) {
		return cmp_arg1 < 0;
	}
	int cmp_arg2 = cmp_arg(exp1.type2, exp1.src2, exp2.type2, exp2.src2);
	return cmp_arg2 < 0;
}

bool
immed_equal
(simple_immed *a, simple_immed *b)
{
	// needs to be the same kind of constant
	if (a->format != b->format) {
		return false;
	}
	
	switch(a->format) {
		case IMMED_INT:
			return a->u.ival == b->u.ival;
		case IMMED_FLOAT:
			return a->u.fval == b->u.fval;
		case IMMED_SYMBOL:
			return a->u.s.symbol == b->u.s.symbol and a->u.s.offset == b->u.s.offset;
		default:
			// weird stuffs
			ASSERT(false);
			break;
	}
	// shouldn't get here
	return false;
}

void
CSE
::make_temp_table
()
{
	temp_table.clear();
	
	// find all ldc temp regs
	FOREACH(InstrIt, i, cfg.instr_list) {
		if ((*i)->opcode == LDC_OP) {
			if ((*i)->u.ldc.dst != NO_REGISTER) {
				temp_table[(*i)->u.ldc.dst] = &(*i)->u.ldc.value;
			}
			
			// debug info
			//printf("temp const:\n");
			//print_instrs(*i, 1);
		}
	}
}

bool
CSE
::reg_equal
(simple_reg *a, simple_reg *b)
{
	if (a == NO_REGISTER or b == NO_REGISTER) {
		// true if both are NO_REGISTER
		return a == b;
	}
	// now know neither of them is NO_REGISTER
	if (a->kind != b->kind) {
		// must be same kind
		return false;
	}
	if (a->kind == PSEUDO_REG) {
		// pseudo registers must be exactly the same
		return a == b;
	} else {
		ASSERT(a->kind == TEMP_REG);
		// check if they are both constants
		TempTableIt lookup = temp_table.find(a);
		if (lookup == temp_table.end()) {
			// not constant, so they must be exactly the same
			return a == b;
		}
		simple_immed *aval = lookup->second;
		lookup = temp_table.find(b);
		if (lookup == temp_table.end()) {
			// definitely not the same
			return false;
		}
		simple_immed *bval = lookup->second;
		return immed_equal(aval, bval);
	}
}

bool
CSE
::want_exp
(simple_instr *i, ExpKey &key)
{
	key.opcode = i->opcode;
	key.type = i->type;
	switch(i->opcode) {
		case ADD_OP:
		case SUB_OP:
		case MUL_OP:
		case DIV_OP:
		case REM_OP:
		case MOD_OP:
		case AND_OP:
		case IOR_OP:
		case XOR_OP:
		case SEQ_OP:
		case SNE_OP:
		case SL_OP:
		case SLE_OP:
			// we want
			break;
		default:
			// we don't want
			return false;
	}
	ASSERT(simple_op_format(i->opcode) == BASE_FORM);
	RegList args;
	get_srcs(i, args);
	int ix = -1;
	int n = 0;
	FOREACH(RegIt, r, args) {
		++ix;
		
		ArgType *type = NULL;
		ConstOrVar *src = NULL;
		if (ix == 0) {
			type = &key.type1;
			src = &key.src1;
		} else {
			ASSERT(ix == 1);
			type = &key.type2;
			src = &key.src2;
		}
		
		if (*r == NO_REGISTER) {
			// this is fine
			*type = REG_ARG;
			src->reg = NO_REGISTER;
			++n;
			continue;
		}
		if ((*r)->kind == PSEUDO_REG) {
			// this is good
			*type = REG_ARG;
			src->reg = *r;
			++n;
			continue;
		}
		if ((*r)->kind == TEMP_REG) {
			// this better be a constant
			TempTableIt lookup = temp_table.find(*r);
			if (lookup != temp_table.end()) {
				// is a constant
				*type = CONST_ARG;
				src->immed = *lookup->second;
				++n;
				continue;
			} else {
				// badd
				return false;
			}
		}
		// not good
		return false;
	}
	// all args were good
	ASSERT(ix == 1);
	ASSERT(n == 2);
	return true;
}

void
CSE
::find_exps
()
{
	exp_table.clear();
	exp_array.clear();
	
	exp_def_table.clear();
	exp_def_array.clear();
	
	
	ExpKey key;
	FOREACH(InstrIt, i, cfg.instr_list) {
		if (want_exp(*i, key)) {
			// add this to the defs of expressions
			int def_ix = exp_def_table.size();
			exp_def_table[*i] = def_ix;
			exp_def_array.push_back(i);
			
			// add this to the expressions
			int exp_ix = exp_table.size();
			ExpTableIt lookup = exp_table.find(key);
			if (lookup == exp_table.end()) {
				// add if not already in it
				exp_table[key] = exp_ix;
				exp_array.push_back(key);
				
				//printf("exp_table add:");
				//print_instrs(*i, 1);
			}
		}
	}
}

void
CSE
::ae_step
(simple_instr *i, BitVecSet &gen_set, BitVecSet &kill_set)
{
	ExpKey key;
	// only defs of pseudo variables can kill expressions
	simple_reg *dst = get_dst(i);
	if (dst != NO_REGISTER and dst->kind == PSEUDO_REG) {
		// find out which ones this kills
		for (int j = 0; j < exp_table.size(); ++j) {
			key = exp_array[j];
			bool killed = false;
			// check if either arg is the same
			killed |= (key.type1 == REG_ARG and key.src1.reg == dst);
			killed |= (key.type2 == REG_ARG and key.src2.reg == dst);
			if (killed) {
				gen_set.set(j, false);
				kill_set.set(j, true);
			}
		}
	}
	// find out which one this gens
	if (want_exp(i, key)) {
		// this should be generating something
		ExpKeyLT key_cmp;
		ExpTableIt lookup = exp_table.find(key);
		
		//if (lookup == exp_table.end()) {
			//print_instrs(i, 1);
			//printf("exp\n");
			//printf("\t%s", simple_op_name(key.opcode));
			//printf("\t");
			//print_arg(key.type1, key.src1);
			//printf(", ");
			//print_arg(key.type2, key.src2);
			//printf("\n");
			//FOREACH(ExpTableIt, it, exp_table) {
				//ExpKey const& key2 = it->first;
				//printf("\t%s", simple_op_name(key.opcode));
				//printf("\t");
				//print_arg(key2.type1, key2.src1);
				//printf(", ");
				//print_arg(key2.type2, key2.src2);
				//printf("\n");
			//}
		//}
		
		ASSERT(lookup != exp_table.end());
		gen_set.set(lookup->second, true);
	}
}

void
CSE
::prepare_ae
()
{
	ae_gen_sets.clear();
	ae_kill_sets.clear();
	
	FOREACH(BlockIt, b, cfg.block_list) {
		BitVecSet gen_set(exp_table.size());
		BitVecSet kill_set(exp_table.size());
		
		gen_set.clear_all();
		kill_set.clear_all();
		
		FOREACH(InstrItIt, it, b->instrs) {
			ae_step(**it, gen_set, kill_set);
		}
		
		ae_gen_sets.push_back(gen_set);
		ae_kill_sets.push_back(kill_set);
	}
}

void
CSE
::re_step
(simple_instr *i, BitVecSet &gen_set, BitVecSet &kill_set)
{
	ExpKeyLT key_cmp;
	ExpKey key;
	bool want = want_exp(i, key);
	
	// defs of pseudo variables can kill expressions
	// evaluation of the same expression can also kill it
	simple_reg *dst = get_dst(i);
	
	// find out which ones this kills
	for (int j = 0; j < exp_def_table.size(); ++j) {
		simple_instr *exp_def = *exp_def_array[j];
		ASSERT(simple_op_format(exp_def->opcode) == BASE_FORM);
		if (dst != NO_REGISTER and dst->kind == PSEUDO_REG) {
			// if either argument gets redefined, then it's no good
			if (dst == exp_def->u.base.src1 or dst == exp_def->u.base.src2) {
				// overwritten
				gen_set.set(j, false);
				kill_set.set(j, true);
			}
		}
		if (want) {
			ExpKey def_key;
			ASSERT(want_exp(exp_def, def_key));
			if (not key_cmp(key, def_key) and not key_cmp(def_key, key)) {
				// if same evaluation, kills them as well
				gen_set.set(j, false);
				kill_set.set(j, true);
			}
		}
	}
	
	// find out which one this gens
	ExpDefTableIt lookup = exp_def_table.find(i);
	if (lookup != exp_def_table.end()) {
		gen_set.set(lookup->second, true);
	}
}

void
CSE
::prepare_re
()
{
	re_gen_sets.clear();
	re_kill_sets.clear();
	
	FOREACH(BlockIt, b, cfg.block_list) {
		BitVecSet gen_set(exp_def_table.size());
		BitVecSet kill_set(exp_def_table.size());
		
		gen_set.clear_all();
		kill_set.clear_all();
		
		FOREACH(InstrItIt, it, b->instrs) {
			re_step(**it, gen_set, kill_set);
		}
		
		re_gen_sets.push_back(gen_set);
		re_kill_sets.push_back(kill_set);
	}
}

bool
CSE
::eliminate_exp
(InstrIt instr, BitVecSet &ae_gen_set, BitVecSet &re_gen_set)
{
	ExpKey key;
	if (want_exp(*instr, key)) {
		if (exp_set.count(key) > 0) {
			// already eliminated this expression from elsewhere
			return false;
		}
		// check if we can eliminate this
		ExpTableIt lookup = exp_table.find(key);
		ASSERT(lookup != exp_table.end());
		int exp_ix = lookup->second;
		if (not ae_gen_set.get(exp_ix)) {
			// not available
			return false;
		}
		// now it is available, and not already done somewhere else
		
		// keep track of which defs reach
		InstrItList reach_evals;
		
		ExpKeyLT key_cmp;
		// for every evaluation of this expression that reaches
		for (int j = 0; j < exp_def_array.size(); ++j) {
			if (re_gen_set.get(j)) {
				ExpKey def_key;
				InstrIt def_instr = exp_def_array[j];
				ASSERT(want_exp(*def_instr, def_key));
				if (*instr == *def_instr) {
					// if you reach yourself, then we can't eliminate anything
					return false;
				} else if (not key_cmp(key, def_key) and not key_cmp(def_key, key)) {
					// same expression
					reach_evals.push_back(def_instr);
				}
			}
		}
		
		// finally we can eliminate an expression
		// have to rewrite every definition that reaches here with a new variable
		// so we can eliminate this evaluation
		
		//printf("to eliminate:\n");
		//print_instrs(*instr, 1);
		
		// make a new variable
		simple_reg *factor = new_register(key.type, PSEUDO_REG);
		
		FOREACH(InstrItIt, it, reach_evals) {
			//printf("to factor out:\n");
			//print_instrs(**it, 1);
			
			//{
				//ExpKey key1;
				//ExpKey key2;
				//ASSERT(*instr != **it);
				//ASSERT(want_exp(*instr, key1));
				//ASSERT(want_exp(**it, key2));
				//ASSERT(not key_cmp(key1, key2));
				//ASSERT(not key_cmp(key2, key1));
				//printf("exp\n");
				//printf("\t%s", simple_op_name(key.opcode));
				//printf("\t");
				//print_arg(key1.type1, key1.src1);
				//printf(", ");
				//print_arg(key1.type2, key1.src2);
				//printf("\n");
				//printf("\t%s", simple_op_name(key.opcode));
				//printf("\t");
				//print_arg(key2.type1, key2.src1);
				//printf(", ");
				//print_arg(key2.type2, key2.src2);
				//printf("\n");
			//}
			
			// change the destination to the new variable
			InstrIt def_instr = *it;
			simple_reg *orig_reg = get_dst(*def_instr);
			ASSERT(orig_reg != NO_REGISTER);
			ASSERT(simple_op_format((*def_instr)->opcode) == BASE_FORM);
			(*def_instr)->u.base.dst = factor;
			// insert a copy instruction afterwards to the original variable
			simple_instr *cpy = new_instr(CPY_OP, (*def_instr)->type);
			cpy->u.base.dst = orig_reg;
			cpy->u.base.src1 = factor;
			cpy->u.base.src2 = NO_REGISTER;
			cfg.instr_list.insert(++def_instr, cpy);
			
			//printf("factored out:\n");
			//print_instrs(cpy, 1);
		}
		
		// change current instruction to a copy
		ASSERT(simple_op_format((*instr)->opcode) == BASE_FORM);
		(*instr)->opcode = CPY_OP;
		(*instr)->u.base.src1 = factor;
		(*instr)->u.base.src2 = NO_REGISTER;
		
		//printf("eliminated:\n");
		//print_instrs(*instr, 1);
		
		// indicate this expression has been eliminated already
		exp_set.insert(key);
		
		return true;
	}
	return false;
}

bool
CSE
::eliminate_exps
()
{
	exp_set.clear();
	
	FOREACH(BlockIt, b, cfg.block_list) {
		BitVecSet ae_gen_set = ae_dfa.in_sets[b->ix];
		BitVecSet ae_kill_set(exp_table.size()); /* unused */
		
		BitVecSet re_gen_set = re_dfa.in_sets[b->ix];
		BitVecSet re_kill_set(exp_def_table.size()); /* unused */
		
		FOREACH(InstrItIt, it, b->instrs) {
			if (eliminate_exp(*it, ae_gen_set, re_gen_set)) {
				return true;
			}
			ae_step(**it, ae_gen_set, ae_kill_set);
			re_step(**it, re_gen_set, re_kill_set);
		}
	}
	
	return false;
}

CSE
::CSE
(CFG &cfg_)
: cfg(cfg_), ae_dfa(ALL_PATH, FORWARDS, cfg_), re_dfa(ANY_PATH, FORWARDS, cfg_)
{
}

bool
CSE
::run()
{
	make_temp_table();
	
	find_exps();
	
	if (exp_table.size() == 0) {
		// nothing to do
		return false;
	}
	
	prepare_ae();
	ae_dfa.solve(exp_table.size(), ae_gen_sets, ae_kill_sets);
	
	ASSERT(exp_def_table.size() != 0);
	prepare_re();
	re_dfa.solve(exp_def_table.size(), re_gen_sets, re_kill_sets);
	
	return eliminate_exps();
}

void
CSE
::print
(char const *proc_name)
{
	printf("cse %s\n", proc_name);
	
	printf("const temps\n");
	FOREACH(TempTableIt, it, temp_table) {
		printf("\t");
		fprint_reg(stdout, it->first);
		printf("\n");
	}
	
	// print expression defs
	printf("exp defs\n");
	FOREACH(ExpDefArrayIt, it, exp_def_array) {
		print_instrs(**it, 1);
	}
	
	// print expressions
	printf("exp\n");
	FOREACH(ExpArrayIt, it, exp_array) {
		ExpKey const& key = *it;
		printf("\t%s", simple_op_name(key.opcode));
		printf("\t");
		print_arg(key.type1, key.src1);
		printf(", ");
		print_arg(key.type2, key.src2);
		printf("\n");
	}
	
	// now print out the def/kill/out sets
	printf("ae DFA\n");
	for (int i = 0; i < ae_gen_sets.size(); ++i) {
		printf("block %d\n", i);
		printf("\tgen  ");
		ae_gen_sets[i].print(stdout);
		printf("\n");
		printf("\tkill ");
		ae_kill_sets[i].print(stdout);
		printf("\n");
		printf("\tout  ");
		ae_dfa.out_sets[i].print(stdout);
		printf("\n");
	}
	
	// now print out the def/kill/out sets
	printf("re DFA\n");
	for (int i = 0; i < re_gen_sets.size(); ++i) {
		printf("block %d\n", i);
		printf("\tgen  ");
		re_gen_sets[i].print(stdout);
		printf("\n");
		printf("\tkill ");
		re_kill_sets[i].print(stdout);
		printf("\n");
		printf("\tout  ");
		re_dfa.out_sets[i].print(stdout);
		printf("\n");
	}
}