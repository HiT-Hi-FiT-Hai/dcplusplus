#include "stdinc.h"
#include "DCPlusPlus.h"

#include "HashBloom.h"

namespace dcpp {

size_t HashBloom::get_k(size_t n) {
	for(size_t k = TTHValue::SIZE/3; k > 1; --k) {
		uint64_t m = get_m(n, k);
		if(m >> 24 == 0) {
			return k;
		}
	}
	return 1;
}

uint64_t HashBloom::get_m(size_t n, size_t k) {
	uint64_t m = (static_cast<uint64_t>(ceil(static_cast<double>(n) * k / log(2.))));
	return ((m / 8) + 1) * 8;
}

void HashBloom::add(const TTHValue& tth) {
	for(size_t i = 0; i < k; ++i) {
		bloom[pos(tth, i)] = true;
	}
}

bool HashBloom::match(const TTHValue& tth) const {
	if(bloom.empty()) {
		return false;
	}
	for(size_t i = 0; i < k; ++i) {
		if(!bloom[pos(tth, i)]) {
			return false;
		}
	}
	return true;
}

void HashBloom::push_back(bool v) {
	bloom.push_back(v);
}

void HashBloom::reset(size_t k_, size_t m) {
	bloom.resize(m);
	k = k_;
}

size_t HashBloom::pos(const TTHValue& tth, size_t n) const {
	uint32_t x = 0;
	for(size_t i = n*3; i < TTHValue::SIZE; i += 3*k) {
		x ^= static_cast<uint32_t>(tth.data[i]) << 2*8;
		x ^= static_cast<uint32_t>(tth.data[i+1]) << 8;
		x ^= static_cast<uint32_t>(tth.data[i+2]);
	}
	
	return x % bloom.size();
}

void HashBloom::copy_to(ByteVector& v) const {
	v.resize(bloom.size() / 8);
	for(size_t i = 0; i < bloom.size(); ++i) {
		v[i/8] |= bloom[i] << (i % 8);
	}
}

}
