#pragma once

#include <vector>
#include <sfserialization/Serialization.h>

/**
 * Types which represent the parsed tree of a C++-File
 * Note: this represents only a very small set of whats possible
 * in C++ but enough to get the generator working.
 */

enum Visibility { Public, Private, Protected };
const char * toString (Visibility v);

/// Base class for all elements inside the parsing tree
struct StackElement {
	enum Type {
		Root,		///< Root element
		Namespace,	///< A namespace block namespace bla { .. }
		Class,		///< A class/struct block class
		Function,	///< A function block
		Enum,		///< A enum block
		Code,		///< A code block { .. }
		Unknown,	///< Unknown block
	};

	StackElement (Type t) : type(t) {}
	virtual ~StackElement (){}

	Type type;											///< Type of stack element
	std::string name;									///< Name of element (e.g. namespace name or class name)
	typedef std::vector<StackElement*> ChildrenVec;
	ChildrenVec children;								///< Tree children
	typedef std::set<std::string> CommandSet;

	/// Commands added by SF_AUTOREFLECT_CMDNAME or SF_AUTOREFLECT_CMDNAME(TypeName)
	/// (The prefix can also be changed using setCommandPrefix (..) )
	CommandSet commands;

	virtual void serialize (sf::Serialization & s) const {
		s ("name", name);
		s ("type", type);
		if (!commands.empty()) s ("commands", commands);
		if (!children.empty()) s ("children", children);
	}

	/// Finds a child with given name; returns 0 if none found
	StackElement * findDirectChild (const std::string & name){
		for (ChildrenVec::iterator i = children.begin(); i != children.end(); i++){
			if ((*i)->name == name) return *i;
		}
		return 0;
	}

	/// Finds a child with qualified name (e.g. ns::Bla)
	StackElement * findChild (const std::string & name) {
		size_t x = name.find ("::");
		if (x == name.npos){
			return findDirectChild (name);
		} else {
			std::string ns  = name.substr (0, x);
			std::string ext = name.substr (x + 2, name.npos);
			StackElement * e = findDirectChild (ns);
			if (!e) return 0;
			return e->findChild (ext);
		}
	}

	/// collects all commands, also from sub elements
	CommandSet subCommands () const {
		CommandSet result = commands;
		for (ChildrenVec::const_iterator i = children.begin(); i != children.end(); i++) {
			CommandSet x = (*i)->subCommands();
			for (CommandSet::const_iterator j = x.begin(); j != x.end(); j++) {
				result.insert(*j);
			}
		}
		return result;
	}

private:
	friend class StructureParser;
	/// Delete stack element and all its descendants
	void deleteTree () {
		for (ChildrenVec::iterator i = children.begin(); i != children.end(); i++){
			(*i)->deleteTree();
		}
		delete this;
	}
};
const char * toString (StackElement::Type v);


struct RootElement : public StackElement {
	RootElement () : StackElement (Root) {}

	virtual void serialize (sf::Serialization & s) const {
		StackElement::serialize (s);
	}

};

struct NamespaceElement : public StackElement {
	NamespaceElement () : StackElement (Namespace) {}

	virtual void serialize (sf::Serialization & s) const {
		StackElement::serialize (s);
	}

};

/// A type (including const, static, etc.) as it is used in member variable definitions
struct CppType {
	CppType () : const_(false), static_(false), mutable_(false) {}
	bool const_;
	bool static_;
	bool mutable_;
	std::string name;

	void serialize (sf::Serialization & s) const {
		if (const_)        s ("const", const_);
		if (static_)       s ("static", static_);
		if (mutable_)      s ("mutable", mutable_);
		if (!name.empty()) s ("name", name);
	}

};

/// A (member-) variable definition
struct VariableDefinition {
	CppType type;		///< Type of the variable
	std::string name;	///< Name of the variable

	void serialize (sf::Serialization & s) const {
		s("type", type);
		s("name", name);
	}

};

struct ClassElement : public StackElement {
	ClassElement () : StackElement (Class), isStruct (false) {}
	bool isStruct;  				///< Its just a struct
	Visibility currentVisibility;	///< Current visibility (struct by default public, class: private)
	typedef std::pair<Visibility, std::string> Parent;
	typedef std::pair<Visibility, VariableDefinition> MemberVariable;
	typedef std::vector<Parent> ParentVec;
	typedef std::vector<MemberVariable> MemberVariableVec;
	ParentVec parents;
	MemberVariableVec memberVariables;

	virtual void serialize (sf::Serialization & s) const {
		StackElement::serialize (s);
		if (isStruct) s ("isStruct", isStruct);
		s ("parents", parents);
		s ("memberVariables", memberVariables);
	}

};

struct FunctionElement : public StackElement {
	FunctionElement () : StackElement (Function) {}
};

struct EnumElement : public StackElement {
	EnumElement () : StackElement (Enum) {}
	typedef std::pair<std::string, std::string> Value;	///< A value of an enum (name and initializer)
	typedef std::vector<Value> ValueVec;
	ValueVec values;

	virtual void serialize (sf::Serialization & s) const {
		StackElement::serialize (s);
		Value x;
		s ("values", values);
	}

};

struct CodeElement : public StackElement {
	CodeElement () : StackElement (Code) {}

	virtual void serialize (sf::Serialization & s) const {
		StackElement::serialize (s);
	}

};

struct UnknownBlock : public StackElement {
	UnknownBlock () : StackElement (Unknown) {}

	virtual void serialize (sf::Serialization & s) const {
		StackElement::serialize (s);
	}
};

