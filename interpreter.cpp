#include "interpreter.h"

AbstractValue
::AbstractValue
(int c1_)
: c1(c1_), c2(0), symbol(-1)
{
}

AbstractValue
::AbstractValue
()
: c1(0), c2(0), symbol(-1)
{
}

void av_print
(AbstractValue const& a)
{
	printf("\t%d+%d*(%d)\n", a.c1, a.c2, a.symbol);
}

bool
av_add
(AbstractValue const& a, AbstractValue const& b, AbstractValue& r)
{
	AbstractValue const* av1 = NULL;
	AbstractValue const* av2 = NULL;
	
	// swap around to get a constant in av1
	if (b.symbol < 0) {
		av1 = &b;
		av2 = &a;
	} else if (a.symbol < 0) {
		av1 = &a;
		av2 = &b;
	} else if (a.symbol != b.symbol) {
		// different symbols is unsupported
		return false;
	}
	
	// now we know either they both have the same symbol, or av1 is just a constant
	
	ASSERT(av1->symbol >= 0 or av1->c2 == 0);
	ASSERT(av2->symbol >= 0 or av2->c2 == 0);
	
	r.symbol = av2->symbol;
	r.c1 = av1->c1 + av2->c1;
	r.c2 = av1->c2 + av2->c2;
	return true;
}


bool
av_mul
(AbstractValue const& a, AbstractValue const& b, AbstractValue& r)
{
	AbstractValue const* av1 = NULL;
	AbstractValue const* av2 = NULL;
	
	// swap around to get a constant in av1
	if (b.symbol < 0) {
		av1 = &b;
		av2 = &a;
	} else if (a.symbol < 0) {
		av1 = &a;
		av2 = &b;
	} else {
		// one of them must be a constant
		return false;
	}
	
	// now we know av1 must be a constant
	ASSERT(av1->symbol < 0 and av1->c2 == 0);
	ASSERT(av2->symbol >= 0 or av2->c2 == 0);
	
	r.symbol = av2->symbol;
	r.c1 = av1->c1 * av2->c1;
	r.c2 = av1->c1 * av2->c2;
	return true;
}

bool
av_extract
(AbstractValue const& a, int& r)
{
	if (a.symbol < 0) {
		ASSERT(a.c2 == 0);
		r = a.c1;
		return true;
	}
	return false;
}

void
Interpreter
::find_labels
()
{
	label_map.clear();
	
	FOREACH(InstrIt, i, cfg.instr_list) {
		if ((*i)->opcode == LABEL_OP) {
			label_map[(*i)->u.label.lab] = i;
		}
	}
}

int
Interpreter
::get_symbol_index
(simple_reg *reg)
{
	SymbolIndexIt lookup = symbol_index.find(reg);
	if (lookup == symbol_index.end()) {
		int ix = symbol_index.size();
		symbol_index[reg] = ix;
		index_symbol.push_back(reg);
		return ix;
	} else {
		return lookup->second;
	}
}

bool
Interpreter
::interpret_instr
(InstrIt instr, InstrIt &next)
{
	// many instructions just fall through
	next = instr;
	++next;
	
	simple_instr *i = *instr;
	switch(i->opcode) {
		case NOP_OP: {
			return true;
		}
		case CPY_OP: {
			
			// make sure it's integral
			if (i->type != simple_type_signed) {
				// can't handle anything else
				return false;
			}
			
			simple_reg *src = i->u.base.src1;
			simple_reg *dst = i->u.base.dst;
			ASSERT(src != NO_REGISTER);
			ASSERT(src->kind == TEMP_REG or src->kind == PSEUDO_REG);
			if (src->kind == TEMP_REG) {
				// this is a temp reg, so it must've been defined and in the table
				RegTableIt lookup = reg_table.find(src);
				ASSERT(lookup != reg_table.end());
				
				// add or overwrite the value of the dst
				reg_table[dst] = lookup->second;
			} else {
				RegTableIt lookup = reg_table.find(src);
				
				if (lookup == reg_table.end()) {
					// create a new abstract value just for this symbol
					AbstractValue av(0);
					av.c2 = 1;
					av.symbol = get_symbol_index(src);;
					
					// add or overwrite the value of dst
					reg_table[dst] = av;
				} else {
					// add or overwrite the value of the dst
					reg_table[dst] = lookup->second;
				}
			}
			return true;
		}
		case LDC_OP: {
			// make sure it's integral
			if (i->type != simple_type_signed) {
				// can't handle anything else
				return false;
			}
			
			ASSERT(i->u.ldc.value.format == IMMED_INT);
			
			// add/overwrite the register's value
			AbstractValue av(i->u.ldc.value.u.ival);
			reg_table[i->u.ldc.dst] = av;
			
			return true;
		}
		case ADD_OP:
		case MUL_OP: {
			// make sure it's integral
			if (i->type != simple_type_signed) {
				// can't handle anything else
				return false;
			}
			
			simple_reg* src1 = i->u.base.src1;
			simple_reg* src2 = i->u.base.src2;
			simple_reg* dst = i->u.base.dst;
			
			ASSERT(src1 != NO_REGISTER);
			ASSERT(src2 != NO_REGISTER);
			ASSERT(dst != NO_REGISTER);
			
			AbstractValue av1(0);
			AbstractValue av2(0);
			
			RegTableIt lookup = reg_table.find(src1);
			if (lookup != reg_table.end()) {
				av1 = lookup->second;
			} else {
				// new symbol time
				ASSERT(src1->kind == PSEUDO_REG);
				av1.c2 = 1;
				av1.symbol = get_symbol_index(src1);
			}
			lookup = reg_table.find(src2);
			if (lookup != reg_table.end()) {
				av2 = lookup->second;
			} else {
				// new symbol time
				ASSERT(src2->kind == PSEUDO_REG);
				av2.c2 = 1;
				av2.symbol = get_symbol_index(src2);
			}
			
			AbstractValue result(0);
			bool possible = false;
			if (i->opcode == ADD_OP) {
				possible = av_add(av1, av2, result);
			} else {
				ASSERT(i->opcode == MUL_OP);
				possible = av_mul(av1, av2, result);
			}
			
			if (not possible) {
				// couldn't combine the two abstract values
				//printf("Couldn't combine for add or mul:\n");
				//av_print(av1);
				//av_print(av2);
				return false;
			}
			
			// otherwise we could
			reg_table[dst] = result;
			return true;
		}
		case SEQ_OP:
		case SNE_OP:
		case SL_OP:
		case SLE_OP: {
			simple_reg* src1 = i->u.base.src1;
			simple_reg* src2 = i->u.base.src2;
			simple_reg* dst = i->u.base.dst;
			
			ASSERT(src1 != NO_REGISTER);
			ASSERT(src2 != NO_REGISTER);
			ASSERT(dst != NO_REGISTER);
			
			int v1;
			int v2;
			
			RegTableIt lookup = reg_table.find(src1);
			if (lookup == reg_table.end()) {
				// can't evaluate at compile time
				return false;
			}
			if (not av_extract(lookup->second, v1)) {
				// also can't evaluate at compile time
				return false;
			}
			
			lookup = reg_table.find(src2);
			if (lookup == reg_table.end()) {
				// can't evaluate at compile time
				return false;
			}
			if (not av_extract(lookup->second, v2)) {
				// also can't evaluate at compile time
				return false;
			}
			
			// now do the comparison
			bool result;
			switch(i->opcode) {
				case SEQ_OP:
					result = v1 == v2;
					break;
				case SNE_OP:
					result = v1 != v2;
					break;
				case SL_OP:
					result = v1 < v2;
					break;
				case SLE_OP:
					result = v1 <= v2;
					break;
				default:
					return false;
			}
			
			reg_table[dst] = AbstractValue(result ? 1 : 0);
			
			return true;
		}
		case JMP_OP: {
			LabelMapIt lookup = label_map.find(i->u.bj.target);
			ASSERT(lookup != label_map.end());
			// just go there
			next = lookup->second;
			return true;
		}
		case BTRUE_OP:
		case BFALSE_OP: {
			simple_reg* src = i->u.bj.src;
			
			ASSERT(src != NO_REGISTER);
			
			int v;
			
			RegTableIt lookup = reg_table.find(src);
			if (lookup == reg_table.end()) {
				// can't evaluate at compile time
				return false;
			}
			if (not av_extract(lookup->second, v)) {
				// also can't evaluate at compile time
				return false;
			}
			
			ASSERT(v == 0 or v == 1);
			
			LabelMapIt lab_lookup = label_map.find(i->u.bj.target);
			ASSERT(lab_lookup != label_map.end());
			
			// branch if necessary
			if (i->opcode == BTRUE_OP and v == 1) {
				next = lab_lookup->second;
			} else if (i->opcode == BFALSE_OP and v == 0) {
				next = lab_lookup->second;
			}
			
			return true;
		}
		case LABEL_OP: {
			return true;
		}
		// handle RET_OP outside
		default: {
			// any other instructions are not supported
			return false;
		}
	}
	
	return false;
}

void
Interpreter
::clear_except
(simple_instr* i)
{
	for(InstrIt instr = cfg.instr_list.begin(); instr != cfg.instr_list.end();) {
		if (*instr != i) {
			instr = cfg.instr_list.erase(instr);
		} else {
			++instr;
		}
	}
}

void
Interpreter
::recompile
(InstrIt ret, AbstractValue const& a)
{
	ASSERT((*ret)->opcode == RET_OP);
	ASSERT((*ret)->u.base.src1 != NO_REGISTER);
	bool is_pseudo = (*ret)->u.base.src1->kind == PSEUDO_REG;
	
	simple_reg* c1_reg = new_register(simple_type_signed, TEMP_REG);
	simple_instr* c1_instr = new_instr(LDC_OP, simple_type_signed);
	c1_instr->u.ldc.dst = c1_reg;
	c1_instr->u.ldc.value.format = IMMED_INT;
	c1_instr->u.ldc.value.u.ival = a.c1;
	cfg.instr_list.insert(ret, c1_instr);
	
	simple_reg* c2_reg = new_register(simple_type_signed, TEMP_REG);
	simple_instr* c2_instr = new_instr(LDC_OP, simple_type_signed);
	c2_instr->u.ldc.dst = c2_reg;
	c2_instr->u.ldc.value.format = IMMED_INT;
	c2_instr->u.ldc.value.u.ival = a.c2;
	cfg.instr_list.insert(ret, c2_instr);
	
	simple_reg* mul_reg = new_register(simple_type_signed, TEMP_REG);
	simple_instr* mul_instr = new_instr(MUL_OP, simple_type_signed);
	mul_instr->u.base.dst = mul_reg;
	mul_instr->u.base.src1 = c2_reg;
	ASSERT(a.symbol >= 0 and a.symbol < index_symbol.size());
	mul_instr->u.base.src2 = index_symbol[a.symbol];
	cfg.instr_list.insert(ret, mul_instr);
	
	simple_reg* add_reg = NULL;
	if (is_pseudo) {
		add_reg = new_register(simple_type_signed, TEMP_REG);
	} else {
		add_reg = (*ret)->u.base.src1;
	}
	simple_instr* add_instr = new_instr(ADD_OP, simple_type_signed);
	add_instr->u.base.dst = add_reg;
	add_instr->u.base.src1 = c1_reg;
	add_instr->u.base.src2 = mul_reg;
	cfg.instr_list.insert(ret, add_instr);
	
	if (is_pseudo) {
		simple_instr* cpy_instr = new_instr(CPY_OP, simple_type_signed);
		cpy_instr->u.base.dst = (*ret)->u.base.src1;
		cpy_instr->u.base.src1 = add_reg;
		cpy_instr->u.base.src2 = NO_REGISTER;
		cfg.instr_list.insert(ret, cpy_instr);
	}
}

bool
Interpreter
::interpret
()
{
	reg_table.clear();
	symbol_index.clear();
	index_symbol.clear();
	
	find_labels();
	
	InstrIt ip = cfg.instr_list.begin();
	
	for(int i = 0; i < MAX_STEPS; ++i) {
		if ((*ip)->opcode == RET_OP) {
			//printf("return:\n");
			if ((*ip)->u.base.src1 != NO_REGISTER) {
				RegTableIt lookup = reg_table.find((*ip)->u.base.src1);
				if (lookup != reg_table.end()) {
					//av_print(lookup->second);
					//print_instrs(*ip, 1);
					
					// write new instructions to directly calculate return value
					clear_except(*ip);
					recompile(ip, lookup->second);
				} else {
					// this returns a function argument directly
					//print_instrs(*ip, 1);
					//clear_except(*ip);
				}
			} else {
				// doesn't return a value
				//print_instrs(*ip, 1);
				//clear_except(*ip);
			}
			return true;
		}
		InstrIt next;
		if (not interpret_instr(ip, next)) {
			
			//printf("failed at:\n");
			//print_instrs(*ip, 1);
			
			return false;
		}
		ip = next;
	}
	
	return false;
}

Interpreter
::Interpreter
(CFG &cfg_, ReachDefs &reach_defs_)
: cfg(cfg_), reach_defs(reach_defs_)
{
}

bool
Interpreter
::run
()
{
	return interpret();
}