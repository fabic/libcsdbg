#include "../include/util.hpp"
#include "../include/tracer.hpp"

/**
	@file src/util.cpp

	@brief Class csdbg::util method implementation
*/

namespace csdbg {

/* Static member variable definition */

pthread_mutex_t util::m_lock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

chain<string> *util::m_config = NULL;


/**
 * @brief Library constructor
 *
 * @note If an exception occurs, the process exits
 */
void util::on_lib_load()
{
	try {
		m_config = new chain<string>;
		return;
	}

	catch (std::exception &x) {
		std::cerr << x;
	}

	exit(EXIT_FAILURE);
}


/**
 * @brief Library destructor
 */
void util::on_lib_unload()
{
	delete m_config;
	m_config = NULL;
}


/**
 * @brief Get the library version numbers
 *
 * @param[out] major the major version number
 *
 * @param[out] minor the minor (and subminor) version number
 *
 * @note Omit either version number by passing NULL
 */
void util::version(u16 *major, u16 *minor)
{
	if ( likely(major != NULL) )
		*major = g_major;

	if ( likely(minor != NULL) )
		*minor = g_minor;
}


/**
 * @brief Get the library installation prefix
 *
 * @returns csdbg::g_prefix
 */
const i8* util::prefix()
{
	return g_prefix;
}


/**
 * @brief Get the absolute path of the executable
 *
 * @returns the path (heap allocated)
 *
 * @throws std::bad_alloc
 * @throws csdbg::exception
 */
const i8* util::exec_path()
{
	/*
	 * The procfs filesystem maintains a directory for each process (/proc/pid)
	 * and a symlink therein (exe) that contains the absolute path of the process
	 * executable
	 */
	i8 path[PATH_MAX + 1];
	i32 len = snprintf(path, PATH_MAX + 1, "/proc/%d/exe", getpid());
	if ( unlikely(len < 0) )
		throw exception("snprintf failed with retval %d", len);

	i8 *retval = new i8[PATH_MAX + 1];

	/* Read the contents of the symlink */
	len = readlink(path, retval, PATH_MAX);
	if ( unlikely(len < 0) ) {
		delete[] retval;
		throw exception(
			"failed to read symlink '%s' (errno %d - %s)",
			path,
			errno,
			strerror(errno)
		);
	}

	retval[len] = '\0';
	return retval;
}


/**
 * @brief Parse a shell (environment) variable to its components
 *
 * @param[in] var the variable name
 *
 * @returns the variable components, tokenized with ':' (heap allocated)
 *
 * @throws std::bad_alloc
 * @throws csdbg::exception
 */
chain<string>* util::getenv(const i8 *var)
{
	__D_ASSERT(var != NULL);
	if ( unlikely(var == NULL) )
		return NULL;

	const i8 *val = ::getenv(var);
	if ( unlikely(val == NULL) )
		return NULL;

	chain<string> *retval = new chain<string>;
	string *token = NULL;

	try {
		token = new string;

		/* Token parsing */
		for (u32 i = 0, sz = strlen(val); likely(i < sz); i++) {
			i8 ch = val[i];

			if ( likely(ch != ':') )
				token->append(ch);

			/* New token */
			else if ( likely(token->length() > 0) ) {
				retval->add(token);
				token = NULL;
				token = new string;
			}
		}

		/* Final token */
		if ( likely(token->length() > 0) )
			retval->add(token);
		else
			delete token;

		return retval;
	}

	catch (...) {
		delete retval;
		delete token;
		throw;
	}
}


/**
 * @brief
 *	Initialize the library runtime configuration. Seek command line arguments
 *	that are related with libcsdbg (prefixed with --csdbg-) and move them from
 *	the argument vector to the runtime configuration list (util::m_config)
 *
 * @param[in,out] argc the argument count
 *
 * @param[in,out] argv the argument vector
 *
 * @note If an exception occurs, the process exits
 */
void util::init(i32 &argc, i8 **argv)
{
	__D_ASSERT(argc >= 1);
	__D_ASSERT(argv != NULL);
	if ( unlikely(argc <= 1 || argv == NULL) )
		return;

	try {
		for (i32 i = 0; likely(i < argc); i++) {
			i8 *arg = argv[i];

			/* If the argument is not libcsdbg-related */
			if ( likely(strstr(arg, "--csdbg-") != arg) )
				continue;

			if ( likely(strlen(arg) > 8) )
				m_config->add(new string(arg + 8));

			/* Remove it from the argument vector */
			for (i32 j = i; likely(j < argc); j++)
				argv[j] = argv[j + 1];

			i--;
			argc--;
		}

#if CSDBG_DBG_LEVEL & CSDBG_DBGL_INFO
		if ( unlikely(m_config->size() > 0) )
			util::dbg_info("libcsdbg runtime configuration:");

		for (u32 i = 0, sz = m_config->size(); likely(i < sz); i++)
			util::dbg_info("  arg %d: --csdbg-(%s)", i, m_config->at(i)->cstr());
#endif

		return;
	}

	catch (exception &x) {
		std::cerr << x;
	}

	catch (std::exception &x) {
		std::cerr << x;
	}

	exit(EXIT_FAILURE);
}


/**
 * @brief Get the number of CLI arguments
 *
 * @returns util::m_config->size()
 */
u32 util::argc()
{
	return m_config->size();
}


/**
 * @brief Get a CLI argument, given its offset in util::m_config
 *
 * @param[in] i the offset (mandate E[0, util::argc() - 1])
 *
 * @returns util::m_config->at(i)
 *
 * @throws csdbg::exception
 *
 * @see util::init()
 */
const string* util::argv(u32 i)
{
	return m_config->at(i);
}


/**
 * @brief Get the demangled name of a type
 *
 * @param[in] inf the type info
 *
 * @returns the demangled name (heap allocated)
 *
 * @throws std::bad_alloc
 *
 * @note If demangling failed the decorated name is returned
 */
const i8* util::type_name(const std::type_info &inf)
{
	const i8 *nm = inf.name();

	/* The abi namespace is part of libstdc++ */
	i8 *retval = abi::__cxa_demangle(nm, NULL, NULL, NULL);
	if ( likely(retval != NULL) )
		return retval;

	retval = new i8[strlen(nm) + 1];
	strcpy(retval, nm);
	return retval;
}


/**
 * @brief Get the minimum of three numbers
 *
 * @param[in] a,b,c the three operands
 *
 * @returns the minimum of the three operands
 */
u32 util::min(u32 a, u32 b, u32 c)
{
	if (a < b)
		return (a < c) ? a : c;

	return (b < c)? b : c;
}


/**
 * @brief Fill a memory block with a constant byte
 *
 * @param[in,out] mem the base address of the block
 *
 * @param[in] val the byte
 *
 * @param[in] sz the block size
 *
 * @returns the first argument
 *
 * @note This method is used for portability (in place of BSD's bzero)
 */
void* util::memset(void *mem, u8 val, u32 sz)
{
	__D_ASSERT(mem != NULL);
	if ( unlikely(mem == NULL) )
		return mem;

	u8 *p = static_cast<u8*> (mem);
	while ( likely(sz-- > 0) )
		*(p++) = val;

	return mem;
}


/**
 * @brief Copy a memory block
 *
 * @param[in,out] dst the destination base address
 *
 * @param[in] src the source base address
 *
 * @param[in] sz the block size
 *
 * @returns the first argument
 *
 * @note This method is used for portability (in place of BSD's bcopy)
 */
void* util::memcpy(void *dst, const void *src, u32 sz)
{
	__D_ASSERT(dst != NULL);
	__D_ASSERT(src != NULL);
	if ( unlikely(dst == NULL || src == NULL) )
		return dst;

	u8 *d = static_cast<u8*> (dst);
	const u8 *s = static_cast<const u8*> (src);
	while ( likely(sz-- > 0) )
		*(d++) = *(s++);

	return dst;
}


/**
 * @brief Reverse the byte order of a memory block
 *
 * @param[in,out] mem the base address of the block
 *
 * @param[in] sz the block size
 *
 * @returns the first argument
 *
 * @note Used to convert big endian data to little endian and vice versa
 */
void* util::memswap(void *mem, u32 sz)
{
	__D_ASSERT(mem != NULL);
	if ( unlikely(mem == NULL) )
		return mem;

	u8 *l = static_cast<u8*> (mem);
	u8 *r = l + sz - 1;
	while ( likely(l < r) ) {
		u8 tmp = *l;
		*(l++) = *r;
		*(r--) = tmp;
	}

	return mem;
}


/**
 * @brief Lock the global access mutex
 *
 * @note Recursive locking is supported
 */
void util::lock()
{
	pthread_mutex_lock(&m_lock);
}


/**
 * @brief Unlock the global access mutex
 */
void util::unlock()
{
	pthread_mutex_unlock(&m_lock);
}


/**
 * @brief Check if a file is a regular one
 *
 * @param[in] inf the file info
 *
 * @returns true if it is regular, false otherwise
 */
bool util::is_regular(const fileinfo_t &inf)
{
	return S_ISREG(inf.st_mode);
}


/**
 * @brief Check if a file is a character device node
 *
 * @param[in] inf the file info
 *
 * @returns true if it is a character device node, false otherwise
 */
bool util::is_chardev(const fileinfo_t &inf)
{
	return S_ISCHR(inf.st_mode);
}


/**
 * @brief Check if the process has read access to a file
 *
 * @param[in] inf the file info
 *
 * @returns true if it is readable, false otherwise
 */
bool util::is_readable(const fileinfo_t &inf)
{
	if ( likely(geteuid() == inf.st_uid && (inf.st_mode & S_IRUSR)) )
		return true;

	if ( likely(getegid() == inf.st_gid && (inf.st_mode & S_IRGRP)) )
		return true;

	return inf.st_mode & S_IROTH;
}


/**
 * @brief Check if the process has write access to a file
 *
 * @param[in] inf the file info
 *
 * @returns true if it is writable, false otherwise
 */
bool util::is_writable(const fileinfo_t &inf)
{
	if ( likely(geteuid() == inf.st_uid && (inf.st_mode & S_IWUSR)) )
		return true;

	if ( likely(getegid() == inf.st_gid && (inf.st_mode & S_IWGRP)) )
		return true;

	return inf.st_mode & S_IWOTH;
}


/**
 * @brief
 *	Compute the size of a printf-style format string expanded with the values of
 *	a variable argument list
 *
 * @param[in] fmt a printf-style format string
 *
 * @param[in] args a variable argument list (as a va_list variable)
 *
 * @returns the computed size (not including the trailing '\\0')
 *
 * @throws csdbg::exception
 */
i32 util::va_size(const i8 *fmt, va_list args)
{
	if ( unlikely(fmt == NULL) ) {
		va_end(args);
		throw exception("invalid argument: fmt (=%p)", fmt);
	}

	i32 retval = vsnprintf(NULL, 0, fmt, args);
	va_end(args);
	if ( unlikely(retval < 0) )
		throw exception("vsnprintf failed with retval %d", retval);

	return retval;
}


/**
 * @brief
 *	Format a buffer with a printf-style string expanded with the values of a
 *	variable argument list
 *
 * @param[in] fmt a printf-style format string
 *
 * @param[in] args a variable argument list (as a va_list variable)
 *
 * @returns the formatted string (heap allocated)
 *
 * @throws std::bad_alloc
 * @throws csdbg::exception
 */
i8* util::va_format(const i8 *fmt, va_list args)
{
	if ( unlikely(fmt == NULL) ) {
		va_end(args);
		throw exception("invalid argument: fmt (=%p)", fmt);
	}

	va_list cpargs;
	va_copy(cpargs, args);
	i8 *retval = NULL;

	try {
		i32 sz = va_size(fmt, cpargs);
		retval = new i8[sz + 1];

		i32 check = vsprintf(retval, fmt, args);
		if ( unlikely(check != sz) )
			throw exception("vsprintf failed with retval %d", check);

		va_end(args);
		return retval;
	}

	catch (...) {
		delete[] retval;
		va_end(args);
		throw;
	}
}


/**
 * @brief
 *	Format a buffer with a printf-style string expanded with the values of a
 *	variable argument list
 *
 * @param[out] dst
 *	the buffer to be formatted. Use util::va_size to compute the suitable buffer
 *	size to avoid memory overflows. If NULL a proper buffer is allocated
 *
 * @param[in] fmt a printf-style format string
 *
 * @param[in] args a variable argument list (as a va_list variable)
 *
 * @returns the formatted string
 *
 * @throws std::bad_alloc
 * @throws csdbg::exception
 */
i8* util::va_format(i8 *dst, const i8 *fmt, va_list args)
{
	if ( unlikely(fmt == NULL) ) {
		va_end(args);
		throw exception("invalid argument: fmt (=%p)", fmt);
	}

	if ( unlikely(dst == NULL) )
		return va_format(fmt, args);

	i32 sz = vsprintf(dst, fmt, args);
	va_end(args);
	if ( unlikely(sz < 0) )
		throw exception("vsprintf failed with retval %d", sz);

	return dst;
}


/**
 * @brief Print a tagged message header on an output stream
 *
 * @param[in] stream the output stream
 *
 * @param[in] tag the message tag
 */
void util::header(std::ostream &stream, const i8 *tag)
{
	__D_ASSERT(tag != NULL);
	if ( unlikely(tag == NULL) )
		return;

#ifdef CSDBG_WITH_COLOR_TERM
	stream << "\e[38;5;" << std::dec;

	i8 ch = tag[0];
	if ( likely(ch == 'i') )
		stream << INFO_TAG_FG;

	else if ( likely(ch == 'w') )
		stream << WARNING_TAG_FG;

	else
		stream << ERROR_TAG_FG;

	stream << "m[" << tag << "]\e[0m";
#else
	stream << "[" << tag << "]";
#endif

	stream << " [" << std::dec << getpid() << ", ";
	stream << "0x" << std::hex << pthread_self();

	const i8 *thr = NULL;
	tracer *iface = tracer::interface();
	if ( likely(iface != NULL) )
		thr = iface->proc()->current_thread()->name();

	stream << " (" << ((thr != NULL) ? thr : "anon") << ")] ";
}


/**
 * @brief Print a tagged debug message on the standard error stream
 *
 * @param[in] tag the message tag
 *
 * @param[in] fmt a printf-style format string
 *
 * @param[in] args a variable argument list (as a va_list variable)
 *
 * @note Deprecated if debugging is disabled
 */
void util::dbg(const i8 *tag, const i8 *fmt, va_list args)
{
#ifdef CSDBG_WITH_DEBUG
	__D_ASSERT(tag != NULL);
	__D_ASSERT(fmt != NULL);
	if ( unlikely(tag == NULL || fmt == NULL) ) {
		va_end(args);
		return;
	}

	i8 *msg = NULL;
	try {
		msg = va_format(fmt, args);

		lock();
		if ( likely(!isspace(fmt[0])) )
			header(std::cerr, tag);

		std::cerr << msg << "\r\n";
		delete[] msg;
		unlock();
	}

	catch (...) {
		__D_ASSERT(msg != NULL);
	}
#endif
}


/**
 * @brief Print an informational debug message on the standard error stream
 *
 * @param[in] fmt a printf-style format string
 *
 * @param[in] ... a variable argument list
 *
 * @note Deprecated if debug level is set lower than DBG_LEVEL_HIGH
 */
void util::dbg_info(const i8 *fmt, ...)
{
#if CSDBG_DBG_LEVEL & CSDBG_DBGL_INFO
	__D_ASSERT(fmt != NULL);
	if ( unlikely(fmt == NULL) )
		return;

	va_list args;
	va_start(args, fmt);
	dbg("i", fmt, args);
#endif
}


/**
 * @brief Print a warning debug message on the standard error stream
 *
 * @param[in] fmt a printf-style format string
 *
 * @param[in] ... a variable argument list
 *
 * @note Deprecated if debug level is set lower than DBG_LEVEL_MEDIUM
 */
void util::dbg_warn(const i8 *fmt, ...)
{
#if CSDBG_DBG_LEVEL & CSDBG_DBGL_WARNING
	__D_ASSERT(fmt != NULL);
	if ( unlikely(fmt == NULL) )
		return;

	va_list args;
	va_start(args, fmt);
	dbg("w", fmt, args);
#endif
}


/**
 * @brief Print an error debug message on the standard error stream
 *
 * @param[in] fmt a printf-style format string
 *
 * @param[in] ... a variable argument list
 *
 * @note Deprecated if debugging is disabled
 */
void util::dbg_error(const i8 *fmt, ...)
{
#if CSDBG_DBG_LEVEL & CSDBG_DBGL_ERROR
	__D_ASSERT(fmt != NULL);
	if ( unlikely(fmt == NULL) )
		return;

	va_list args;
	va_start(args, fmt);
	dbg("e", fmt, args);
#endif
}

}

