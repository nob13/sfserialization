#include "StructureParser.h"
#include <stdio.h>
#include <ctype.h>
#include <assert.h>

/// Prints out a string vec and marks the position p (for debugging purposes)
static void markPosition (StructureParser::StringVec::const_iterator begin, StructureParser::StringVec::const_iterator end, StructureParser::StringVec::const_iterator p) {
	fprintf (stderr, "Vec\n");
	for (StructureParser::StringVec::const_iterator i = begin; i != end; i++){
		fprintf (stderr, "  %s [%d]\n", i->c_str(), (int) (i == p));
	}
	fprintf (stderr, "End\n");
}


StructureParser::StructureParser () {
	mRoot = new RootElement;
	mStack.push (mRoot);
	mDebug = false;
	mCommandPrefix = "SF_AUTOREFLECT_";
}

StructureParser::~StructureParser () {
	clear ();
	delete mRoot;
}

void StructureParser::clear () {
	while (mStack.size() > 0){
		mStack.pop();
	}
	mRoot->deleteTree (); // also deleted itself
	mRoot = new RootElement;
	mStack.push (mRoot);
}

bool StructureParser::onToken (Iterator begin, Iterator end) {
	std::string s (begin, end);
	mIncomingLine.push_back (s);
	return true;
}

/// Returns the position in x where the prefix stops if it has a given prefix.
/// If it doens't have the prefix it returns x.npos
static size_t beginsWidth (const std::string & x, const std::string & prefix){
	if (x.length () < prefix.length()) return x.npos;
	if (x.substr (0, prefix.length()) == prefix) return prefix.length();
	return x.npos;
}

bool StructureParser::onSyntactic (char c) {
	// Special handling for special C++ tokens which do not necessary finish a line
	// 1. private/public/protected handling
	if (c == ':') {
		if (mIncomingLine.size() == 1){
			const std::string & last (mIncomingLine.back());
			if (last == "public" || last == "private" || last == "protected"){
				// must be inside a class definition
				if (mStack.empty()) {
					fprintf (stderr, "visibility operator outside of any class definition\n");
					return false;
				}
				StackElement * element = mStack.top();
				if (element->type != StackElement::Class){
					fprintf (stderr, "visiblity operator outside of any class definition (current=%s)\n", sf::toJSON(element).c_str());
					return false;
				}
				ClassElement * celement = static_cast<ClassElement*> (element);
				Visibility v = Public;
				if (last == "public")    v = Public;
				if (last == "private")   v = Private;
				if (last == "protected") v = Protected;
				celement->currentVisibility = v;
				mIncomingLine.pop_back(); // removing this modifer
				return true;
			}
		}
	}

	// 2. Enum Values
	if ((c == ',' || c == '}') && mStack.top ()->type == StackElement::Enum){
		// when c equals '}' we also have to call matchLine to pop the enum from the stack
		// this causes the folling if (c == ',') return calls.

		size_t size = mIncomingLine.size();
		if (size == 3 && mIncomingLine[1] == "="){
			// 2.a value with (simple) initializer
			EnumElement::Value v;
			v.first  = mIncomingLine[0];
			v.second = mIncomingLine[2];
			EnumElement * e = static_cast<EnumElement*> (mStack.top());
			e->values.push_back (v);
			mIncomingLine.clear();
			if (c == ',') return true;
		} else if (size == 1){
			// 2.b plain value
			EnumElement::Value v;
			v.first = mIncomingLine[0];
			EnumElement * e = static_cast<EnumElement*> (mStack.top());
			e->values.push_back (v);
			mIncomingLine.clear();
			if (c == ',') return true;
		} else if (size == 0){
			// 3. Allowed, Empty enum ?
			if (c == ',') return true;
		} else {
			fprintf (stderr, "(Internal?) Error on parsing enum value\n");
			return false;
		}
	}

	// Finisher of one line or block
	if (c == '{' || c == '}' || c == ';'){
		bool suc = matchLine (c);
		if (!suc) return false;
		mIncomingLine.clear();
		return true;
	}

	// Just doing it into current line
	std::string s (&c, 1);
	mIncomingLine.push_back (s);
	return true;
}

bool StructureParser::onEnd () {
	if (mStack.size() > 1){
		fprintf (stderr, "Error: Still open braces / stack elements at end of document");
		return false;
	}
	if (mStack.size() < 1){
		fprintf (stderr, "Too much elements closed");
		return false;
	}
	return true;
}

const RootElement * StructureParser::root () const {
	return mRoot;
}

/*static*/ bool StructureParser::matchLine (char finishing) {
	// Case 1. Namespace definition
	if (mIncomingLine.size() == 2 && mIncomingLine[0] == "namespace" && finishing == '{'){
		NamespaceElement * element = new NamespaceElement();
		element->name = mIncomingLine[1];
		push (element);
		return true;
	}
	// Case 2. Enum definition
	if (mIncomingLine.size() == 2 && mIncomingLine[0] == "enum" && finishing == '{'){
		EnumElement * element = new EnumElement;
		element->name = mIncomingLine[1];
		push (element);
		return true;
	}
	// Case 3. Simple Structure/Class Definition
	if (mIncomingLine.size() == 2 && (mIncomingLine[0] == "struct" || mIncomingLine[0] == "class") && finishing == '{'){
		ClassElement * element = new ClassElement;
		element->name = mIncomingLine[1];
		element->isStruct = (mIncomingLine[0] == "struct");
		element->currentVisibility = element->isStruct ? Public : Private;
		push (element);
		return true;
	}
	// Case 4. Structure/Class derived from some other type
	if (mIncomingLine.size() > 3 && (mIncomingLine[0] == "struct" || mIncomingLine[0] == "class") && mIncomingLine[2] == ":"){
		ClassElement * element = new ClassElement;
		element->name = mIncomingLine[1];
		element->isStruct = (mIncomingLine[0] == "struct");
		element->currentVisibility = element->isStruct ? Public : Private;
		StringVec::const_iterator i = mIncomingLine.begin() + 3;
		while (i != mIncomingLine.end()){
			Visibility v = element->currentVisibility;
			if (*i == "public")   { v = Public; i++; }
			if (*i == "private")  { v = Private; i++; }
			if (*i == "protected"){ v = Protected; i++;}
			bool suc;
			StringVec::const_iterator end = matchTypeName (i, mIncomingLine.end(), &suc);
			if (!suc) {
				fprintf (stderr, "Error: could not match a typename\n");
				delete element;
				return false;
			}
			ClassElement::Parent p;
			p.first  = v;
			p.second = formatType (i,end);
			element->parents.push_back (p);
			i = end;
			if (i == mIncomingLine.end()) break;
			if (*i == ",") { i++; continue; }
			fprintf (stderr, "Error: Invalid character: %s\n", i->c_str());
			delete element;
			return false;
		}
		push (element);
		return true;
	}
	// Case 5. Member Variable
	if (finishing == ';'){
		VariableDefinition definition;
		bool result = matchVariableDefinition (mIncomingLine.begin(), mIncomingLine.end(), &definition);
		if (result) {
			if (mDebug) printf ("Matched variable definition: %s\n", sf::toJSON(definition).c_str());
			StackElement * element = mStack.top();
			if (element->type == StackElement::Class){
				ClassElement * celement = static_cast<ClassElement*> (element);
				celement->memberVariables.push_back (std::make_pair(celement->currentVisibility,definition));
				return true;
			}
		}
	}
	// Case 6a: Handling of SF_AUTOREFLECT_COMMAND;
	if (finishing == ';'){
		size_t size =  mIncomingLine.size();
		if (size >= 1){
			const std::string & x (mIncomingLine.back());
			size_t prefix = beginsWidth (x, mCommandPrefix);
			if (prefix != x.npos){
				std::string cmd = x.substr (prefix, x.npos);
				assert (!mStack.empty());
				StackElement * element = mStack.top ();
				element->commands.insert (cmd);
				mIncomingLine.pop_back ();
				return true;
			}
		}
	}
	// Case 6b: handling of SF_AUTOREFLECT_COMMAND(element-name)
	if (finishing == ';'){
		size_t size = mIncomingLine.size();
		if (size >= 4){
			const std::string & x (mIncomingLine.front());
			size_t prefix = beginsWidth (x, mCommandPrefix);
			if (prefix != x.npos && mIncomingLine[1] == "(" && mIncomingLine[size-1] == ")"){
				std::string cmd = x.substr (prefix, x.npos);
				StringVec::iterator beginType = mIncomingLine.begin() + 2;
				StringVec::iterator endType   = mIncomingLine.end() - 1;
				bool suc;
				StringVec::const_iterator foundEnd = matchTypeName (beginType, endType, &suc);
				if (!suc) {
					fprintf (stderr, "Could not match a type name in %s command", cmd.c_str());
					return false;
				}
				std::string elementName = formatType (beginType, foundEnd);
				StackElement * parent = mStack.top ();
				StackElement * child = parent->findChild (elementName);
				if (child){
					child->commands.insert (cmd);
					mIncomingLine.erase(mIncomingLine.end() - 4, mIncomingLine.end());
					return true;
				} else {
					fprintf (stderr, "Did not found a element with name %s as child of element of name %s, for command %s\n", elementName.c_str(), sf::toJSON(parent).c_str(), cmd.c_str());
					return false;
				}

			}
		}
	}
	// Case 7 Function declaration
	if (finishing == ';' || finishing == '{') {
		FunctionDeclarationElement * declaration = new FunctionDeclarationElement();
		bool result = matchFunctionDeclaration (mIncomingLine.begin(), mIncomingLine.end(), declaration);
		if (result) {
			if (mDebug) printf ("Matched function declaration %s\n", sf::toJSON (declaration).c_str());
			StackElement * element = mStack.top();
			if (element->type == StackElement::Class)
				declaration->visibility = (static_cast<ClassElement*> (element))->currentVisibility;

			if (
					(element->type == StackElement::Class)
				 || (element->type == StackElement::Root)
				 || (element->type == StackElement::Namespace)){
				element->children.push_back (declaration);
			} else {
				// May happen. Regular functions are similar to be parsed like functions
				// E.g. int bla () { X x ();} // must not declare x but can also declare an instance of (struct/class) X, called x.
				// fprintf (stderr, "Matched a function declaration %s outside a class/root/namespace element, parent=%s\n", sf::toJSON (declaration).c_str(), sf::toJSON (*element).c_str());
				delete declaration;
				if (finishing != '{') // otherwise there is still a block
					return true;
			}
		}
		if (finishing != '{') // otherwise there is still a block
			return true;
	}

	// Unknown block, starting
	if (finishing == '{'){
		if (mDebug){
			printf ("Unknown block start: ");
			for (StringVec::const_iterator i = mIncomingLine.begin(); i != mIncomingLine.end(); i++){
				printf ("%s ", i->c_str());
			}
			printf ("<stop>\n");
		}

		UnknownBlock * element = new UnknownBlock ();
		push (element);
		return true;
	}
	// End of a block, ending
	if (finishing == '}'){
		bool ret = pop ();
		return ret;
	}
	// Command, ending
	if (finishing == ';'){
		if (mDebug){
			printf ("Unknown command: ");
			for (StringVec::const_iterator i = mIncomingLine.begin(); i != mIncomingLine.end(); i++){
				printf ("%s ", i->c_str());
			}
			printf ("<stop>\n");
		}
		return true;
	}
	return true;
}

/*static*/ StructureParser::StringVec::const_iterator StructureParser::matchTypeName (StringVec::const_iterator begin, StringVec::const_iterator end, bool * result) {
	if (begin == end) { *result = false; return begin; }
	int tDepth = 0; // template depth
	StringVec::const_iterator i;
	bool awaitNext = false;
	for (i = begin + 1; i != end; i++){
		if (*i == "<")  { tDepth++; continue; }
		if (*i == ">")  { tDepth--; continue; }
		if (*i == "::") { awaitNext = true; continue; } // lets wait for a next token
		if (tDepth == 0 && !awaitNext){
			*result = true;
			return i;
		}
		awaitNext = false;
	}
	*result = (tDepth == 0);
	// markPosition (begin, end, i);
	return i;
}

/*static*/ std::string StructureParser::formatType (StringVec::const_iterator begin, StringVec::const_iterator end) {
	std::ostringstream ss;
	bool noSpace = true;
	for (StringVec::const_iterator i = begin; i != end; i++){
		if (!noSpace && *i != "::") ss << " ";
		noSpace = false;
		ss << *i;
		if (*i == "::") noSpace = true;
	}
	return ss.str();
}

/*static*/ StructureParser::StringVec::const_iterator StructureParser::matchCppType (StringVec::const_iterator begin, StringVec::const_iterator end, bool * result, CppType * type) {
	int tDepth = 0;			  // template depth
	bool modifierPart = true; // currently at left side where modifiers are
	StringVec::const_iterator i = begin;
	StringVec::const_iterator j = i == end ? end : i + 1;
	StringVec::const_iterator typeBegin = end;
	bool awaitNext = false;
	for (; i != end; i++){
		j = i + 1;
		if (modifierPart){
			if (*i == "static"){
				if (type->static_){	// static can be there only once
					*result = false;
					return end;
				}
				type->static_ = true;
				continue;
			}
			if (*i == "const"){
				if (type->const_){ // const can be there only once
					*result = false;
					return end;
				}
				type->const_ = true;
				continue;
			}
			if (*i == "mutable"){
				if (type->mutable_){ // mutable can be there only once
					*result = false;
					return end;
				}
				type->mutable_ = true;
				continue;
			}

			// ignoring struct class forward definitions (hack?)
			if (*i == "struct" || *i == "class"){
				*result = false;
				return end;
			}
			// ignoring typedef
			if (*i == "typedef"){
				*result = false;
				return end;
			}

			modifierPart = false;
			typeBegin = i;
		}
		if (*i == "<") {
			tDepth++;
			continue;
		}
		if (*i == ">") {
			tDepth--;
			continue;
		}
		if (*i == "&" || *i == "*"){
			continue;
		}
		if ((j != end && *j == "::") || *i == "::" ){
			awaitNext = true;
			continue;
		}
		if (tDepth == 0 && i != typeBegin && !awaitNext){
			// ready
			break;
		}
		awaitNext = false;
	}
	if (tDepth > 0) {
		*result = false;
		return end;
	}
	if (typeBegin == end){
		*result = false;
		return end;
	}

	type->name = formatType (typeBegin, i);
	*result = true;
	return i;
}

/*static*/ bool StructureParser::isRegularIdentifier (const std::string & id){
	if (id.empty()) return false;
	if (isdigit(id[0])) return false; // identifiers may not start with numbers
	for (std::string::const_iterator i = id.begin(); i != id.end(); i++){
		if (!isalnum(*i)) return false; // may only contain alpha numerical characters
	}
	return true;
}

/*static*/ bool StructureParser::matchVariableDefinition (StringVec::const_iterator begin, StringVec::const_iterator end, VariableDefinition * definition) {
	bool suc;
	StringVec::const_iterator pos = matchCppType (begin, end, &suc, &definition->type);
	if (!suc) {
		return false;
	}
	if (pos == end || pos + 1 != end) {
		return false;
	}
	if (!isRegularIdentifier (*pos)){
		return false;
	}
	definition->name = *pos;
	return true;
}

/*static*/ StructureParser::StringVec::const_iterator StructureParser::matchArgumentDefinition (StringVec::const_iterator begin, StringVec::const_iterator end, bool * result, ArgumentDefinition * definition) {
	bool suc;
	StringVec::const_iterator pos = matchCppType (begin, end, &suc, &definition->type);
	if (!suc) {
		*result = false;
		return end;
	}
	if (*pos == "," || *pos == ")") {
		definition->name.clear();
		*result = true;
		return pos;
	}
	if (!isRegularIdentifier(*pos)){
		// wrong stop character
		*result = false;
		return end;
	}
	definition->name = *pos;
	*result = true;
	pos++;
	return pos;
}

/*static*/ bool StructureParser::matchFunctionDeclaration (StringVec::const_iterator begin, StringVec::const_iterator end, FunctionDeclarationElement * declaration) {
	// Consists of [virtual] CppType NAME ( [Argument [,Argument]*] ) [const]
	// 0. virtual/inline
	while (true) {
		if (begin == end) return false;
		if (*begin == "virtual") { declaration->virtual_ = true; begin++; continue; }
		if (*begin == "inline")  { declaration->inline_  = true; begin++; continue; }
		break;
	}
	// 1. Return Value
	bool suc;
	StringVec::const_iterator pos = matchCppType (begin, end, &suc, &declaration->returnType);
	if (!suc) {
		return false;
	}
	if (pos == end) return false;
	// 2. NAME
	if (!isRegularIdentifier (*pos)) return false;
	declaration->name = *pos;
	pos++;
	if (pos == end) return false;
	// 3. '('
	if (*pos != "(") return false;
	pos++;
	if (pos == end) return false;
	// 4. Arguments
	while (pos != end && *pos != ")") {
		ArgumentDefinition argument;
		pos = matchArgumentDefinition (pos, end, &suc, &argument);
		if (!suc)
			return false;
		declaration->arguments.push_back (argument);
		if (*pos == ",") pos++;
		else break;
	}
	// 5. ')'
	if (pos == end || *pos != ")") return false;
	pos++;
	if (pos == end) return true;
	if (*pos == "const") {
		declaration->const_ = true;
		pos++;
	}
	if (pos == end) return true;
	if (*pos == "=" && ((pos + 1) != end) && *(pos + 1) == "0"){
		declaration->abstract = true;
		pos += 2;
	}
	return pos == end;
}

void StructureParser::push (StackElement * elem) {
	if (mDebug) printf ("Pushing %s\n", sf::toJSON(elem).c_str());
	assert (mStack.size() > 0);
	StackElement * e = mStack.top ();
	e->children.push_back (elem);
	mStack.push (elem);
}

bool StructureParser::pop () {
	if (mStack.size() == 1){
		fprintf (stderr, "Cannot pop root element, too many closing braces?");
		return false;
	}
	StackElement * e = mStack.top();
	if (mDebug) printf ("Popping %s\n", sf::toJSON(e).c_str());
	mStack.pop();
	return true;
}

/// Create a string vec out of tokenized elements of all (splitted at ' ')
static StructureParser::StringVec createStringVec (const std::string & all){
	StructureParser::StringVec v;
	size_t l = 0;
	size_t c = 0;
	while ((c = all.find (' ', l)) != all.npos){
		std::string s = all.substr(l, c - l);
		v.push_back (s);
		l = c + 1;
	}
	std::string s = all.substr (l, all.npos);
	v.push_back (s);
	return v;
}

/*static*/ bool StructureParser::test_matchTypeName () {
	// Test 1
	{
		StringVec v = createStringVec ("int");
		bool result;
		StringVec::const_iterator i = matchTypeName (v.begin(), v.end(), &result);
		if (!result || i != v.end()){
			fprintf (stderr, "test_matchTypeName1 failed\n");
			return false;
		}
	}
	// Test 2 (more complex)
	{
		StringVec v = createStringVec ("std :: vector < std :: string > notInType");
		bool result;
		StringVec::const_iterator i = matchTypeName (v.begin(), v.end(), &result);
		if (!result || i != v.begin() + 8){
			fprintf (stderr, "test_matchTypeName2 failed\n");
			markPosition (v.begin(),v.end(), i);
			return false;
		}
	}
	{
		StringVec v = createStringVec ("std :: string x");
		bool result;
		StringVec::const_iterator i = matchTypeName (v.begin(), v.end(), &result);
		if (!result || i != v.begin() + 3){
			fprintf (stderr, "test_matchTypeName3 failed\n");
			markPosition (v.begin(), v.end(), i);
			return false;
		}
	}
	// Test 4 failing parsing
	{
		StringVec v = createStringVec ("std :: vector < < std :: astring > notInType");
		// missing '>'
		bool result;
		StringVec::const_iterator i = matchTypeName (v.begin(), v.end(), &result);
		if (result){
			fprintf (stderr, "test_matchTypeName4 failed\n");
			return false;
		}
	}
	return true;
}

/*static*/ bool StructureParser::test_matchCppType () {
	// Test 1 (regression)
	{
		StringVec v = createStringVec ("static std :: pair < string , int > var");
		bool result;
		CppType type;
		StringVec::const_iterator i = matchCppType (v.begin(), v.end(), &result, &type);
		if (!result || i != v.begin() + 9){
			fprintf (stderr, "test_matchCppType1 failed\n");
			markPosition (v.begin(), v.end(), i);
			return false;
		}
	}
	// Test2
	{
		StringVec v = createStringVec ("void");
		bool result;
		CppType type;
		StringVec::const_iterator i = matchCppType (v.begin(), v.end(), &result, &type);
		if (!result || i != v.end()){
			fprintf (stderr, "test_matchCppType2 failed\n");
			markPosition (v.begin(), v.end(), i);
			return false;
		}
	}
	// Test 3 with reference
	{
		StringVec v = createStringVec ("std :: string &");
		bool result;
		CppType type;
		StringVec::const_iterator i = matchCppType (v.begin(), v.end(), &result, &type);
		if (!result || i != v.end()){
			fprintf (stderr, "test_matchCppType3 failed: %d %s\n", result, sf::toJSON (type).c_str());
			markPosition (v.begin(), v.end(), i);
			return false;
		}
	}
	// Test 4 const reference
	{
		StringVec v = createStringVec ("const std :: string & bla");
		bool result;
		CppType type;
		StringVec::const_iterator i = matchCppType (v.begin(), v.end(), &result, &type);
		if (!result || i != v.begin() + 5){
			fprintf (stderr, "test_matchCppType4 failed: %d %s\n", result, sf::toJSON (type).c_str());
			markPosition (v.begin(), v.end(), i);
			return false;
		}
	}

	return true;
}

/*static*/ bool StructureParser::test_matchVariableDefinition () {
	// Test 1 regular variable
	{
		StringVec v = createStringVec ("int a");
		VariableDefinition d;
		bool result = matchVariableDefinition (v.begin(), v.end(), &d);
		if (!result || d.name != "a" || d.type.name != "int" || d.type.const_ || d.type.static_ || d.type.mutable_){
			fprintf (stderr, "test_matchVariableDefinition1 failed\n");
			return false;
		}
	}
	// Test 2 variable with additional modifiers
	{
		StringVec v = createStringVec ("static mutable const int var");
		VariableDefinition d;
		bool result = matchVariableDefinition (v.begin(), v.end(), &d);
		if (!result || d.name != "var" || d.type.name != "int" || !d.type.const_ || !d.type.static_ || !d.type.mutable_){
			fprintf (stderr, "test_matchVariableDefinition2 failed\n");
			return false;
		}
	}
	// Test 3 complex variable type
	{
		StringVec v = createStringVec ("static std :: pair < string , int > var");
		VariableDefinition d;
		bool result = matchVariableDefinition (v.begin(), v.end(), &d);
		if (!result || d.name != "var" || d.type.name != "std::pair < string , int >" || d.type.const_ || !d.type.static_ || d.type.mutable_){
			fprintf (stderr, "test_matchVariableDefinition3 failed\n");
			return false;
		}
	}
	// Test 4 not a variable defintion but a function
	{
		StringVec v = createStringVec ("static const int getNumber ( )");
		VariableDefinition d;
		bool result = matchVariableDefinition (v.begin(), v.end(), &d);
		if (result){
			fprintf (stderr, "test_matchVariableDefinition4 failed: %s\n", sf::toJSON(d).c_str());
			return false;
		}
	}
	// Test 5 not a variable defintion but a forward definition
	{
		StringVec v = createStringVec ("class x");
		VariableDefinition d;
		bool result = matchVariableDefinition (v.begin(), v.end(), &d);
		if (result){
			fprintf (stderr, "test_matchVariableDefinition5 failed: %s\n", sf::toJSON(d).c_str());
			return false;
		}
	}
	// Test 6 not a variable defintion but a typedef
	{
		StringVec v = createStringVec ("typedef std :: vector < MyType > MyTypeVec");
		VariableDefinition d;
		bool result = matchVariableDefinition (v.begin(), v.end(), &d);
		if (result){
			fprintf (stderr, "test_matchVariableDefinition6 failed: %s\n", sf::toJSON(d).c_str());
			return false;
		}
	}
	return true;
}

/*static*/ bool StructureParser::test_matchFunctionDeclaration () {
	// Test 1 simple
	{
		StringVec v = createStringVec ("void helloWorld ( )");
		FunctionDeclarationElement element;
		bool result = matchFunctionDeclaration (v.begin(), v.end(), &element);
		if (!result || element.name != "helloWorld" || element.returnType.name != "void") {
			fprintf (stderr, "test_matchFunctionDeclaration1 failed\n");
			return false;
		}
	}
	// Test2 const method
	{
		StringVec v = createStringVec ("void helloWorld ( ) const");
		FunctionDeclarationElement element;
		bool result = matchFunctionDeclaration (v.begin(), v.end(), &element);
		if (!result || element.name != "helloWorld" || element.returnType.name != "void" || !element.const_) {
			fprintf (stderr, "test_matchFunctionDeclaration2 failed\n");
			return false;
		}
	}
	// Test3 simple argument
	{
		StringVec v = createStringVec ("void func ( int a )");
		FunctionDeclarationElement element;
		bool result = matchFunctionDeclaration (v.begin(), v.end(), &element);
		if (!result || element.name != "func" || element.returnType.name != "void" || element.const_ || element.arguments.size() != 1) {
			fprintf (stderr, "test_matchFunctionDeclaration3 failed: %s\n", sf::toJSON(element).c_str());
			return false;
		}
	}
	// Test4 two arguments
	{
		StringVec v = createStringVec ("void func ( int a , const std::string & b )");
		FunctionDeclarationElement element;
		bool result = matchFunctionDeclaration (v.begin(), v.end(), &element);
		if (!result || element.name != "func" || element.returnType.name != "void" || element.const_ || element.arguments.size () != 2) {
			fprintf (stderr, "test_matchFunctionDeclaration4 failed\n");
			return false;
		}
	}
	// Test5 two arguments, omitting name
	{
		StringVec v = createStringVec ("void func ( int , const std::string & )");
		FunctionDeclarationElement element;
		bool result = matchFunctionDeclaration (v.begin(), v.end(), &element);
		if (!result || element.name != "func" || element.returnType.name != "void" || element.const_ || element.arguments.size () != 2) {
			fprintf (stderr, "test_matchFunctionDeclaration5 failed\n");
			return false;
		}
	}
	// Test6 complex return value
	{
		StringVec v = createStringVec ("const std :: string < char > & func ( const int & a , std :: vector < int > & b ) const");
		FunctionDeclarationElement element;
		bool result = matchFunctionDeclaration (v.begin(), v.end(), &element);
		if (!result || element.name != "func" || element.returnType.name != "std::string < char > &" || !element.returnType.const_ || element.arguments.size() != 2
			|| element.arguments[0].type.name != "int &" || !element.arguments[0].type.const_ || element.arguments[1].type.name != "std::vector < int > &" || element.arguments[1].type.const_){
			fprintf (stderr, "test_matchFunctionDeclaration6 failed: %d %s\n", result, sf::toJSON(element).c_str());
			return false;
		}
	}
	// Test 7 a function pointer
	{
		StringVec v = createStringVec ("int ( * MyFunction ) ( )");
		FunctionDeclarationElement element;
		bool result = matchFunctionDeclaration (v.begin(), v.end(), &element);
		if (result) {
			fprintf (stderr, "test_matchFunctionDeclaration7 failed\n");
			return false;
		}
	}
	// Test 8 a c++ function pointer
	{
		StringVec v = createStringVec ("int ( MyClass :: * pointer ) ( )");
		FunctionDeclarationElement element;
		bool result = matchFunctionDeclaration (v.begin(), v.end(), &element);
		if (result) {
			fprintf (stderr, "test_matchFunctionDeclaration8 failed\n");
			return false;
		}
	}
	// Test 9 a virtual function
	{
		StringVec v = createStringVec ("virtual int bla ( )");
		FunctionDeclarationElement element;
		bool result = matchFunctionDeclaration (v.begin(), v.end(), &element);
		if (!result || !element.virtual_) {
			fprintf (stderr, "test_matchFunctionDeclaration9 failed\n");
			return false;
		}
	}
	// Test 10 a inline function
	{
		StringVec v = createStringVec ("inline int bla ( )");
		FunctionDeclarationElement element;
		bool result = matchFunctionDeclaration (v.begin(), v.end(), &element);
		if (!result || !element.inline_) {
			fprintf (stderr, "test_matchFunctionDeclaration10 failed\n");
			return false;
		}
	}
	// Test 11 an abstract function
	{
		StringVec v = createStringVec ("virtual int bla ( ) = 0");
		FunctionDeclarationElement element;
		bool result = matchFunctionDeclaration (v.begin(), v.end(), &element);
		if (!result || !element.abstract || !element.virtual_) {
			fprintf (stderr, "test_matchFunctionDeclaration11 failed\n");
			return false;
		}
	}


	return true;
}








