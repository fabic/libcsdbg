#include "../include/sttybuf.hpp"
#include "../include/util.hpp"
#if !defined CSDBG_WITH_PLUGIN && !defined CSDBG_WITH_HIGHLIGHT
#include "../include/exception.hpp"
#endif

/**
	@file src/sttybuf.cpp

	@brief Class csdbg::sttybuf method implementation
*/

namespace csdbg {

/**
 * @brief Configure the serial interface
 *
 * @returns *this
 *
 * @throws csdbg::exception
 */
sttybuf& sttybuf::config() const
{
	__D_ASSERT(m_handle >= 0);
	if ( unlikely(m_handle < 0) )
		return const_cast<sttybuf&> (*this);

	u32 real_baud = translate_baud(m_baud);

	struct termios conf;
	util::memset(&conf, 0, sizeof(struct termios));
	conf.c_cflag = real_baud | CS8 | CRTSCTS | CREAD | CLOCAL;
	conf.c_iflag = IGNPAR;
	conf.c_cc[VMIN] = 1;

	i32 retval;
	do {
		retval = tcsetattr(m_handle, TCSANOW, &conf);
	}
	while ( unlikely(retval < 0 && errno == EINTR) );

	if ( unlikely(retval < 0) )
		throw exception(
			"failed to configure serial interface '%s' (errno %d - %s)",
			m_devnode,
			errno,
			strerror(errno)
		);

	/* Discard all pending data */
	return discard();
}


/**
 * @brief Object constructor
 *
 * @param[in] port the path of the serial interface device node
 *
 * @param[in] baud the baud rate
 *
 * @throws std::bad_alloc
 * @throws csdbg::exception
 *
 * @note The port configuration (except baud rate) is fixed to 8N1
 */
sttybuf::sttybuf(const i8 *port, u32 baud)
try:
streambuf(),
m_devnode(NULL),
m_baud(baud)
{
	if ( unlikely(port == NULL) )
		throw exception("invalid argument: port (=%p)", port);

	m_devnode = new i8[strlen(port) + 1];
	strcpy(m_devnode, port);
}

catch (...) {
	delete[] m_data;
	m_data = NULL;
	m_devnode = NULL;
}


/**
 * @brief Object copy constructor
 *
 * @param[in] src the source object
 *
 * @throws std::bad_alloc
 * @throws csdbg::exception
 */
sttybuf::sttybuf(const sttybuf &src)
try:
streambuf(src),
m_devnode(NULL),
m_baud(src.m_baud)
{
	m_devnode = new i8[strlen(src.m_devnode) + 1];
	strcpy(m_devnode, src.m_devnode);
}

catch (...) {
	close();

	delete[] m_data;
	m_data = NULL;
	m_devnode = NULL;
}


/**
 * @brief Object destructor
 */
sttybuf::~sttybuf()
{
	delete[] m_devnode;
	m_devnode = NULL;
}


/**
 * @brief Object virtual copy constructor
 *
 * @returns the object copy (heap allocated)
 *
 * @throws std::bad_alloc
 * @throws csdbg::exception
 */
inline sttybuf* sttybuf::clone() const
{
	return new sttybuf(*this);
}


/**
 * @brief Get the path of the serial interface device node
 *
 * @returns this->m_devnode
 */
inline const i8* sttybuf::devnode() const
{
	return m_devnode;
}


/**
 * @brief Get the baud rate
 *
 * @returns this->m_baud
 */
inline u32 sttybuf::baud() const
{
	return m_baud;
}


/**
 * @brief Set the baud rate
 *
 * @param[in] baud the new baud rate
 *
 * @returns *this
 *
 * @throws csdbg::exception
 */
sttybuf& sttybuf::set_baud(u32 baud)
{
	if ( unlikely(m_baud == baud) )
		return *this;

	m_baud = baud;
	if ( unlikely(m_handle < 0) )
		return *this;

	return config();
}


/**
 * @brief Assignment operator
 *
 * @param[in] rval the assigned object
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 * @throws csdbg::exception
 */
sttybuf& sttybuf::operator=(const sttybuf &rval)
{
	if ( unlikely(this == &rval) )
		return *this;

	/* Copy the buffer and duplicate the stream descriptor */
	streambuf::operator=(rval);

	u32 len = strlen(rval.m_devnode);
	if (len > strlen(m_devnode)) {
		delete[] m_devnode;
		m_devnode = NULL;
		m_devnode = new i8[len + 1];
	}

	strcpy(m_devnode, rval.m_devnode);
	return set_baud(rval.m_baud);
}


/**
 * @brief Check if the device node is a terminal
 *
 * @returns true if the node is a terminal, false otherwise
 */
inline bool sttybuf::is_tty() const
{
	if ( likely(m_handle >= 0) )
		return isatty(m_handle);

	return false;
}


/**
 * @brief Open the serial interface for output
 *
 * @returns *this
 *
 * @throws csdbg::exception
 */
inline sttybuf& sttybuf::open()
{
	return open(false);
}


/**
 * @brief Open the serial interface for output
 *
 * @param[in] ctty true to make the interface the process controlling terminal
 *
 * @returns *this
 *
 * @throws csdbg::exception
 *
 * @note If the interface is already open, it is re-opened with the new settings
 */
sttybuf& sttybuf::open(bool ctty)
{
	if ( unlikely(m_handle >= 0) )
		close();

	/* Stat the device node path and make some preliminary checks */
	fileinfo_t inf;
	i32 retval = stat(m_devnode, &inf);
	if ( unlikely(errno == ENOENT) )
		throw exception("device node '%s' does not exist", m_devnode);

	else if ( unlikely(retval < 0) )
		throw exception(
			"failed to stat path '%s' (errno %d - %s)",
			m_devnode,
			errno,
			strerror(errno)
		);

	else if ( unlikely(!util::is_chardev(inf)) )
		throw exception("'%s' is not a character device", m_devnode);

	else if ( unlikely(!util::is_writable(inf)) )
		throw exception("serial interface '%s' is not writable", m_devnode);

	u32 flags = O_WRONLY;
	if ( likely(!ctty) )
		flags |= O_NOCTTY;

	/* Open the device node */
	do {
		m_handle = ::open(m_devnode, flags);
	}
	while ( unlikely(m_handle < 0 && errno == EINTR) );

	if ( unlikely(m_handle < 0) )
		throw exception(
			"failed to open serial interface '%s' (errno %d - %s)",
			m_devnode,
			errno,
			strerror(errno)
		);

	return config();
}


/**
 * @brief Flush the buffered data to the serial interface
 *
 * @returns *this
 *
 * @throws csdbg::exception
 */
sttybuf& sttybuf::flush()
{
	try {
		streambuf::flush();
		return sync();
	}

	catch (i32 err) {
		discard();
		throw exception(
			"failed to send data to serial interface '%s' (errno %d - %s)",
			m_devnode,
			err,
			strerror(err)
		);
	}
}


/**
 * @brief Commit cached data to the serial interface line
 *
 * @returns *this
 *
 * @throws csdbg::exception
 */
sttybuf& sttybuf::sync() const
{
	i32 retval;
	do {
		retval = tcdrain(m_handle);
	}
	while ( unlikely(retval < 0 && errno == EINTR) );

	if ( unlikely(retval < 0) )
		throw exception(
			"failed to flush serial interface '%s' (errno %d - %s)",
			m_devnode,
			errno,
			strerror(errno)
		);

	return const_cast<sttybuf&> (*this);
}


/**
 * @brief Discard the data cached in the serial interface (in-kernel) buffers
 *
 * @returns *this
 */
sttybuf& sttybuf::discard() const
{
	i32 retval;
	do {
		retval = tcflush(m_handle, TCIOFLUSH);
	}
	while ( unlikely(retval < 0 && errno == EINTR) );

#if CSDBG_DBG_LEVEL & CSDBG_DBGL_WARNING
	if ( unlikely(retval < 0) )
		util::dbg_warn(
			"failed to clear the buffers of serial interface '%s' (errno %d - %s)",
			m_devnode,
			errno,
			strerror(errno)
		);
#endif

	return const_cast<sttybuf&> (*this);
}


/**
 * @brief Translate a number to a baud rate as defined in termios.h
 *
 * @param[in] rate the value to translate
 *
 * @returns the translated value
 *
 * @throws csdbg::exception
 *
 * @attention
 *	For simplicity, only a subset (most commonly used) of the values defined in
 *	termios.h are translatable
 */
u32 sttybuf::translate_baud(u32 rate)
{
	switch (rate) {
	case 9600:
		return B9600;

	case 19200:
		return B19200;

	case 38400:
		return B38400;

	case 57600:
		return B57600;

	case 115200:
		return B115200;

	case 230400:
		return B230400;

	case 460800:
		return B460800;

	default:
		throw exception("invalid argument: rate (=%d)", rate);
	}
}

}

