// cfg related functions and data structures
#ifndef CFG_H
#define CFG_H

#include <cstdlib>
#include <list>
#include <map>

#include "common.h"
#include "instr.h"
#include "block.h"

// Working with the label table
typedef std::map<simple_sym*, BlockIt> LabelTable;
typedef LabelTable::iterator LabelTableIt;

// control flow graph
class CFG {
private:
	// clean up unused temp regs because they can cause trouble with other code
	void cleanup_temps();
	
	// After making the instruction list, make the block list.
	// Also populates the label_table. No edges are added yet.
	void make_block_list();
	
	// Makes new edge and adds to edge list.
	BlockEdgeIt new_edge(EdgeType type, BlockIt src, BlockIt dest);
	
	// Make edges when constructing CFG.
	// The fall thru case returns a value because I need it for the
	// special case of Entry.
	void connect_reg_jump(BlockIt src, simple_sym *lbl);
	void connect_reg_jump(BlockIt src, BlockIt dest);
	void connect_fall_thru(BlockIt src, BlockIt dest);
	void connect_cond_jump(BlockIt src, simple_sym *lbl);
	void connect_def_label(BlockIt src, simple_sym *lbl);
	void connect_reg_label(BlockIt src, simple_sym **lbl, ulong n);
	
	// After making basic blocks, make all connections.
	// This creates edges and populates the edge list.
	void make_connections();
	
	// Generate all the preds and succs summary lists for the blocks.
	// Needs all connections to be made already. Uses the edge list.
	void gen_preds_succs();
	
	// removes all unreachable blocks
	void remove_unreachable();
	
public:
	// List of instructions. The numbering is the ordering.
	InstrList &instr_list;
	
	// List of basic blocks. The numbering is the ordering.
	BlockList block_list;
	
	// List of edges.
	BlockEdgeList block_edge_list;
	
	// Maps labels to basic blocks.
	LabelTable label_table;
	
	// Initializes instr_list, and does nothing else
	CFG(InstrList &instr_list_);
	
	// Find all reachable blocks from Entry.
	// Needs succs.
	void calc_reachable();
	
	// Scan through the basic blocks and number them. This is done when
	// the block's numbers need to be accessed.
	void number();
	
	// Redirect an edge from src->dest to be src->new_dest.
	// void redirect(BlockIt src, BlockIt dest, BlockIt new_dest, simple_sym *new_lbl);
	
	// This does all the things.
	// Create the CFG.
	// Also numbers, generates succs at the end, and calculates isReachable.
	void run();
	
	// Print it out like in A2 to stdout.
	// Needs numbered blocks and succs.
	void print(char const *proc_name);
};

#endif /* CFG_H */
