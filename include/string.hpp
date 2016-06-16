#ifndef _CSDBG_STRING
#define _CSDBG_STRING 1

/**
	@file include/string.hpp

	@brief Class csdbg::string definition
*/

#if defined CSDBG_WITH_PLUGIN || defined CSDBG_WITH_HIGHLIGHT
#include "./chain.hpp"
#else
#include "./object.hpp"
#endif

namespace csdbg {

/**
	@brief Lightweight string buffer class (for ISO-8859-1 text)

	A string object is mainly used to create trace text. Text is easily appended
	using printf-style format strings and variable argument lists. Memory can be
	allocated in blocks (aligning) to reduce overhead when appending multiple
	small strings. It is comparable against POSIX extended regular expressions. By
	creating traces in string buffers it is easy to direct library output to any
	kind of stream (console, file, serial, network, plugin, device e.t.c). Apart
	from traces a string can be used for generic dynamic text manipulation. If the
	library is compiled with plugin support (<b>CSDBG_WITH_PLUGIN</b>) or with
	support for stack trace syntax highlighting (<b>CSDBG_WITH_HIGHLIGHT</b>) a
	string object gets equipped with a method to tokenize it using POSIX extended
	regular expressions and other advanced text processing methods. This class is
	not thread safe, the caller must implement thread sychronization

	@todo Use std::regex (C++11) class for portability
*/
class string: virtual public object
{
protected:

	/* Protected variables */

	i8 *m_data;								/**< @brief String data */

	u32 m_length;							/**< @brief Character count */

	u32 m_size;								/**< @brief Buffer size */


	/* Protected generic methods */

	virtual string& memalign(u32, bool = false);

	virtual string& format(const i8*, va_list);

public:

	/* Friend classes and functions */

	friend std::ostream& operator<<(std::ostream&, const string&);


	/* Constructors, copy constructors and destructor */

	explicit string(u32 = 0);

	string(const i8*, ...);

	string(const string&);

	virtual	~string();

	virtual string* clone() const;


	/* Accessor methods */

	virtual	const i8* cstr() const;

	virtual	u32 length() const;

	virtual	u32 bufsize() const;

	virtual i8& at(u32);

	virtual	string& set(const i8*, ...);

	virtual string& set(const string&);


	/* Operator overloading methods */

	virtual string& operator=(const string&);

	virtual string& operator+=(const string&);

	virtual i8& operator[](u32);


	/* Generic methods */

	virtual u32 available() const;

	virtual string& shred(u8 = 0);

	virtual string& clear();

	virtual string& append(const string&);

	virtual string& append(const i8*, ...);

	virtual string& append(i8);

	virtual i32 cmp(const string&, bool = false) const;

	virtual bool match(const string&, bool = false) const;

#if defined CSDBG_WITH_PLUGIN || defined CSDBG_WITH_HIGHLIGHT
	virtual string& trim(i32 = 0);

	virtual string& insert(u32, const string&);

	virtual string& insert(u32, const i8*, ...);

	virtual chain<string>* split(const string&, bool = true, bool = false) const;
#endif
};

}

#endif

