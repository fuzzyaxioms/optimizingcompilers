#include <map>
#include <utility>
#include <algorithm>

#include "common.h"
#include "bitvecset.h"
#include "block.h"
#include "dom.h"
#include "loop.h"

bool
loop_cmp(HeadLoop const &a, HeadLoop const &b) {
	return (a.head->ix) < (b.head->ix);
}

void
NaturalLoops
::find_loops
()
{
	// start from scratch
	loop_list.clear();
	
	// keep track of which blocks are already heads of loops
	typedef std::map<ulong, HeadLoopIt> LoopTable;
	typedef std::map<ulong, HeadLoopIt>::iterator LoopTableIt;
	
	// Keep track of which blocks are already heads of loops.
	// Uses the index of the head block as the key.
	LoopTable loop_table;
	
	// go over all edges, finding the back edges
	// i.e. where the dest dominates the src
	for (BlockEdgeIt e = cfg.block_edge_list.begin(); e != cfg.block_edge_list.end(); ++e) {
		BlockIt tail = e->src;
		int tail_ix = e->src->ix;
		
		BlockIt head = e->dest;
		int head_ix = e->dest->ix;
		
		// if dest dominates the src
		if (doms.dfa.in_sets[tail_ix].get(head_ix)) {
			// check if we already have a loop with this head
			LoopTableIt lookup = loop_table.find(head_ix);
			HeadLoopIt h = loop_list.end();
			if (lookup == loop_table.end()) {
				// if we don't, make one
				h = loop_list.insert(loop_list.end(), HeadLoop());
				h->head = head;
			} else {
				h = lookup->second;
			}
			
			// add the tail to the loop
			h->tails.push_back(tail);
		}
	}
}

void
NaturalLoops
::visit_upwards
(BlockIt curr, BitVecSet &visited, BlockItList &blks)
{
	ASSERT(curr->isReachable);
	// stop when already visited
	if (visited.get(curr->ix)) {
		return;
	}
	
	visited.set(curr->ix, true);
	blks.push_back(curr);
	
	// Visit preds.
	for (BlockItIt pd = curr->preds.begin(); pd != curr->preds.end(); ++pd) {
		visit_upwards(*pd, visited, blks);
	}
}

void
NaturalLoops
::populate_loops() {
	for (HeadLoopIt h = loop_list.begin(); h != loop_list.end(); ++h) {
		BitVecSet visited(cfg.block_list.size());
		visited.clear_all();
		
		// The head is visited.
		visited.set(h->head->ix, true);
		h->blocks.push_back(h->head);
		
		// Visit all tails and go upwards recursively.
		// Since the head dominates, all paths upwards must end up at head.
		for (BlockItIt t = h->tails.begin(); t != h->tails.end(); ++t) {
			visit_upwards(*t, visited, h->blocks);
		}
	}
}

bool
NaturalLoops
::find_preheader
(HeadLoopIt h)
{
	// make a set out of the blocks of this loop for fast lookup
	BitVecSet loop_blocks(cfg.block_list.size());
	loop_blocks.clear_all();
	
	for (BlockItIt i = h->blocks.begin(); i != h->blocks.end(); ++i) {
		loop_blocks.set((*i)->ix, true);
	}
	
	// check if the head has exactly one predecessor not in the loop
	int num_outside = 0;
	BlockIt outside_block;
	for (BlockItIt p = h->head->preds.begin(); p != h->head->preds.end(); ++p) {
		if (not loop_blocks.get((*p)->ix)) {
			++num_outside;
			outside_block = *p;
		}
	}
	
	if (num_outside != 1) {
		return false;
	}
	
	// make sure the outside block isn't a special block
	if (outside_block->instrs.size() <= 0) {
		return false;
	}
	
	// check to make sure it's not a conditional jump
	simple_op lastop = (*outside_block->instrs.back())->opcode;
	switch(lastop) {
		case BTRUE_OP:
		case BFALSE_OP:
		case MBR_OP:
			// conditional jumps can go to other places
			return false;
		case JMP_OP:
			ASSERT(outside_block->reg_jump->dest == h->head);
			h->fall_thru = false;
			break;
		default:
			ASSERT(outside_block->fall_thru->dest == h->head);
			h->fall_thru = true;
			break;
	}
	
	// found a good block for a preheader, so set it
	h->preheader = outside_block;
	return true;
}

void
NaturalLoops
::make_preheader
(HeadLoopIt h)
{
	simple_sym *loop_label = NULL;
	
	// add a label to the head block if it doesn't have one
	InstrIt first_instr = h->head->instrs.front();
	simple_op firstop = (*first_instr)->opcode;
	if (firstop != LABEL_OP) {
		simple_instr *lbl_instr = new_instr(LABEL_OP, simple_type_void);
		loop_label = new_label();
		lbl_instr->u.label.lab = loop_label;
		
		// insert it before the head
		InstrIt lbl = cfg.instr_list.insert(first_instr, lbl_instr);
		
		// keep track of the new first instruction
		first_instr = lbl;
		
		// don't update the cfg's label table since we don't need to
	} else {
		// update loop_label accordingly
		loop_label = (*first_instr)->u.label.lab;
	}
	
	// add the preheader label - don't need to make a new basic block
	simple_sym *preheader_label = new_label();
	simple_instr *preheader_instr = new_instr(LABEL_OP, simple_type_void);
	preheader_instr->u.label.lab = preheader_label;
	cfg.instr_list.insert(first_instr, preheader_instr);
	// now the preheader falls through to the loop head
	
	// make a set out of the blocks of this loop for fast lookup
	// this will miss newly created preheaders, but that should be fine
	BitVecSet loop_blocks(cfg.block_list.size());
	loop_blocks.clear_all();
	
	for (BlockItIt i = h->blocks.begin(); i != h->blocks.end(); ++i) {
		loop_blocks.set((*i)->ix, true);
	}
	
	// go through all edges and find the preds
	for (BlockEdgeIt e = cfg.block_edge_list.begin(); e != cfg.block_edge_list.end(); ++e) {
		// if this is a pred
		if (e->dest == h->head) {
			BlockIt b = e->src;
			
			// check if the source is inside the loop or not
			if (loop_blocks.get(b->ix) && e->type == FALL_THRU) {
				// take care of blocks inside the loop that fall through to the head
				// the ones that jump stay as they are
				// add an unconditional jump to the block
				InstrIt past_last_instr = ++b->instrs.back();
				simple_instr* jump_instr = new_instr(JMP_OP, simple_type_void);
				jump_instr->u.bj.target = loop_label;
				cfg.instr_list.insert(past_last_instr, jump_instr);
			} else if (e->type != FALL_THRU) {
				// don't need to take care of other blocks that fall through to the head,
				// because they now must necessary fall through to the preheade
				// but we do need to take care of other blocks that jump to the loop head
				simple_instr* last_instr = *b->instrs.back();
				switch(last_instr->opcode) {
					case JMP_OP:
					case BTRUE_OP:
					case BFALSE_OP:
						ASSERT(last_instr->u.bj.target == loop_label);
						last_instr->u.bj.target = preheader_label;
						break;
					case MBR_OP:
						if (last_instr->u.mbr.deflab == loop_label) {
							last_instr->u.mbr.deflab = preheader_label;
						}
						for (int j = 0; j < last_instr->u.mbr.ntargets; ++j) {
							if (last_instr->u.mbr.targets[j] == loop_label) {
								last_instr->u.mbr.targets[j] == preheader_label;
							}
						}
						break;
					default:
						// this shouldn't happen
						ASSERT(false);
						break;
				}
			}
		}
	}
}

NaturalLoops
::NaturalLoops
(CFG &cfg_, Dominance &doms_)
: cfg(cfg_), doms(doms_)
{
}

bool
NaturalLoops
::run
()
{
	// find the backedges and natural loops
	find_loops();
	// find the blocks of each natural loop group
	populate_loops();
	
	// try to find preheaders for all loops
	// if it can't, it creates just one and stops
	for (HeadLoopIt h = loop_list.begin(); h != loop_list.end(); ++h) {
		if (not find_preheader(h)) {
			make_preheader(h);
			return true;
		}
	}
	return false;
}

void
NaturalLoops
::print(char const *proc_name) {
	printf("natural_loops %s %lu\n", proc_name, loop_list.size());
	
	// sort the loops by head blocks
	loop_list.sort(loop_cmp);
	
	ulong count = 0;
	for (HeadLoopIt h = loop_list.begin(); h != loop_list.end(); ++h, ++count) {
		printf("natloop %lu\n", count);
		printf("\tbackedge %lu", h->head->ix);
		// sort the tail blocks
		h->tails.sort(block_cmp);
		for (BlockItIt t = h->tails.begin(); t != h->tails.end(); ++t) {
			printf(" %lu", (*t)->ix);
		}
		printf("\n");
		printf("\tpreheader %d\n", h->preheader->ix);
		printf("\tloop_blocks");
		// sort the loop blocks
		h->blocks.sort(block_cmp);
		for (BlockItIt bk = h->blocks.begin(); bk != h->blocks.end(); ++bk) {
			printf(" %lu", (*bk)->ix);
		}
		printf("\n");
	}
	
}
