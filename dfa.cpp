#include <queue>

#include "bitvecset.h"
#include "dfa.h"

void
DFA
::initialize_inout
(ulong num_blocks, ulong num_props)
{
	in_sets.clear();
	out_sets.clear();
	
	// Initialize to either empty or universe
	for (ulong i = 0; i < num_blocks; ++i) {
		BitVecSet in_set(num_props);
		BitVecSet out_set(num_props);
		
		switch(meet_op) {
			case ANY_PATH:
				in_set.clear_all();
				out_set.clear_all();
				break;
			case ALL_PATH:
				in_set.set_all();
				out_set.set_all();
				break;
			default:
				ASSERT(false);
				break;
		}
		
		in_sets.push_back(in_set);
		out_sets.push_back(out_set);
	}
	
	// Initialize either Entry or Exit appropriately.
	
	ulong block_ix;
	switch(data_dir) {
		case FORWARDS:
			block_ix = 0;
			break;
		case BACKWARDS:
			block_ix = num_blocks - 1;
			break;
		default:
			ASSERT(false);
			break;
	}
	
	// For now assume starting point is always empty.
	in_sets[block_ix].clear_all();
	out_sets[block_ix].clear_all();
}

void
DFA
::gen_links
()
{
	block_array.clear();
	for (BlockIt b = cfg.block_list.begin(); b != cfg.block_list.end(); ++b) {
		block_array.push_back(b);
	}
}

void
DFA
::get_near
(ulong ix, std::vector<ulong> &prevs, std::vector<ulong> &nexts)
{
	prevs.clear();
	nexts.clear();
	
	std::vector<ulong> &one = (data_dir == FORWARDS ? prevs : nexts);
	std::vector<ulong> &two = (data_dir == FORWARDS ? nexts: prevs);
	
	BlockItList &preds = block_array[ix]->preds;
	BlockItList &succs = block_array[ix]->succs;
	for (BlockItIt i = preds.begin(); i != preds.end(); ++i) {
		//ASSERT((*i)->isReachable); - going to handle this case now
		one.push_back((*i)->ix);
	}
	for (BlockItIt i = succs.begin(); i != succs.end(); ++i) {
		//ASSERT((*i)->isReachable); - going to handle this case now
		two.push_back((*i)->ix);
	}
}


void
DFA
::meet_with
(BitVecSet &a, BitVecSet &b)
{
	// Either union or intersection.
	switch(meet_op) {
		case ANY_PATH:
			a.or_with(b);
			break;
		case ALL_PATH:
			a.and_with(b);
			break;
		default:
			ASSERT(false);
			break;
	}
}

DFA
::DFA
(MeetOperator meet_op_, DataDirection data_dir_, CFG &cfg_)
: meet_op(meet_op_), data_dir(data_dir_), cfg(cfg_)
{
}

void
DFA
::solve
(ulong num_props, PropArray &gen_sets, PropArray &kill_sets)
{
	ASSERT(gen_sets.size() == kill_sets.size());
	ulong num_blocks = gen_sets.size();
	
	// First initialize.
	initialize_inout(num_blocks, num_props);
	
	// Generate the links to the blocks.
	gen_links();
	
	// Determine correct direction.
	// Data flows from prev to next.
	PropArray &prev_sets = (data_dir == FORWARDS ? in_sets : out_sets);
	PropArray &next_sets = (data_dir == FORWARDS ? out_sets : in_sets);
	
	// Work queue of next basic blocks to process.
	std::queue<ulong> work_q;
	
	// Start with everything on the queue.
	// Put start and exit at the front of the queue so their ins and/or
	// outs are initialized properly from the gen sets.
	work_q.push(0);
	work_q.push(num_blocks-1);
	for (ulong i = 1; i < num_blocks-1; ++i) {
		work_q.push(i);
	}
	
	while (!work_q.empty()) {
		ulong block_ix = work_q.front();
		work_q.pop();
		
		// special handling of non reachable blocks
		BlockIt curr_block = block_array[block_ix];
		if (not curr_block->isReachable) {
			// do no analysis on them
			continue;
		}
		
		// To check if anything has changed.
		BitVecSet old_next = next_sets[block_ix];
		
		// Get the next and previous blocks in data flow direction.
		// This isn't very efficient, but it's good enough.
		std::vector<ulong> prevs, nexts;
		get_near(block_ix, prevs, nexts);
		
		// Meet operation.
		for (ulong i = 0; i < prevs.size(); ++i) {
			// only meet with reachable blocks
			BlockIt prev_block = block_array[prevs[i]];
			if (prev_block->isReachable) {
				meet_with(prev_sets[block_ix], next_sets[prevs[i]]);
			}
		}
		
		// Perform the transfer function
		next_sets[block_ix] = prev_sets[block_ix];
		next_sets[block_ix].sub_with(kill_sets[block_ix]);
		next_sets[block_ix].or_with(gen_sets[block_ix]);
		
		// Add next ones to the work queue if things changed.
		if (!old_next.equal_to(next_sets[block_ix])) {
			for (ulong i = 0; i < nexts.size(); ++i) {
				work_q.push(nexts[i]);
			}
		}
	}
}
