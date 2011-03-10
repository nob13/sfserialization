#pragma once
#include <sfserialization/autoreflect.h>
#include <string>
#include <vector>
#include <stdio.h>

/// Tool class which generates a static hash table for storing strings
/// Hashing is done via sf::hash function
class StaticHashTableBuilder {
public:
	typedef std::pair< std::string, std::string  > KeyValue;
	typedef std::vector<std::vector<KeyValue> > HashTable;

	/// 1. Initialization - add values
	void add (const std::string & key, const std::string & value);

	/// 2. Automatically generate C++ code for hash table (calls calsBestModulus and calcHashTable for you)
	/// Returns true on success
	/// In-variable must be called const char * key.
	/// Out variable will be called TypeName value.
	/// Success of lookup will be stored in bool foundKey.
	bool generateHashCode (FILE * out, const std::string& typeName);

	/// Calc (guess) best modolus for current hash values
	int calcBestModulus ();

	/// Calculates the final hash table
	void calcHashTable (int mod, HashTable * out);

private:

	/// Counts number of collisions for a given modulus
	int countCollisions (int mod);

	/// Counts a weighting function for a given modulus
	inline int price (int mod, int sizeWeight = 1, int collisionWeight = 4){
		int collisions = countCollisions (mod);
		return sizeWeight * mHashes.size() + collisionWeight * collisions;
	}

	typedef std::pair<KeyValue, sf::HashValue> HashPair;		///< One KeyValue with the hash of the key
	typedef std::vector<HashPair> StringHashes;					///< Maps all strings to hashes
	StringHashes mHashes;
};
