#include <map>
#include <set>

#include "instr.h"
#include "reachdefs.h"

void
ReachDefs
::make_fake_defs
(InstrList &instr_list)
{
	// start from scratch
	initial_defs.clear();
	
	// keep track of which pseudo registers we've seen already
	std::set<simple_reg*> pseudo_regs;
	
	// keep track of all appearences
	RegList reg_list;
	
	for (InstrIt i = instr_list.begin(); i != instr_list.end(); ++i) {
		reg_list.push_back(get_dst(*i));
		get_srcs(*i, reg_list);
	}
	
	// filter and find pseudo register and make fake definitions
	for (RegIt i = reg_list.begin(); i != reg_list.end(); ++i) {
		if ((*i) != NO_REGISTER && (*i)->kind == PSEUDO_REG && pseudo_regs.find(*i) == pseudo_regs.end()) {
			// if not seen this pseudo reg before, then make fake def
			ASSERT((*i)->var != NULL);
			ASSERT((*i)->var->kind == VAR_SYM);
			ASSERT((*i)->var->type != NULL);
			
			simple_instr *fake_def = new_instr(CPY_OP, (*i)->var->type);
			fake_def->u.base.dst = *i;
			fake_def->u.base.src1 = *i;
			fake_def->u.base.src2 = NO_REGISTER;
			initial_defs.push_back(fake_def);
			
			pseudo_regs.insert(*i);
		}
	}
}

void
ReachDefs
::find_all_defs
(InstrList &instr_list)
{
	// start from scratch
	def_table.clear();
	def_array.clear();
	
	// keep track of the index of the defintion
	int ix = 0;
	
	// first process the fake definitions
	for (InstrIt i = initial_defs.begin(); i != initial_defs.end(); ++i) {
		simple_reg *dst = get_dst(*i);
		ASSERT(dst != NO_REGISTER);
		ASSERT(dst->kind == PSEUDO_REG);
		// Found a definition, so add it to the table.
		def_table[*i] = ix;
		def_array.push_back(*i);
		++ix;
	}
	
	// then process the real instructions
	for (InstrIt i = instr_list.begin(); i != instr_list.end(); ++i) {
		simple_reg *dst = get_dst(*i);
		if (dst != NO_REGISTER && dst->kind == PSEUDO_REG) {
			// Found a definition, so add it to the table.
			def_table[*i] = ix;
			def_array.push_back(*i);
			++ix;
		}
	}
	
	
}

void
ReachDefs
::block_set
(BlockIt b, BitVecSet &def_set, BitVecSet &kill_set)
{
	kill_set.clear_all();
	def_set.clear_all();
	
	// special case for the Entry block
	if (b->ix == 0) {
		// no kill set
		
		// gen set is the fake defs
		for (int i = 0; i < initial_defs.size(); ++i) {
			def_set.set(i, true);
		}
	} else {
		// go through instructions from begin to end
		InstrIt instr = b->instrs.front();
		for (int i = 0; i < b->instrs.size(); ++i) {
			simple_reg *dst = get_dst(*instr);
			if (dst != NO_REGISTER && dst->kind == PSEUDO_REG) {
				// kills all defs of this reg
				for (int j = 0; j < def_array.size(); ++j) {
					if (get_dst(def_array[j]) == dst) {
						kill_set.set(j, true);
						def_set.set(j, false);
					}
				}
				// gens a single new def here
				def_set.set(def_table[*instr], true);
			}
			++instr;
		}
	}
}

void
ReachDefs
::make_chains
()
{
	// start from scratch
	def_use_chain.clear();
	use_def_chain.clear();
	
	// first deal with temporary registers
	
	typedef std::map<simple_reg*, InstrIt> TempDefs;
	typedef TempDefs::iterator TempDefsIt;
	
	TempDefs temp_defs;
	
	// first find all definitions of temporary registers
	for (InstrIt i = cfg.instr_list.begin(); i != cfg.instr_list.end(); ++i) {
		simple_reg *dst = get_dst(*i);
		if (dst != NO_REGISTER && dst->kind == TEMP_REG) {
			temp_defs[dst] = i;
		}
	}
	
	// now find all uses and create the links
	// going to iterate block by block and all the instructions in a block
	RegList args;
	for (BlockIt b = cfg.block_list.begin(); b != cfg.block_list.end(); ++b) {
		
		// need to run through the def set again, so make a copy
		BitVecSet in_def_set = dfa.in_sets[b->ix];
		
		InstrIt instr = b->instrs.front();
		for(int i = 0; i < b->instrs.size(); ++i) {
			
			RegList args;
			get_srcs(*instr, args);
			int ix = 0;
			for (RegIt r = args.begin(); r != args.end(); ++r, ++ix) {
				if (*r != NO_REGISTER) {
					
					if ((*r)->kind == TEMP_REG) {
						// use our temp_defs to make links
						ASSERT(temp_defs.find(*r) != temp_defs.end());
						InstrIt def_instr = temp_defs[*r];
						
						// make def->use
						def_use_chain[*def_instr].push_back(UseLink(*instr, ix));
						
						// make use->def
						use_def_chain[*instr].push_back(DefLink(*def_instr, ix));
						
					} else if ((*r)->kind == PSEUDO_REG) {
						// use the results of the dfa
						
						// go through the incoming defs
						for (int k = 0; k < def_table.size(); ++k) {
							if (in_def_set.get(k)) {
								
								simple_instr *def_instr = def_array[k];
								simple_reg *dst = get_dst(def_instr);
								if (dst == *r) {
									// if this is a def of this reg
									// make def->use
									def_use_chain[def_instr].push_back(UseLink(*instr, ix));
									
									// make use->def
									use_def_chain[*instr].push_back(DefLink(def_instr, ix));
								}
							}
						}
						
					}
				}
			}
			
			// update the stuff afterwards, not before
			simple_reg *curr_dst = get_dst(*instr);
			// need to update the def set in place
			if (curr_dst != NO_REGISTER and curr_dst->kind == PSEUDO_REG) {
				// kills all defs of this reg
				for (int j = 0; j < def_array.size(); ++j) {
					if (get_dst(def_array[j]) == curr_dst) {
						in_def_set.set(j, false);
					}
				}
				// gens a single new def here
				in_def_set.set(def_table[*instr], true);
			}
			
			++instr;
		}
	}
	
}

ReachDefs
::ReachDefs
(CFG &cfg_)
: cfg(cfg_), dfa(ANY_PATH, FORWARDS, cfg_)
{
}

void
ReachDefs
::run
()
{
	make_fake_defs(cfg.instr_list);
	find_all_defs(cfg.instr_list);
	
	// start from scratch
	def_sets.clear();
	kill_sets.clear();
	
	// Compute def and use sets for each basic block.
	for (BlockIt i = cfg.block_list.begin(); i != cfg.block_list.end(); ++i) {
		BitVecSet def_set(def_table.size());
		BitVecSet kill_set(def_table.size());
		
		block_set(i, def_set, kill_set);
		
		def_sets.push_back(def_set);
		kill_sets.push_back(kill_set);
	}
	
	// Compute in and out sets using DFA.
	dfa.solve(def_table.size(), def_sets, kill_sets);
	
	// now make the chains
	make_chains();
}

void
ReachDefs
::print
(char const *proc_name)
{
	printf("reaching definitions %s %u\n", proc_name, def_table.size());
	
	// first print out all the defs
	for (int i = 0; i < def_table.size(); ++i) {
		if (i == initial_defs.size()) {
			printf("\t-- real --\n");
		}
		printf("ix%d: %p", i, def_array[i]);
		print_instrs(def_array[i], 1);
	}
	
	// now print out the def/kill/out sets
	//for (int i = 0; i < def_sets.size(); ++i) {
		//printf("block %d\n", i);
		//printf("\tdef  ");
		//def_sets[i].print(stdout);
		//printf("\n");
		//printf("\tkill ");
		//kill_sets[i].print(stdout);
		//printf("\n");
		//printf("\tout  ");
		//dfa.out_sets[i].print(stdout);
		//printf("\n");
	//}
	
	// now print out the chains
	//for (DefUseChainIt i = def_use_chain.begin(); i != def_use_chain.end(); ++i) {
		//simple_instr *instr = i->first;
		//UseLinkList &ul = i->second;
		//printf("def %p %c%d\n", instr, get_dst(instr)->kind == PSEUDO_REG? 'r':'t', get_dst(instr)->num);
		//for (UseLinkIt j = ul.begin(); j != ul.end(); ++j) {
			//printf("\tuse %p:%d\n", j->usage, j->i);
		//}
	//}
	//for (UseDefChainIt i = use_def_chain.begin(); i != use_def_chain.end(); ++i) {
		//simple_instr *instr = i->first;
		//DefLinkList &dl = i->second;
		//printf("use %p\n", instr);
		//for (DefLinkIt j = dl.begin(); j != dl.end(); ++j) {
			//printf("\tdefs %p:%d\n", j->def, j->i);
		//}
	//}
}
