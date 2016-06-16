#ifndef _CSDBG_TCPSOCKBUF
#define _CSDBG_TCPSOCKBUF 1

/**
	@file include/tcpsockbuf.hpp

	@brief Class csdbg::tcpsockbuf definition
*/

#include "./streambuf.hpp"

namespace csdbg {

/**
	@brief A buffered TCP/IP socket output stream

	A tcpsockbuf object is a buffered TCP/IP client socket, designed specifically
	to implement the client side of LDP, or any other unidirectional application
	protocol (write only). The class currently supports only IPv4 addresses. This
	class is not thread safe, the caller must implement thread synchronization,
	nevertheless basic stream locking methods are inherited from csdbg::streambuf

	@see <a href="index.html#sec5_4"><b>5.4 LDP (Libcsdbg Debug Protocol)</b></a>
	@see <a href="index.html#sec5_5_2"><b>5.5.2 Using csdbg::tcpsockbuf</b></a>

	@todo Add domain name lookup (getaddrinfo will also resolve IPv4 vs IPv6)
	@todo Implement connection drop detection (SO_KEEPALIVE, SIGPIPE)
	@todo Fine tune socket options (buffer size, linger, no-delay e.t.c)

	@test Stream locking
	@test TCP_NODELAY option or other means to flush cached network data
	@test Exploit shutdown on close
*/
class tcpsockbuf: virtual public streambuf
{
protected:

	/* Protected variables */

	i8 *m_address;							/**< @brief Peer IP address (numerical, IPv4) */

	i32 m_port;									/**< @brief Peer TCP port */

public:

	/* Constructors, copy constructors and destructor */

	explicit tcpsockbuf(const i8*, i32 = g_ldp_port);

	tcpsockbuf(const tcpsockbuf&);

	virtual ~tcpsockbuf();

	virtual tcpsockbuf* clone() const;


	/* Accessor methods */

	virtual const i8* address() const;

	virtual i32 port() const;

	virtual bool is_connected() const;


	/* Operator overloading methods */

	virtual tcpsockbuf& operator=(const tcpsockbuf&);


	/* Generic methods */

	virtual tcpsockbuf& open();

	virtual tcpsockbuf& flush();

	virtual tcpsockbuf& sync() const;

	virtual tcpsockbuf& set_option(i32, const void*, u32);

	virtual tcpsockbuf& shutdown(i32) const;
};

}

#endif

