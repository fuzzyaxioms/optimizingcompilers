#include <cstdio>
#include <map>
#include <vector>
#include <list>
#include <set>
#include <algorithm>
#include <utility>

#include "cfg.h"

void
CFG
::cleanup_temps()
{
	typedef std::set<simple_reg*> RegSet;
	typedef RegSet::iterator RegSetIt;
	
	RegSet temps;
	
	// find all uses
	FOREACH(InstrIt, i, instr_list) {
		RegList args;
		get_srcs(*i, args);
		FOREACH(RegIt, r, args) {
			if (*r != NO_REGISTER and (*r)->kind == TEMP_REG) {
				temps.insert(*r);
			}
		}
	}
	
	// find defs with no uses and delete them
	for (InstrIt i = instr_list.begin(); i != instr_list.end();) {
		simple_reg *dst = get_dst(*i);
		if (dst != NO_REGISTER and dst->kind == TEMP_REG and temps.find(dst) == temps.end()) {
			// remove it
			i = instr_list.erase(i);
		} else {
			// move along
			++i;
		}
	}
}

void
CFG
::make_block_list
()
{
	// Start from scratch.
	label_table.clear();
	block_list.clear();
	
	// Add entry basic block.
	block_list.push_back(Block());
	
	// Scan:
	// - find leaders and make basic blocks
	// - map labels to basic blocks
	
	// For counting the number of instructions within the current block.
	ulong block_instr_count = 0;
	// For keeping an iterator to the current block.
	BlockIt curr_block = block_list.begin();
	simple_instr *prev = NULL;
	for (InstrIt iter = instr_list.begin(); iter != instr_list.end(); ++iter) {
		
		// See new instruction.
		++block_instr_count;
		
		// If is leader, close off current block and start a new block.
		if (is_leader(prev, *iter)) {
			// the previous basic block is now finished
			// so update its range of instructions
			block_list.back().finish_make(block_instr_count-1);
			
			// start a new basic block
			//starts with with its leader instruction
			block_instr_count = 1;
			
			// make it reference the leader as the first instruction
			curr_block = block_list.insert(block_list.end(), Block(iter));
		}
		
		// If label, then map it to the corresponding block.
		if ((*iter)->opcode == LABEL_OP) {
			ASSERT((*iter)->u.label.lab != NULL);
			label_table.insert(std::make_pair((*iter)->u.label.lab, curr_block));
		}
		
		prev = *iter;
	}
	
	// We should have a dangling basic block so we should close it off
	curr_block->finish_make(block_instr_count);
	
	// finally add Exit block
	block_list.push_back(Block());
}

BlockEdgeIt
CFG
::new_edge
(EdgeType type, BlockIt src, BlockIt dest)
{
	return block_edge_list.insert(block_edge_list.end(), BlockEdge(type, src, dest));
}

void
CFG
::connect_reg_jump
(BlockIt src, simple_sym *lbl)
{
	LabelTableIt lookup = label_table.find(lbl);
	ASSERT(lookup != label_table.end());
	BlockIt dest = lookup->second;
	BlockEdgeIt e = new_edge(REG_JUMP, src, dest);
	src->reg_jump = e;
}

void
CFG
::connect_reg_jump
(BlockIt src, BlockIt dest)
{
	BlockEdgeIt e = new_edge(REG_JUMP, src, dest);
	src->reg_jump = e;
}

void
CFG
::connect_fall_thru
(BlockIt src, BlockIt dest)
{
	BlockEdgeIt e = new_edge(FALL_THRU, src, dest);
	src->fall_thru = e;
}

void
CFG
::connect_cond_jump
(BlockIt src, simple_sym *lbl)
{
	LabelTableIt lookup = label_table.find(lbl);
	ASSERT(lookup != label_table.end());
	BlockIt dest = lookup->second;
	BlockEdgeIt e = new_edge(COND_JUMP, src, dest);
	src->cond_jump = e;
}

void
CFG
::connect_def_label
(BlockIt src, simple_sym *lbl)
{
	LabelTableIt lookup = label_table.find(lbl);
	ASSERT(lookup != label_table.end());
	BlockIt dest = lookup->second;
	BlockEdgeIt e = new_edge(DEF_LABEL, src, dest);
	src->def_label = e;
}

void
CFG
::connect_reg_label
(BlockIt src, simple_sym **lbl, ulong n)
{
	src->reg_label.clear();
	for (ulong i = 0; i < n; ++i) {
		LabelTableIt lookup = label_table.find(lbl[i]);
		ASSERT(lookup != label_table.end());
		BlockIt dest = lookup->second;
		BlockEdgeIt e = new_edge(REG_LABEL, src, dest);
		src->reg_label.push_back(e);
	}
}

void
CFG
::make_connections
()
{
	// start from scratch
	block_edge_list.clear();
	
	BlockIt entry_block = block_list.begin();
	BlockIt start_block = ++block_list.begin();
	
	// Make the special Entry fall through to real first block.
	connect_fall_thru(entry_block, start_block);
	
	BlockIt exit_block = --block_list.end();
	
	// Now we can skip the first block.
	// Also skip the exit block since it only has predecessors.
	for (BlockIt iter = start_block; iter != exit_block; ++iter) {
		ASSERT(iter->instrs.size() > 0);
		
		BlockIt next_iter = iter;
		++next_iter;
		
		simple_instr *last = *iter->instrs.back();
		
		switch(last->opcode) {
			
			case JMP_OP:
				connect_reg_jump(iter, last->u.bj.target);
				break;
			case RET_OP:
				// This is the only way to connect to Exit.
				connect_reg_jump(iter, exit_block);
				break;
			case MBR_OP:
				connect_def_label(iter, last->u.mbr.deflab);
				connect_reg_label(iter, last->u.mbr.targets, last->u.mbr.ntargets);
				break;
			case BTRUE_OP:
			case BFALSE_OP:
				// Has both conditional jump and fall through.
				connect_cond_jump(iter, last->u.bj.target);
				// Fall through to take care of fall through.
			default:
				// Make sure not to fall through to Exit
				ASSERT(next_iter != exit_block);
				connect_fall_thru(iter, next_iter);
				break;
		}
	}
}

void
CFG
::gen_preds_succs
()
{
	// first clear the lists.
	for (BlockIt i = block_list.begin(); i != block_list.end(); ++i) {
		i->preds.clear();
		i->succs.clear();
	}
	// now process all edges
	for (BlockEdgeIt e = block_edge_list.begin(); e != block_edge_list.end(); ++e) {
		// set predecessor
		e->dest->preds.push_back(e->src);
		// set successor
		e->src->succs.push_back(e->dest);
	}
}

void
CFG
::remove_unreachable
()
{
	FOREACH(BlockIt, b, block_list) {
		if (not b->isReachable) {
			// remove all instructions
			FOREACH(InstrItIt, it, b->instrs) {
				instr_list.erase(*it);
			}
		}
	}
}

CFG
::CFG
(InstrList &instr_list_)
: instr_list(instr_list_)
{
}

// Helper recursive function for calculating reachability from Entry
void expand_reachable(BlockIt block) {
	// Stop if already visisted
	if (block->isReachable) {
		return;
	}
	
	// Otherwise, visit this and all successors.
	block->isReachable = true;
	
	for (BlockItIt s = block->succs.begin(); s != block->succs.end(); ++s) {
		expand_reachable(*s);
	}
}

void
CFG
::calc_reachable() {
	// First initialize all blocks to being unreachable.
	for (BlockIt iter = block_list.begin(); iter != block_list.end(); ++iter) {
		iter->isReachable = false;
	}
	
	// Now do recursive DFS from Entry. Use isReachable to mark visisted blocks.
	expand_reachable(block_list.begin());
	
	// print out if any blocks are unreachable
	for (BlockIt iter = block_list.begin(); iter != block_list.end(); ++iter) {
		if (not iter->isReachable) {
			printf("NOT REACHABLE %d\n", iter->ix);
		}
	}
}

void
CFG
::number
()
{
	int ix = 0;
	for (BlockIt iter = block_list.begin(); iter != block_list.end(); ++iter) {
		iter->ix = ix;
		++ix;
	}
}


// do all the things
void
CFG
::run
()
{
	// do important cleanup
	cleanup_temps();
	
	// Next scan through instructions and make the basic blocks.
	// There are no edges yet.
	make_block_list();
	
	// Now make the edges.
	make_connections();
	
	// Finish with some initializations.
	number();
	gen_preds_succs();
	calc_reachable();
	
	// now clean up unreachable code
	remove_unreachable();
	
	// and redo the whole thing
	// do important cleanup
	cleanup_temps();
	
	// Next scan through instructions and make the basic blocks.
	// There are no edges yet.
	make_block_list();
	
	// Now make the edges.
	make_connections();
	
	// Finish with some initializations.
	number();
	gen_preds_succs();
	calc_reachable();
}
void
CFG
::print
(char const *proc_name)
{
	
	// keep track of how many instructions we've outputted
	ulong instr_count = 0;
	
	printf("cfg %s %lu\n", proc_name, block_list.size());
	for (BlockIt iter = block_list.begin(); iter != block_list.end(); ++iter) {
		printf("block %lu\n", iter->ix);
		
		// print the instruction numbers
		// this will automatically be sorted
		printf("\tinstrs %lu", iter->instrs.size());
		for (ulong j = 0; j < iter->instrs.size(); ++j) {
			printf(" %lu", instr_count + j);
		}
		printf("\n");
		
		// Update how many instructions we've outputted
		instr_count += iter->instrs.size();
		
		// print the successors
		// the reference seems to actually sort these, so I will too
		iter->succs.sort(block_cmp);
		
		printf("\tsuccessors %lu", iter->succs.size());
		for (BlockItIt jter = iter->succs.begin(); jter != iter->succs.end(); ++jter) {
			printf(" %lu", (*jter)->ix);
		}
		printf("\n");
		
		// print the predecessors
		iter->preds.sort(block_cmp);
		
		printf("\tpredecessors %lu", iter->preds.size());
		for (BlockItIt jter = iter->preds.begin(); jter != iter->preds.end(); ++jter) {
			printf(" %lu", (*jter)->ix);
		}
		printf("\n");
	}
}
