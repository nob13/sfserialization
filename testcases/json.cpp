#include <sfserialization/JSONParser.h>
#include "test.h"

/// checks a string wether it detects some errors, result = true means the string should be parsed correctly
void checkParser (const char * code, bool result = true) {
	printf ("Testing %s expected: %d\n", code, result);
	sf::json::Object parser (code);
	if (parser.error() == result){
		if (parser.error()){
			printf ("Could not parse %s error = %s (this error was expected!)\n", code, parser.errorMessage().c_str());
		}
		else{
			tassert (false, "Parser returned no error while it should have");
		}
	}
}

/// Tests correctness of object-fetching
template <class T> void checkField (const sf::json::Object & parser, const char * name, T t, bool shallExist = true){
	T x;
	bool retValue = parser.get(name).fetch(x);
	tassert (retValue == shallExist, "Existing mismatch");
	tassert ((x == t), "Return value is different to created value");
}

/// Tests correctness of array-fetching
template <class T> void checkField (const sf::json::Array & array, int id, T t, bool shallExist = true){
	T x;
	bool retValue = array.get(id).fetch(x);
	tassert (retValue == shallExist, "Existing mismatch");
	tassert ((x == t), "Return value is different to created value");
}


void jsonParserTest () {
	// shall pass
	const char * testEmpty = "{}";
	checkParser (testEmpty);
	// shall fail
	const char * testAdditional = "{}   \tBlgegjelgj}";
	checkParser (testAdditional, false);
	// shall pass
	const char * testString = "{\"name\":\"nosc\",\n\"Dude\":\"Yes, you said\\\"Dude?\\\"\",,,}";
	checkParser (testString);
	// shall fail
	const char * wrongBegin = "[{}]";
	checkParser (wrongBegin, false);
	// shall pass
	const char * allInArray = "{\"a\":[\"String\",1, 0.2, null, 0.5e-23, -0.23e24, [2,3], {\"x\", []}]}";
	checkParser (allInArray);
	{
		// shall fail
		const char * tooEarlyEnding1 = "{\"a\": {\"b\":{\"c\":5}}";
		const char * tooMuchEnding1  = "{\"a\": {\"b\"}}}";
		const char * tooEarlyEnding2 = "{\"a\": [b,[c,[d[e[}";
		const char * tooMuchEnding2  = "{\"a\": [b,[c,d]]]}";
		checkParser (tooEarlyEnding1, false);
		checkParser (tooMuchEnding1, false);
		checkParser (tooEarlyEnding2, false);
		checkParser (tooMuchEnding2, false);
	}
	{
		const char * test1 = 
			"{\"name\" : \"nosc\","
			"\"hardstring\":\"hard\\tTo \\\"parse\\\"\","
			"\"number1\": 13,"
			"\"number2\":-23.49e39, "
			"\"arraytype\" : [3,5,2,-34,\"text\", "
				"{\"id\":null\t"
			    ", \"stringtype\":\"this is a nice string with heavy quotes \\\"}]\"}],"
			"\"booleantype\": true,"
			"\"booleantype2\" : false,"
			"\"nulltype\" : null,"
			"\"objecttype\" : { \"a\":5, \"b\" : \"string\" }"
			"			}";
		checkParser (test1);
		sf::json::Object parser (test1);
		tassert ((!parser.error()), "Should not give an error");

    tassert(parser.get("name").str() == "\"nosc\"");
    tassert(parser.get("number1").str() == "13");
    tassert(parser.get("arraytype").str() == "[3,5,2,-34,\"text\", {\"id\":null\t, \"stringtype\":\"this is a nice string with heavy quotes \\\"}]\"}]");

		checkField (parser, "number1", 13);
		checkField (parser, "number2", -23.49e39);
		checkField (parser, "booleantype", true);
		checkField (parser, "booleantype2", false);
		checkField (parser, "name", std::string ("nosc"));
		
		tassert ((parser.get("nulltype").type()  == sf::json::NullType), "Wrong type");
		tassert ((parser.get("arraytype").type() == sf::json::ArrayType), "Wrong type");
		
		std::string withEscapes;
		bool ret = parser.get("hardstring").fetch (withEscapes, true);
		tassert (ret);
		tassert (withEscapes == "hard\tTo \"parse\"");
		
		// sub object
		sf::json::Object subParser;
		ret = parser.get("objecttype").fetch (subParser);
		tassert (ret);
		checkField (subParser, "a", 5);
		checkField (subParser, "b", std::string ("string"));

		// the array
		sf::json::Array arr;
		ret = parser.get("arraytype").fetch(arr);
		tassert (ret);
		checkField (arr, 0, 3);
		checkField (arr, 1, 5);
		checkField (arr, 3, -34);
		checkField (arr, 4, std::string ("text"));
		tassert ((arr.get(5).type() == sf::json::ObjectType), "Wrong type");


	}
	{
		// Empty objects
		const char * emptyObjects = "{\"emptySubObject\" : {}, \"emptyVector\" : [], \"emptyString\" : \"\"}";
		sf::json::Object parser (emptyObjects);
		tassert ((!parser.error()), "Should not give an error");
		
		// sub object
		sf::json::Object subObject;
		parser.get("emptySubObject").fetch(subObject);
		tassert (!subObject.error());
		tassert (subObject.entryCount() == 0);
		
		// empty vector
		sf::json::Array emptyVector;
		parser.get ("emptyVector").fetch(emptyVector);
		tassert (!emptyVector.error());
		tassert (emptyVector.count() == 0);
	
		// empty string
		sf::json::Value emptyString;
		std::string s;
		bool suc = parser.get ("emptyString").fetch(s);
		tassert (suc);
		tassert (s.empty());

		tassert(emptyString.str() == "");
	}

	{
		// test command parser
		std::string test = "MyCommand{\"arg\":\"ument\"}";
		std::string test2= "  XtraLong\n{\"x\":5}";
		std::string cmd;
		{
			sf::json::Object o (test.c_str(), cmd);
			tassert (cmd == "MyCommand");
			checkField (o, "arg", std::string("ument"));
		}
		{
			sf::json::Object o (test2.c_str(), cmd);
			tassert (cmd == "XtraLong");
			checkField (o, "x", 5);
		}
	}
}

int main (int argc, char * argv[]){
	jsonParserTest ();
	return 0;
}
