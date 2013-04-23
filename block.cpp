#include "block.h"

BlockEdge
::BlockEdge
(EdgeType type_, BlockIt src_, BlockIt dest_)
: type(type_), src(src_), dest(dest_)
{
}

Block
::Block
()
{
}

Block
::Block
(InstrIt leader)
{
	instrs.push_back(leader);
}

void
Block
::finish_make
(int n)
{
	if (n > 0) {
		ASSERT(instrs.size() == 1);
		InstrIt cur_instr = instrs.front();
		for (int i = 1; i < n; ++i) {
			++cur_instr;
			instrs.push_back(cur_instr);
		}
	}
}

bool
src_cmp
(BlockEdgeIt const &a, BlockEdgeIt const &b)
{
	return (a->src->ix) < (b->src->ix);
}

bool
dest_cmp
(BlockEdgeIt const &a, BlockEdgeIt const &b)
{
	return (a->dest->ix) < (b->dest->ix);
}

bool
block_cmp
(BlockIt const &a, BlockIt const &b)
{
	return (a->ix) < (b->ix);
}
