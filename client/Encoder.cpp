#include "stdinc.h"
#include "DCPlusPlus.h"

#include "Encoder.h"

const int8_t Encoder::base32Table[] = {
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,26,27,28,29,30,31,-1,-1,-1,-1,-1,-1,-1,-1,
		-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
		15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
		-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
		15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
};

const char Encoder::base32Alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

string Encoder::toBase32(const u_int8_t* src, size_t len) {
	// Code snagged from the bitzi bitcollider
	size_t i, index;
	u_int8_t word;
	string dst;
	dst.reserve(((len * 8) / 5) + 1);

	for(i = 0, index = 0; i < len;) {
		/* Is the current word going to span a byte boundary? */
		if (index > 3) {
			word = (src[i] & (0xFF >> index));
			index = (index + 5) % 8;
			word <<= index;
			if ((i + 1) < len)
				word |= src[i + 1] >> (8 - index);

			i++;
		} else {
			word = (src[i] >> (8 - (index + 5))) & 0x1F;
			index = (index + 5) % 8;
			if (index == 0)
				i++;
		}

		dcassert(word < 32);
		dst += base32Alphabet[word];
	}
	return dst;
}

void Encoder::fromBase32(const char* src, u_int8_t* dst, size_t len) {
	size_t i, index, offset;

	memset(dst, 0, len);
	for(i = 0, index = 0, offset = 0; src[i]; i++) {
		// Skip what we don't recognise
		int8_t tmp = base32Table[src[i]];

		if(tmp == -1)
			continue;

		if (index <= 3) {
			index = (index + 5) % 8;
			if (index == 0) {
				dst[offset] |= tmp;
				offset++;
				if(offset == len)
					break;
			} else {
				dst[offset] |= tmp << (8 - index);
			}
		} else {
			index = (index + 5) % 8;
			dst[offset] |= (tmp >> index);
			offset++;
			if(offset == len)
				break;
			dst[offset] |= tmp << (8 - index);
		}
	}
}

/**
 * @file
 * $Id: Encoder.cpp,v 1.1 2004/01/28 20:19:20 arnetheduck Exp $
 */
