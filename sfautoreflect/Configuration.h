#pragma once

#include "generators/Generator.h"

/**
 * Configuration class for the sfautoreflect tool
 */
class Configuration {
public:
	typedef std::map<std::string, Generator*> GeneratorMap; ///< Available generators
	typedef std::vector<Generator*> GeneratorVec;			///< Selected generator vector

	/// Init
	Configuration ();
	~Configuration ();

	/// Parse command line arguments.
	/// And sets concrete configuration
	/// @return true on success
	bool parse (int argc, char * argv[]);

	/// There is an input file name set
	bool hasFilename () {
		return !mFilename.empty();
	}

	/// There is an output file name set
	bool hasOutputFilename () {
		return !mOutputFilename.empty();
	}

	/// Returns source file name
	const std::string & filename () const { return mFilename; }

	/// Returns output filename
	const std::string & outputFilename () const { return mOutputFilename; }

	/// Prints out mHelp info
	void printInfo ();

	/// Prints out error message (if parsing failed etc.)
	void printErr  ();

	/// predebug flag set
	bool predebug () const { return mPredebug; }

	/// debug flag is set
	bool debug () const { return mDebug; }

	/// help flag set
	bool help () const { return mHelp; }

	/// version flag set
	bool version () const { return mVersion; }

	/// available generators
	const GeneratorMap & generators () const { return mGenerators; }

	/// (user-) selected generators.
	const GeneratorVec & selectedGenerators () const { return mSelectedGenerators; }
private:
	bool mPredebug;
	bool mHelp;
	bool mDebug;
	bool mVersion;
	std::string mFilename;
	std::string mOutputFilename;
	GeneratorMap mGenerators;
	GeneratorVec mSelectedGenerators;
	GeneratorVec mDefaultGenerators;
};
