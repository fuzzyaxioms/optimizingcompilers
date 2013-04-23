// Using instructions and lists of instructions.
#ifndef INSTR_H
#define INSTR_H

#include <list>
#include <set>

#include "common.h"

// I'm using a list so that adding/removing instructions in the future
// will be easy. The numbering of the instructions will be implicit in
// their order in the list, so I won't have to renumber them all
// whenever an instruction is added/removed. But it might be simpler
// just to use the doubly linked list of simple_instr* directly, however
// using a list allows me to use other STL algorithms which may help in
// the future.

// Iterators will be used as references to instructions.
typedef std::list<simple_instr*> InstrList;
typedef InstrList::iterator InstrIt;
typedef InstrList::const_iterator CInstrIt;

typedef std::set<simple_instr*> InstrSet;
typedef InstrSet::iterator InstrSetIt;

// General list of registers
typedef std::list<simple_reg*> RegList;
typedef RegList::iterator RegIt;

// Create the list from the given procedure list.
// This doesn't do any other processing.
void to_instr_list(simple_instr *inlist, InstrList &instr_list);

// Remakes the linked list of simple_instr*.
simple_instr* from_instr_list(InstrList &instr_list);

// Decide whether the current instruction is a leader
// i.e. start of new basic block
bool is_leader(simple_instr *prev, simple_instr *curr);

// get the destination register of an instruction, or NO_REGISTER if n/a
simple_reg *get_dst(simple_instr *instr);

// set the destination
void set_dst(simple_instr *instr, simple_reg *dst);

// get the registers that are being used
// gets all types of registers, even NO_REGISTER
void get_srcs(simple_instr *instr, RegList &reg_list);

simple_reg *get_src(simple_instr *instr, int i);

// change one of the arguments
void set_src(simple_instr *instr, simple_reg *arg, int i);

// Print range of instructions using print_simple's library.
// end is one past the end just like in STL.
void print_instrs(InstrIt begin, InstrIt end);

// Alternative version with an explicit number.
void print_instrs(InstrIt begin, ulong count);

// Print range using raw pointers.
void print_instrs(simple_instr *begin, ulong count);

// Print until NULL using raw pointers.
void print_instrs(simple_instr *begin);

#endif /* INSTR_H */
