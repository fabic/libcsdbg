#ifndef _CSDBG_FILEBUF
#define _CSDBG_FILEBUF 1

/**
	@file include/filebuf.hpp

	@brief Class csdbg::filebuf definition
*/

#include "./streambuf.hpp"

namespace csdbg {

/**
	@brief A buffered file output stream

	A filebuf object is a buffered output stream used to output LDP and generic
	data to a file. Based on the unique identifiers of the instrumented process, a
	filebuf object can assign file names in an unambiguous way. The class is not
	thread safe, the caller must implement thread synchronization, nevertheless
	basic file locking methods are inherited from csdbg::streambuf

	@note Methods seek_to and resize are not const in case mmap is used

	@see <a href="index.html#sec5_4"><b>5.4 LDP (Libcsdbg Debug Protocol)</b></a>
	@see <a href="index.html#sec5_5_1"><b>5.5.1 Using csdbg::filebuf</b></a>

	@todo Recode unique_id to return a new filebuf object
	@todo Add a specifier for a fixed length random string to unique_id
*/
class filebuf: virtual public streambuf
{
protected:

	/* Protected variables */

	i8 *m_path;										/**< @brief Output file path */

public:

	/* Constructors, copy constructors and destructor */

	explicit filebuf(const i8*);

	filebuf(const filebuf&);

	virtual ~filebuf();

	virtual filebuf* clone() const;


	/* Accessor methods */

	virtual const i8* path() const;


	/* Operator overloading methods */

	virtual filebuf& operator=(const filebuf&);


	/* Generic methods */

	virtual filebuf& open();

	virtual filebuf& open(u32, u32);

	virtual filebuf& flush();

	virtual filebuf& sync() const;

	virtual filebuf& sync(bool) const;

	virtual filebuf& seek_to(i32, bool = false);

	virtual filebuf& resize(u32);

	static string* unique_id(const i8*);
};

}

#endif

