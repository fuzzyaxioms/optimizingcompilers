// My own version of a bit vector set

#include "bitvecset.h"

BitVecSet
::BitVecSet(ulong size)
: _size(size), _words((size / WORD_SIZE) + (size % WORD_SIZE == 0 ? 0 : 1))
{
}

void
BitVecSet
::set_all()
{
	for (BitSetIt iter = _words.begin(); iter != _words.end(); ++iter) {
		iter->set();
	}
}

void
BitVecSet
::clear_all() {
	for (BitSetIt iter = _words.begin(); iter != _words.end(); ++iter) {
		iter->reset();
	}
}

void
BitVecSet
::set(ulong pos, bool b) {
	ASSERT(pos < _size);
	// find which bitset it's in
	ulong word_pos_hi = pos / WORD_SIZE;
	ulong word_pos_lo = pos % WORD_SIZE;
	_words[word_pos_hi][word_pos_lo] = b;
}

bool
BitVecSet
::get(ulong pos) const {
	ASSERT(pos < _size);
	// find which bitset it's in
	ulong word_pos_hi = pos / WORD_SIZE;
	ulong word_pos_lo = pos % WORD_SIZE;
	return _words[word_pos_hi][word_pos_lo];
}

// AND another one with this one.
void
BitVecSet
::and_with(BitVecSet const &rhs) {
	ASSERT(_size == rhs._size);
	
	BitSetIt iter = _words.begin();
	BitSetCIt iter2 = rhs._words.begin();
	
	while (iter != _words.end()) {
		(*iter) &= (*iter2);
		++iter;
		++iter2;
	}
}

// OR another with this.
void
BitVecSet
::or_with(BitVecSet const &rhs) {
	ASSERT(_size == rhs._size);
	
	BitSetIt iter = _words.begin();
	BitSetCIt iter2 = rhs._words.begin();
	
	while (iter != _words.end()) {
		(*iter) |= (*iter2);
		++iter;
		++iter2;
	}
}

// SUB another from this.
void
BitVecSet
::sub_with(BitVecSet const &rhs) {
	ASSERT(_size == rhs._size);
	
	BitSetIt iter = _words.begin();
	BitSetCIt iter2 = rhs._words.begin();
	
	while (iter != _words.end()) {
		(*iter) &= ~(*iter2);
		++iter;
		++iter2;
	}
}

bool
BitVecSet
::equal_to(BitVecSet const &rhs) {
	ASSERT(_size == rhs._size);
	
	BitSetCIt iter = _words.begin();
	BitSetCIt iter2 = rhs._words.begin();
	
	while (iter != _words.end()) {
		if ((*iter) != (*iter2)) {
			return false;
		}
		++iter;
		++iter2;
	}
	return true;
}

void
BitVecSet
::print(FILE *f)
{
	for (ulong i = 0; i < _size; ++i) {
		fprintf(f, "%c", get(i) ? '1' : '0');
	}
}