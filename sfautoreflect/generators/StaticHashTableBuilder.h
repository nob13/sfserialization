#pragma once
#include <sfserialization/autoreflect.h>
#include <string>
#include <vector>

/// Tool class which generates a static hash table for storing strings
/// Hashing is done via sf::hash function
class StaticHashTableBuilder {
public:
	typedef std::vector<std::vector<std::string> > HashTable;

	/// 1. Initialization - add values
	void add (const std::string & key);

	/// 2. Calc (guess) best modolus for current hash values
	int calcBestModulus ();

	/// 3. Calculates the final hash table
	void calcHashTable (int mod, HashTable * out);

private:

	/// Counts number of collisions for a given modulus
	int countCollisions (int mod);

	/// Counts a weighting function for a given modulus
	inline int price (int mod, int sizeWeight = 1, int collisionWeight = 4){
		int collisions = countCollisions (mod);
		return sizeWeight * mHashes.size() + collisionWeight * collisions;
	}

	typedef std::pair<std::string, sf::HashValue> HashPair;		///< One string with its hash
	typedef std::vector<HashPair> StringHashes;					///< Maps all strings to hashes
	StringHashes mHashes;
};
