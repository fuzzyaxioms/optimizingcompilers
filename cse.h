// common subexpression elimination
#ifndef CSE_H
#define CSE_H

#include <map>
#include <vector>
#include <set>

#include "common.h"
#include "bitvecset.h"
#include "cfg.h"
#include "dfa.h"

// not all expressions can be eliminated
// we will stick with ordinary binary expressions - no memory instructions
// maybe add unary expressions as well - but dunno if that'll help or not
// the expression must use either pseudo registers, or a constant temp register
// because of the constant temp register, we need to have a little bit of work
// to determine if two expressions "look" alike i.e. we need to keep track of
// constant temporary registers and their values

// like the other optimizations, this needs to be run multiple times to get
// the full optimization. Right now, this can only eliminate a single evaluation
// at a time.

// an alternative representation of an expression to be used as a key in a map
// no argument is represented by NO_REGISTER and a REG_ARG
enum ArgType {
	CONST_ARG,
	REG_ARG
};
union ConstOrVar {
	simple_reg *reg;
	simple_immed immed;
};
struct ExpKey {
	
	simple_op opcode;
	simple_type *type;
	
	ArgType type1;
	ConstOrVar src1;
	
	ArgType type2;
	ConstOrVar src2;
};

// print out a nice representation
void print_arg(ArgType const& type, ConstOrVar const& src);

// makes a total ordering
class ExpKeyLT {
private:
	// compare two arguments
	// negative for lt, 0 for eq, positive for gt
	int cmp_arg(ArgType const& type1, ConstOrVar const& src1, ArgType const& type2, ConstOrVar const& src2) const;
public:
	// compare two exp keys
	bool operator()(ExpKey const& exp1, ExpKey const& exp2) const;
};

// whether two constants are the same
bool immed_equal(simple_immed *a, simple_immed *b);


// keep track of constant temp regs
typedef std::map<simple_reg*, simple_immed*> TempTable;
typedef TempTable::iterator TempTableIt;

// for available expressions; keep track of the expressions, and their index
typedef std::map<ExpKey, int, ExpKeyLT> ExpTable;
typedef ExpTable::iterator ExpTableIt;
// inverse
typedef std::vector<ExpKey> ExpArray;
typedef ExpArray::iterator ExpArrayIt;

// for reaching evaluations
// this is slightly tricky
// i want to find earliest reaching expression, but i can't with my framework
// so i do closest reaching expression instead, which should work well enough
typedef std::map<simple_instr*,int> ExpDefTable;
typedef ExpDefTable::iterator ExpDefTableIt;
// inverse
typedef std::vector<InstrIt> ExpDefArray;
typedef ExpDefArray::iterator ExpDefArrayIt;

// keep track of which expressions we've already eliminated
typedef std::set<ExpKey, ExpKeyLT> ExpSet;
typedef ExpSet::iterator ExpSetIt;

// because my DFA framework only works with bit vectors, I can't just use
// available expressions; I also need reaching expressions so I can actually
// find out where the available expressions come from.
class CSE {
private:
	// useful references
	CFG &cfg;
	
	// constant temp refs
	TempTable temp_table;
	
	// available expressions
	ExpTable exp_table;
	ExpArray exp_array;
	
	// reaching expressions
	ExpDefTable exp_def_table;
	ExpDefArray exp_def_array;
	
	// keep track of expressions we've already eliminated
	ExpSet exp_set;
	
	// find all constant temp regs
	void make_temp_table();
	
	// whether two registers are equal
	// needs the temp table
	bool reg_equal(simple_reg *a, simple_reg *b);
	
	// whether this expression is of the kind we want
	// uses temp_table to find constant temp regs
	// makes the ExpKey version if it is of the kind we want
	bool want_exp(simple_instr *i, ExpKey &key);
	
	// find all expressions and defs of expressions
	// populate the tables and arrays of the DFA analysis
	// uses temp_table
	void find_exps();
	
	// DFA step for available expressions
	void ae_step(simple_instr *i, BitVecSet &gen_set, BitVecSet &kill_set);
	// make the gen and kill sets for ae
	void prepare_ae();
	
	// DFA step for reaching expressions defs
	void re_step(simple_instr *i, BitVecSet &gen_set, BitVecSet &kill_set);
	// make the gen and kill sets for re
	void prepare_re();
	
	// see if we can eliminate the expression for this instruction
	// uses exp_set to filter
	// returns whether we did or not
	bool eliminate_exp(InstrIt instr, BitVecSet &ae_gen_set, BitVecSet &re_gen_set);
	
	// go through all basic blocks and instructions and try to eliminate expressions
	// returns whether anything changed i.e. anything eliminated
	bool eliminate_exps();
	
public:
	// available expressions
	PropArray ae_gen_sets;
	PropArray ae_kill_sets;
	// reaching expressions
	PropArray re_gen_sets;
	PropArray re_kill_sets;
	
	// Dataflow analysis
	DFA ae_dfa;
	DFA re_dfa;
	
	// Initialize
	CSE(CFG &cfg_);
	
	// returns whether anything changed i.e. any copies propagated
	bool run();
	
	// Prints out definitions and stuff.
	void print(char const *proc_name);
};


#endif /* CSE_H */
