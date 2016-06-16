#ifndef _CSDBG_STREAMBUF
#define _CSDBG_STREAMBUF 1

/**
	@file include/streambuf.hpp

	@brief Class csdbg::streambuf definition
*/

#include "./string.hpp"

namespace csdbg {

/**
	@brief
		This abstract class is the base for all buffered output stream types (for
		files, sockets, serial interfaces e.t.c)

	Subclassing class streambuf is the standard way to create objects that output
	trace and other data to various media. A streambuf-derived object is both a
	string buffer and an output stream for any type of media that can be accessed
	using an integer descriptor/handle. Currently, libcsdbg is shipped with three
	streambuf implementations, csdbg::filebuf for <b>files</b>, csdbg::tcpsockbuf
	for <b>TCP/IP sockets</b> and csdbg::sttybuf for <b>serial interfaces</b>.
	Class streambuf is not thread safe, but it implements basic stream locking.
	The buffer part of the object can be manipulated using the methods inherited
	from csdbg::string. For example if you need to copy only the buffer from one
	object to another (even of different types) use the string::set(const string&)
	method instead of the overloaded assignment operator

	@see <a href="index.html#sec5_4"><b>5.4 LDP (Libcsdbg Debug Protocol)</b></a>
	@see <a href="index.html#sec5_5"><b>5.5 Buffered output streams</b></a>

	@todo <b style="color: #ff0000">[ ? ]</b> Implement the udpsockbuf subclass
	@todo <b style="color: #ff0000">[ ? ]</b> Add method try_lock
*/
class streambuf: virtual public string
{
protected:

	/* Protected variables */

	i32 m_handle;										/**< @brief Stream handle (descriptor) */

public:

	/* Constructors, copy constructors and destructor */

	streambuf();

	streambuf(const streambuf&);

	virtual ~streambuf() = 0;									/**< @brief To be implemented */

	virtual streambuf* clone() const = 0;			/**< @brief To be implemented */


	/* Accessor methods */

	virtual i32 handle() const;

	virtual bool is_opened() const;


	/* Operator overloading methods */

	virtual streambuf& operator=(const streambuf&);


	/* Generic methods */

	virtual streambuf& open() = 0;						/**< @brief To be implemented */

	virtual streambuf& close();

	virtual streambuf& flush() = 0;						/**< @brief To be implemented */

	virtual streambuf& sync() const = 0;			/**< @brief To be implemented */

	virtual streambuf& lock() const;

	virtual streambuf& unlock() const;

	virtual streambuf& header();
};

}

#endif

