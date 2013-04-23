// My own version of a bit vector set

#ifndef BITVECSET_H
#define BITVECSET_H

#include <bitset>
#include <vector>

#include "common.h"

// Try to convince std::bitset to use a single word as its unit.
#define WORD_SIZE (sizeof(uint)*8)

class BitVecSet {
public:
	// Construct the array of bitsets
	BitVecSet(ulong size);
	
	// Set all bits to true;
	void set_all();
	
	// Set all bits to false;
	void clear_all();
	
	// Set a particular bit.
	void set(ulong pos, bool b);
	
	// Get a particular bit.
	bool get(ulong pos) const;
	
	// AND another one with this one.
	void and_with(BitVecSet const &rhs);
	
	// OR another with this.
	void or_with(BitVecSet const &rhs);
	
	// SUB another from this.
	void sub_with(BitVecSet const &rhs);
	
	// Compare equality of bits.
	bool equal_to(BitVecSet const &rhs);
	
	// The default copy behaviour is fine.
	
	// Print out as 0s and 1s.
	void print(FILE *f);
	
private:
	typedef std::vector<std::bitset<WORD_SIZE> > BitVec;
	typedef BitVec::iterator BitSetIt;
	typedef BitVec::const_iterator BitSetCIt;
	// Dynamic vector of bitsets.
	BitVec _words;
	
	// Number of bits.
	ulong _size;
};

#endif /* BITVECSET_H */
