#include "instr.h"

void
to_instr_list
(simple_instr *inlist, InstrList &instr_list)
{
	while (inlist != NULL) {
		instr_list.push_back(inlist);
		inlist = inlist->next;
	}
}

simple_instr*
from_instr_list
(InstrList &instr_list)
{
	InstrIt begin = instr_list.begin();
	InstrIt end = instr_list.end();
	
	// Empty list.
	if (begin == end) {
		return NULL;
	}
	
	// Non empty list.
	simple_instr *first = *begin;
	first->prev = NULL;
	
	// Keep sliding window of two iterators.
	simple_instr *prev = first;
	while ((++begin) != end) {
		simple_instr *curr = *begin;
		
		// Make the forward and backward links
		prev->next = curr;
		curr->prev = prev;
		
		prev = curr;
	}
	
	// End the list.
	prev->next = NULL;
	
	// DON"T FORGET TO RETURN A VALUE!!!!!!!
	// This took me like 20 minutes to figure out.
	return first;
}

bool
is_leader
(simple_instr *prev, simple_instr *curr)
{
	// If current instruction is the very first one in the procedure.
	if (prev == NULL) {
		return true;
	}
	
	// Check if current instruction signifies a leader.
	switch(curr->opcode) {
		case LABEL_OP:
			return true;
	}
	
	// Check if previous instruction signifies a leader.
	switch(prev->opcode) {
		case JMP_OP:
		case BTRUE_OP:
		case BFALSE_OP:
		case MBR_OP:
		case RET_OP:
			return true;
	}
	
	// Otherwise just a regular non-disruptive instruction.
	// CALLs are treated as regular non-disruptive instructions.
	return false;
}

simple_reg*
get_dst
(simple_instr *instr)
{
	switch(instr->opcode) {
		case JMP_OP:
		case BTRUE_OP:
		case BFALSE_OP:
			// BJ_FORM
		case MBR_OP:
			// MBR_FORM
		case LABEL_OP:
			// LABEL_FORM
			return NO_REGISTER;
		case LDC_OP:
			// LDC_FORM
			return instr->u.ldc.dst;
		case CALL_OP:
			// CALL_FORM
			return instr->u.call.dst;
		default:
			// BASE_FORM
			return instr->u.base.dst;
	}
}

void
set_dst
(simple_instr *instr, simple_reg *dst)
{
	switch(instr->opcode) {
		case JMP_OP:
		case BTRUE_OP:
		case BFALSE_OP:
			// BJ_FORM
		case MBR_OP:
			// MBR_FORM
		case LABEL_OP:
			// LABEL_FORM
			break;
		case LDC_OP:
			// LDC_FORM
			instr->u.ldc.dst = dst;
			break;
		case CALL_OP:
			// CALL_FORM
			instr->u.call.dst = dst;
			break;
		default:
			// BASE_FORM
			instr->u.base.dst = dst;
			break;
	}
}

void
get_srcs
(simple_instr *instr, RegList &reg_list)
{
	switch(instr->opcode) {
		case JMP_OP:
		case BTRUE_OP:
		case BFALSE_OP:
			// BJ_FORM
			reg_list.push_back(instr->u.bj.src);
			break;
		case MBR_OP:
			// MBR_FORM
			reg_list.push_back(instr->u.mbr.src);
			break;
		case LABEL_OP:
			// LABEL_FORM
			break;
		case LDC_OP:
			// LDC_FORM
			break;
		case CALL_OP:
			// CALL_FORM
			reg_list.push_back(instr->u.call.proc);
			for (int i = 0; i < instr->u.call.nargs; ++i) {
				reg_list.push_back(instr->u.call.args[i]);
			}
			break;
		default:
			// BASE_FORM
			reg_list.push_back(instr->u.base.src1);
			reg_list.push_back(instr->u.base.src2);
			break;
	}
}


simple_reg *
get_src
(simple_instr *instr, int i)
{
	switch(instr->opcode) {
		case JMP_OP:
		case BTRUE_OP:
		case BFALSE_OP:
			// BJ_FORM
			ASSERT(i == 0);
			return instr->u.bj.src;
			break;
		case MBR_OP:
			// MBR_FORM
			ASSERT(i == 0);
			return instr->u.mbr.src;
			break;
		case LABEL_OP:
			// LABEL_FORM
			ASSERT(false);
			break;
		case LDC_OP:
			// LDC_FORM
			ASSERT(false);
			break;
		case CALL_OP:
			// CALL_FORM
			ASSERT(i >= 0 and i <= instr->u.call.nargs);
			if (i == 0) {
				return instr->u.call.proc;
			} else {
				return instr->u.call.args[i-1];
			}
			break;
		default:
			// BASE_FORM
			ASSERT(i == 0 or i == 1);
			if (i == 0) {
				return instr->u.base.src1;
			} else {
				return instr->u.base.src2;
			}
			break;
	}
}

void
set_src
(simple_instr *instr, simple_reg *arg, int i)
{
	switch(instr->opcode) {
		case JMP_OP:
		case BTRUE_OP:
		case BFALSE_OP:
			// BJ_FORM
			ASSERT(i == 0);
			instr->u.bj.src = arg;
			break;
		case MBR_OP:
			// MBR_FORM
			ASSERT(i == 0);
			instr->u.mbr.src = arg;
			break;
		case LABEL_OP:
			// LABEL_FORM
			ASSERT(false);
			break;
		case LDC_OP:
			// LDC_FORM
			ASSERT(false);
			break;
		case CALL_OP:
			// CALL_FORM
			ASSERT(i >= 0 and i <= instr->u.call.nargs);
			if (i == 0) {
				instr->u.call.proc = arg;
			} else {
				instr->u.call.args[i-1] = arg;
			}
			break;
		default:
			// BASE_FORM
			ASSERT(i == 0 or i == 1);
			if (i == 0) {
				instr->u.base.src1 = arg;
			} else {
				instr->u.base.src2 = arg;
			}
			break;
	}
}

void
print_instrs
(InstrIt begin, InstrIt end)
{
	for (; begin != end; ++begin) {
		fprint_instr(stdout, *begin);
	}
}

void
print_instrs
(InstrIt begin, ulong count)
{
	for (; count > 0; --count) {
		fprint_instr(stdout, *begin);
		++begin;
	}
}

void
print_instrs
(simple_instr *begin, ulong count)
{
	for (; count > 0; --count) {
		ASSERT(begin != NULL);
		fprint_instr(stdout, begin);
		begin = begin->next;
	}
}

void
print_instrs
(simple_instr *begin)
{
	while (begin != NULL) {
		fprint_instr(stdout, begin);
		begin = begin->next;
	}
}
