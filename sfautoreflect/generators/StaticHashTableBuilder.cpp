#include "StaticHashTableBuilder.h"
#include <assert.h>

void StaticHashTableBuilder::add (const std::string & key, const std::string & value) {
	sf::HashValue h = sf::hash ((unsigned char*) key.c_str());
	mHashes.push_back (HashPair (KeyValue (key, value), h));
}

bool StaticHashTableBuilder::generateHashCode (FILE * out, const std::string& typeName) {
	if (mHashes.empty()) return false;
	int mod = calcBestModulus();
	HashTable table;
	calcHashTable(mod, &table);

	// Lookup structure
	fprintf (out, "\t// hash table lookup structure\n");
	fprintf (out, "\tconst int hashTable[] = {0");
	{
		int current     = table[0].size() + 1; // + null element
		int nullElement = table[0].size();
		for (int i = 1; i < mod; i++) {
			if (table[i].empty()){
				fprintf (out, ", %d", nullElement); // just point to null element
			} else {
				fprintf (out, ", %d", current);     // full entry.
				current+=table[i].size() + 1;
			}
		}
		fprintf (out, "};\n");
	}

	// Table
	fprintf (out, "\t// hash table\n");
	fprintf (out, "\tstruct HashEntry { const char * name; %s value; };\n", typeName.c_str());
	fprintf (out, "\tconst HashEntry entries[] = {\n");
	bool firstOutElement = true;
	bool firstInElement  = true;
	for (HashTable::const_iterator i = table.begin(); i != table.end(); i++){
		if (!firstInElement && i->empty()) continue;
		for (std::vector<StaticHashTableBuilder::KeyValue>::const_iterator j = i->begin(); j != i->end(); j++){
			fprintf (out, "\t\t");
			if (!firstOutElement) { fprintf (out, ","); }
			firstOutElement = false;
			fprintf (out, "{\"%s\", %s}\n", j->first.c_str(), j->second.c_str());
		}
		fprintf (out, "\t\t");
		if (!firstOutElement) { fprintf (out, ","); }
		firstOutElement = false;
		fprintf (out, "{0, %s()}\n", typeName.c_str());
		firstInElement = false;
	}
	fprintf (out, "\t};\n");

	// Lookup function
	fprintf (out, "\t// lookup\n");
	fprintf (out, "\t%s value;\n", typeName.c_str());
	fprintf (out, "\tbool foundKey = false;\n");
	fprintf (out, "\tint hashCode = sf::hash ((unsigned char*) key) %% %d;\n", mod);
	fprintf (out, "\tconst HashEntry * entry = entries + hashTable[hashCode];\n");
	fprintf (out, "\twhile (entry->name != 0){\n");
	fprintf (out, "\t\tif(strcmp (entry->name, key) == 0) {value = entry->value; foundKey = true; break; }\n");
	fprintf (out, "\t\tentry++;\n");
	fprintf (out, "\t}\n");
	return true;
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
		((*out)[m]).push_back (i->first);
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
