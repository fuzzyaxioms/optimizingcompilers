// basic block related functions and data structures
#ifndef BLOCK_H
#define BLOCK_H

#include "common.h"
#include "instr.h"

// Each basic block will include a list of its intructions
typedef std::list<InstrIt> InstrItList;
typedef InstrItList::iterator InstrItIt;

// Similar to instructions, I'm using a list to store the basic blocks.
class Block;

// The iterator is used to reference basic blocks.
typedef std::list<Block> BlockList;
typedef BlockList::iterator BlockIt;
typedef BlockList::const_iterator CBlockIt;

typedef std::list<BlockIt> BlockItList;
typedef BlockItList::iterator BlockItIt;
typedef BlockItList::const_iterator CBlockItIt;

// These edges will all exist independently in some list.
// And again I'll use the iterator as a reference.
class BlockEdge;
typedef std::list<BlockEdge> BlockEdgeList;
typedef BlockEdgeList::iterator BlockEdgeIt;
typedef BlockEdgeList::const_iterator CBlockEdgeIt;

typedef std::list<BlockEdgeIt> BlockEdgeItList;
typedef BlockEdgeItList::iterator BlockEdgeItIt;
typedef BlockEdgeItList::const_iterator CBlockEdgeItIt;

// Each basic block will specify the opcode of its last instruction.
// This is so you know what kind of control flow is happening after the
// block.
// 
// There are 6 main types:
// JMP_OP
// RET_OP
// BTRUE_OP
// BFALSE_OP
// MBR_OP
// other...
// 
// JMP_OP and RET_OP are treated the same way; they both only use the
// direct REG_JUMP type of edge and nothing else.
// BTRUE_OP and BFALSE_OP both use the FALL_THRU and COND_JUMP edges.
// MBR_OP uses the DEF_LABEL and REG_LABEL edges.
// Anything else uses only the FALL_THRU since they just fall through.
// Now I can rewire edges around, and detect when a fall through is not
// falling through to the right block, which I can remedy by adding a
// jump instruction.

// What kind of a control flow it is.
enum EdgeType {
	FALL_THRU,
	COND_JUMP,
	REG_JUMP,
	DEF_LABEL,
	REG_LABEL
};

// A directed edge from a basic block to another.
class BlockEdge {
public:
	BlockIt src;
	BlockIt dest;
	EdgeType type;
	
	// Make a new directed edge.
	BlockEdge(EdgeType type, BlockIt src, BlockIt dest);
};

// a basic block structure
class Block {
public:
	
	// Different kinds of successors.
	// Their usage is detailed above. No block uses all of them together.
	BlockEdgeIt reg_jump;
	BlockEdgeIt fall_thru;
	BlockEdgeIt cond_jump;
	
	BlockEdgeIt def_label;
	BlockEdgeItList reg_label;
	
	// The succs and preds below are generated from CFG.
	
	// List of successors in an easier to use format.
	BlockItList succs;
	
	// List of predecessors in an easier to use format.
	BlockItList preds;
	
	// This will be used to store the block's number. Generated when
	// needed and when blocks are changed.
	int ix;
	
	// The block's instructions
	InstrItList instrs;
	
	// Whether this is reachable from Entry.
	bool isReachable;
	
	// Completely empty block. For Entry and Exit.
	Block();
	
	// Block with a leader instruction. For normal basic blocks.
	Block(InstrIt leader);
	
	// Populates the list of instructions.
	void finish_make(int n);
};

// Compares blocks by block number - used for sorting preds and succs.
bool src_cmp(BlockEdgeIt const &a, BlockEdgeIt const &b);
bool dest_cmp(BlockEdgeIt const &a, BlockEdgeIt const &b);
bool block_cmp(BlockIt const &a, BlockIt const &b);

#endif /* BLOCK_H */
