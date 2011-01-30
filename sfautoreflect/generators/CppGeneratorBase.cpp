#include "CppGeneratorBase.h"
#include <ctype.h>
#include <stdio.h>
#include <sstream>
#include "StaticHashTableBuilder.h"

CppGeneratorBase::CppGeneratorBase () {
	mOutput = stdout;
	mOwnFile = false;
}

CppGeneratorBase::~CppGeneratorBase() {
	if (mOwnFile)
		fclose (mOutput);
}


void CppGeneratorBase::setOutput(FILE * output) {
	mOutput = output;
}

bool CppGeneratorBase::generate (const RootElement * tree) {
	bool ret = recGenerate (tree);
	if (!ret) return false;
	fprintf (mOutput, "\n");
	return true;
}

bool CppGeneratorBase::recGenerate (const StackElement * element) {
	switch (element->type){
		case StackElement::Namespace: {
			if (!handleNamespaceUp (static_cast<const NamespaceElement*> (element))) return false;
		}
		break;
		case StackElement::Class: {
			if (!handleClassUp (static_cast<const ClassElement*> (element))) return false;
		}
		case StackElement::Enum: {
			if (!handleEnumUp (static_cast<const EnumElement*> (element))) return false;
		}
		default:
			break;
	}

	for (StackElement::ChildrenVec::const_iterator i = element->children.begin(); i != element->children.end(); i++){
		bool v = recGenerate (*i);
		if (!v) return false;
	}

	switch (element->type){
		case StackElement::Namespace: {
			if (!handleNamespaceDown (static_cast<const NamespaceElement*> (element))) return false;
		}
		break;
		case StackElement::Class: {
			if (!handleClassDown (static_cast<const ClassElement*> (element))) return false;
		}
		case StackElement::Enum: {
			if (!handleEnumDown (static_cast<const EnumElement*> (element))) return false;
		}
		default:
			break;
	}
	return true;
}

bool CppGeneratorBase::handleNamespaceUp (const NamespaceElement * e) {
	fprintf (mOutput, "namespace %s {\n", e->name.c_str());
	return true;
}

bool CppGeneratorBase::handleNamespaceDown (const NamespaceElement * e) {
	fprintf (mOutput, "} // namespace %s\n", e->name.c_str());
	return true;
}

bool CppGeneratorBase::handleEnumUp (const EnumElement * e) {
	return true;
}

bool CppGeneratorBase::handleEnumDown (const EnumElement * e) {
	return true;
}

bool CppGeneratorBase::handleClassUp (const ClassElement * e) {
	mClassScope.push_back (e->name);
	return true;
}

bool CppGeneratorBase::handleClassDown (const ClassElement * e) {
	mClassScope.pop_back();
	return true;
}

std::string CppGeneratorBase::classScope () {
	if (mClassScope.empty()){
		return "";
	}
	std::ostringstream stream;
	stream << mClassScope[0] << "::";
	for (std::vector<std::string>::const_iterator i = mClassScope.begin() + 1; i != mClassScope.end(); i++){
		stream << *i << "::";
	}
	return stream.str();
}
