// compile time interpreter
// interpret a section of code, outputting a condensed list of instructions
// must only be able to enter section through first instruction and exit
// through last instruction

#ifndef INTERPRETER_H
#define INTERPRETER_H

// maximum number of steps to interpret before concluding it's an infinite loop
#define MAX_STEPS 15000

#include <cstdio>
#include <vector>
#include <map>

#include "common.h"
#include "bitvecset.h"
#include "cfg.h"
#include "reachdefs.h"

// the value of a register during interpretation
// only work with integral values
struct AbstractValue {
	// only support this kind of expression: (C1 + C2*x)
	// where C1 and C2 are constants and x is a unique symbolic value (like in
	// local value numbering)
	
	int c1;
	int c2;
	
	// represents a pseudo register
	// negative means no register
	// if negative, c2 must be 0
	int symbol;
	
	// make a constant to start out with
	AbstractValue(int c1_);
	
	// start off with 0
	AbstractValue();
};

// add two abstract values
// only possible if both have the same symbol or at least one has no symbol
// returns whether it was possible
bool av_add(AbstractValue const& a, AbstractValue const& b, AbstractValue& r);

// add two abstract values
// only possible if at least one has no symbol
bool av_mul(AbstractValue const& a, AbstractValue const& b, AbstractValue& r);

// tries to extract out a constant
// returns whether it is a constant
bool av_extract(AbstractValue const& a, int& r);

// store where the labels are
typedef std::map<simple_sym*, InstrIt> LabelMap;
typedef LabelMap::iterator LabelMapIt;

// keep track of values of registers
typedef std::map<simple_reg*, AbstractValue> RegTable;
typedef RegTable::iterator RegTableIt;

// keep track of symbols
typedef std::map<simple_reg*, int> SymbolIndex;
typedef SymbolIndex::iterator SymbolIndexIt;
// inverse
typedef std::vector<simple_reg*> IndexSymbol;
typedef IndexSymbol::iterator IndexSymbolIt;

class Interpreter {
private:
	// keep references to useful stuff
	CFG &cfg;
	ReachDefs &reach_defs;
	
	LabelMap label_map;
	RegTable reg_table;
	SymbolIndex symbol_index;
	IndexSymbol index_symbol;
	
	// populate the label map
	void find_labels();
	
	// get the index of this register, making a new one if needed
	int get_symbol_index(simple_reg *reg);
	
	// interpret a single instruction
	// sets the next instruction to interpret
	// returns whether we could interpret it
	bool interpret_instr(InstrIt instr, InstrIt &next);
	
	// delete all instructions except the given one
	void clear_except(simple_instr* i);
	
	// add instructions before ret to directly compute the value
	void recompile(InstrIt ret, AbstractValue const& a);
	
	// try to interpret the code
	// return whether the interpretation was successful or not
	bool interpret();
	
public:
	Interpreter(CFG &cfg_, ReachDefs &reach_defs_);
	
	// returns whether anything was changed
	bool run();
};

#endif /* INTERPRETER_H */
