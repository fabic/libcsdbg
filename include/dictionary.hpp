#ifndef _CSDBG_DICTIONARY
#define _CSDBG_DICTIONARY 1

/**
	@file include/dictionary.hpp

	@brief Class csdbg::dictionary definition
*/

#include "./string.hpp"

namespace csdbg {

/**
	@brief A named collection of words (for syntax highlighters)

	A dictionary object is used to create a collection of tokens, under a common
	name. Dictionary data can be loaded from regular text files (.dict extension).
	Each non-empty line in the source file is translated as a single token. A line
	with only whitespace characters is considered an empty line. The tokens are
	trimmed to remove leading and trailing whitespace characters. If the source
	file is empty no tokens are loaded, but the dictionary object remains valid.
	The dictionary class inherits from csdbg::chain (T = csdbg::string) all its
	methods for item management. A dictionary can be looked up for literal strings
	or for POSIX extended regular expressions (with or without case sensitivity).
	If a word appears more than once, its first occurence is used. A dictionary is
	not thread safe, users must implement thread synchronization

	@see csdbg::parser
	@see <a href="index.html#sec5_7"><b>5.7 Using the stack trace parser (syntax highlighter)</b></a>
*/
class dictionary: virtual public chain<string>
{
protected:

	/* Protected variables */

	i8 *m_name;								/**< @brief Dictionary name */

	bool m_mode;							/**< @brief Lookup mode */

public:

	/* Constructors, copy constructors and destructor */

	dictionary(const i8*, const i8* = NULL, bool = false);

	dictionary(const dictionary&);

	virtual	~dictionary();

	virtual dictionary* clone() const;


	/* Accessor methods */

	virtual const i8* name() const;

	virtual bool mode() const;

	virtual dictionary& set_name(const i8*);

	virtual dictionary& set_mode(bool);


	/* Operator overloading methods */

	virtual dictionary& operator=(const dictionary&);


	/* Generic methods */

	virtual dictionary& load_file(const i8*);

	virtual const string* lookup(const string&, bool = false) const;
};

}

#endif

