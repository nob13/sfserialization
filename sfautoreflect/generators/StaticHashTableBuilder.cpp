#include "StaticHashTableBuilder.h"
#include <assert.h>

void StaticHashTableBuilder::add (const std::string & key) {
	sf::HashValue h = sf::hash ((unsigned char*) key.c_str());
	mHashes.push_back (HashPair (key, h));
}


int StaticHashTableBuilder::calcBestModulus (){
	int bestPrice  = price (1);
	int bestMod    = 1;
	for (size_t i = 2; i < 3 * mHashes.size(); i++){
		int p = price (i);
		if (p < bestPrice){
			bestPrice = p;
			bestMod   = i;
		}
	}
	return bestMod;
}

void StaticHashTableBuilder::calcHashTable (int mod, HashTable * out) {
	assert (out);
	out->clear();
	out->resize (mod);
	for (StringHashes::const_iterator i = mHashes.begin(); i != mHashes.end(); i++){
		int m = i->second % mod;
		std::string s = i->first;
		((*out)[m]).push_back (s);
	}
}


int StaticHashTableBuilder::countCollisions (int mod){
	std::vector<int> counts (mod, int());
	counts.resize(mod);
	for (StringHashes::const_iterator i = mHashes.begin(); i != mHashes.end(); i++){
		int h = i->second % mod;
		// fprintf (stderr, "i->second: %d --> %d = ", (int) i->second, (int) h);
		counts[h]++;
	}
	int result = 0;
	for (std::vector<int>::const_iterator i = counts.begin(); i != counts.end(); i++){
		if ( (*i) > 1 ) result += (*i - 1);
	}
	return result;
}
