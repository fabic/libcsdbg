#ifndef _CSDBG_STTYBUF
#define _CSDBG_STTYBUF 1

/**
	@file include/sttybuf.hpp

	@brief Class csdbg::sttybuf definition
*/

#include "./streambuf.hpp"

namespace csdbg {

/**
	@brief A buffered output stream for serial interfaces

	An sttybuf object is a buffered output stream used to output LDP or generic
	data to any type of serial interface. The interfaces are configured for 8N1
	transmition, the baud rate is configurable (throughout a session). The class
	is not thread safe, the caller must implement thread synchronization. Basic
	stream locking methods are inherited from csdbg::streambuf

	@see <a href="index.html#sec5_4"><b>5.4 LDP (Libcsdbg Debug Protocol)</b></a>
	@see <a href="index.html#sec5_5_3"><b>5.5.3 Using csdbg::sttybuf</b></a>

	@test Test all class and streambuf superclass methods
*/
class sttybuf: virtual public streambuf
{
protected:

	/* Protected variables */

	i8 *m_devnode;									/**< @brief Device node file (devfs) */

	u32 m_baud;											/**< @brief Baud rate */


	/* Protected generic methods */

	virtual sttybuf& config() const;

public:

	/* Constructors, copy constructors and destructor */

	sttybuf(const i8*, u32 = 9600);

	sttybuf(const sttybuf&);

	virtual ~sttybuf();

	virtual sttybuf* clone() const;


	/* Accessor methods */

	virtual const i8* devnode() const;

	virtual u32 baud() const;

	virtual sttybuf& set_baud(u32);


	/* Operator overloading methods */

	virtual sttybuf& operator=(const sttybuf&);


	/* Generic methods */

	virtual bool is_tty() const;

	virtual sttybuf& open();

	virtual sttybuf& open(bool);

	virtual sttybuf& flush();

	virtual sttybuf& sync() const;

	virtual sttybuf& discard() const;

	static u32 translate_baud(u32);
};

}

#endif

