#include <vector>

#include "dom.h"
#include "bitvecset.h"
#include "cfg.h"
#include "dfa.h"

Dominance
::Dominance
(CFG &cfg_, DataDirection dir)
: cfg(cfg_), dfa(ALL_PATH, dir, cfg_)
{
}

void
Dominance
::run
()
{
	// Start from scratch
	gen_sets.clear();
	kill_sets.clear();
	idoms.clear();
	
	int num_blocks = cfg.block_list.size();
	
	// Kill sets are empty
	// Gen sets are each block itself
	for (BlockIt i = cfg.block_list.begin(); i != cfg.block_list.end(); ++i) {
		BitVecSet gen_set(num_blocks);
		BitVecSet kill_set(num_blocks);
		
		gen_set.clear_all();
		gen_set.set(i->ix, true);
		
		kill_set.clear_all();
		
		gen_sets.push_back(gen_set);
		kill_sets.push_back(kill_set);
	}
	
	dfa.solve(num_blocks, gen_sets, kill_sets);
	
	// now find immediate (post)dominators
	// initialize them all to have no idom
	idoms.clear();
	for (int i = 0; i < num_blocks; ++i) {
		idoms.push_back(NO_IDOM);
	}
	// use whichever is appropriate
	PropArray &doms = (dfa.data_dir == FORWARDS ? dfa.out_sets : dfa.in_sets);
	// I'm using the fact that if block A immediately dominates block B,
	// then block A's dominator set must be equal to block B's proper
	// dominator set.
	// Using this fact instead of the other algorithm of removing blocks
	// from dominator sets means I can keep the dominator sets around
	// instead of having to make a copy.
	// Still, either way is fine, but I just like the first way because
	// I came up with it.
	for (int i = 0; i < num_blocks; ++i) {
		// first make this a proper dominator set
		doms[i].set(i, false);
		
		// check all proper dominators for which one is the immediate
		for (int j = 0; j < num_blocks; ++j) {
			if (doms[i].get(j) && doms[j].equal_to(doms[i])) {
				idoms[i] = j;
				break;
			}
		}
		
		// make it back into a normal dominator set
		doms[i].set(i, true);
	}
}

void
Dominance
::print(char const *proc_name)
{
	if (dfa.data_dir == FORWARDS) {
		printf("idominators %s %lu\n", proc_name, gen_sets.size());
	} else {
		printf("ipostdominators %s %lu\n", proc_name, gen_sets.size());
	}
	for (int i = 0; i < gen_sets.size(); ++i) {
		printf("block %d\n", i);
		printf("\t");
		if (dfa.data_dir == FORWARDS) {
			dfa.out_sets[i].print(stdout);
		} else {
			dfa.in_sets[i].print(stdout);
		}
		printf("\n");
		printf("\tidom");
		if (idoms[i] != NO_IDOM) {
			printf(" %d", idoms[i]);
		}
		printf("\n");
	}
}
