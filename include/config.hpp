#ifndef _CSDBG_CONFIG
#define _CSDBG_CONFIG 1

/**
	@file include/config.hpp

	@brief Library configuration, type, macro and global variable definition
*/

#include <typeinfo>
#include <iostream>

#include <cstdarg>
#include <cstring>
#include <climits>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <cctype>


#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include <unistd.h>
#include <cxxabi.h>
#include <regex.h>
#include <link.h>
#include <bfd.h>
#include <sys/stat.h>

#ifdef CSDBG_WITH_STREAMBUF
#include <sys/time.h>
#include <sys/file.h>

#ifdef CSDBG_WITH_STREAMBUF_TCP
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

#ifdef CSDBG_WITH_STREAMBUF_STTY
#include <termios.h>
#endif
#endif

#ifdef CSDBG_WITH_HIGHLIGHT
#include <fcntl.h>
#include <sys/mman.h>
#endif

#ifdef __cplusplus
}
#endif


/**
	@brief Complete library namespace

	This namespace consists of the libcsdbg classes/templates and global variables
	(csdbg::g_*) that provide the default library configuration

	@todo Replace strerror with strerror_r
	@todo Check const variables
*/
namespace csdbg {

/**
	@brief 8-bit signed integer
*/
typedef char								i8;

/**
	@brief 16-bit signed integer
*/
typedef short								i16;

/**
	@brief 32-bit signed integer
*/
typedef int									i32;

/**
	@brief 64-bit signed integer
*/
typedef long long						i64;

/**
	@brief 8-bit unsigned integer
*/
typedef unsigned char				u8;

/**
	@brief 16-bit unsigned integer
*/
typedef unsigned short			u16;

/**
	@brief 32-bit unsigned integer
*/
typedef unsigned int				u32;

/**
	@brief 64-bit unsigned integer
*/
typedef unsigned long long	u64;

/**
	@brief File metadata
*/
typedef struct stat					fileinfo_t;


#if defined __x86_64__ || defined __ppc64__

/**
	@brief 64-bit memory address
*/
typedef unsigned long long	mem_addr_t;

#else

/**
	@brief 32-bit memory address
*/
typedef unsigned int				mem_addr_t;

#endif


#ifdef CSDBG_WITH_STREAMBUF_TCP

/**
	@brief TCP IPv4 address
*/
typedef struct sockaddr_in	tcp_addr_t;

/**
	@brief IP address
*/
typedef struct sockaddr			ip_addr_t;

#endif


#ifdef CSDBG_WITH_PLUGIN

/**
	@brief Plugin callback
*/
typedef void (*							modsym_t)(void*, void*);

#endif


#ifdef CSDBG_WITH_HIGHLIGHT

/**
	@brief VT100 terminal color
*/
typedef u8									color_t;

/**
	@brief VT100 attribute bitmask

	This type could be defined as a u8, it is defined as u16 to reserve space. The
	sizeof(csdbg::style) remains 8 bytes due to 4 byte alignment
*/
typedef u16									attrset_t;

#endif


/**
	@brief Library installation prefix
*/
static const i8 g_prefix[] = "/usr/local";

/**
	@brief DSO filtering shell variable

	@see tracer::on_dso_load
*/
static const i8 g_libs_env[] = "CSDBG_LIBS";

/**
	@brief Library version major
*/
static const u16 g_major = 1;

/**
	@brief Library version minor
*/
static const u16 g_minor = 28;

/**
	@brief Block size (allocation alignment)

	@see string::memalign
*/
static const u16 g_memblock_sz = 64;


#ifdef CSDBG_WITH_STREAMBUF_TCP

/**
	@brief LDP service port
*/
static const i32 g_ldp_port = 4242;

#endif


#ifdef CSDBG_WITH_HIGHLIGHT

/**
	@brief C++ stack trace syntax

	@see csdbg::parser
*/
static const i8 g_trace_syntax[] = "[ \t\n\r\\{\\}\\(\\)\\*&,:<>]+";

#endif
}


#ifdef CSDBG_WITH_DEBUG

/**
	@brief Error debug level
*/
#define CSDBG_DBGL_ERROR		0x01

/**
	@brief Warning debug level
*/
#define CSDBG_DBGL_WARNING	0x02

/**
	@brief Generic debug level
*/
#define CSDBG_DBGL_INFO			0x04

/**
	@brief Low debug level (only errors)
*/
#define CSDBG_DBGL_LOW			(CSDBG_DBGL_ERROR)

/**
	@brief Medium debug level (errors and warnings)
*/
#define CSDBG_DBGL_MEDIUM		(CSDBG_DBGL_LOW | CSDBG_DBGL_WARNING)

/**
	@brief High debug level (all messages)
*/
#define CSDBG_DBGL_HIGH			(CSDBG_DBGL_MEDIUM | CSDBG_DBGL_INFO)

/**
	@brief Selected debug level
*/
#define CSDBG_DBG_LEVEL			CSDBG_DBGL_HIGH

/**
	@brief Assertion macro
*/
#define __D_ASSERT(x)																				\
if (!(x)) { 																								\
	std::cerr << "assertion '" << #x << "' failed";						\
	std::cerr << "\r\non line " << std::dec << __LINE__;			\
	std::cerr << "\r\nin file '" << __FILE__ << "'";					\
	std::cerr << "\r\nin function " << __PRETTY_FUNCTION__;		\
	std::cerr << "\r\n\r\n";																	\
}

#else

#define __D_ASSERT(x)

#endif


#ifdef CSDBG_WITH_COLOR_TERM

/**
	@brief Tag color for informational messages
*/
#define INFO_TAG_FG					61

/**
	@brief Tag color for warning messages
*/
#define WARNING_TAG_FG			60

/**
	@brief Tag color for error and exception messages
*/
#define ERROR_TAG_FG				9

#endif


#ifdef CSDBG_WITH_HIGHLIGHT

/**
	@brief Highlighter color for numbers (any base)
*/
#define HLT_NUMBER_FG				208

/**
	@brief Highlighter color for C++ keywords
*/
#define HLT_KEYWORD_FG			61

/**
	@brief Highlighter color for C++ intrinsic types
*/
#define HLT_TYPE_FG					105

/**
	@brief Highlighter color for C++ files
*/
#define HLT_FILE_FG					250

/**
	@brief Highlighter color for C++ ABI scopes
*/
#define HLT_SCOPE_FG				250

/**
	@brief Highlighter color for C++ function names
*/
#define HLT_FUNCTION_FG			214

#endif


#ifdef __GNUC__

/**
	@brief Offer a hint (positive) to the pipeline branch predictor
*/
#define likely(expr)				__builtin_expect((expr), true)

/**
	@brief Offer a hint (negative) to the pipeline branch predictor
*/
#define unlikely(expr)			__builtin_expect((expr), false)

/**
	@brief Prefetch a block from memory to the cache (for read)
*/
#define precache_r(addr)		__builtin_prefetch((addr), 0, 3)

/**
	@brief Prefetch a block from memory to the cache (for write)
*/
#define precache_w(addr)		__builtin_prefetch((addr), 1, 3)

#else

#define likely(expr)				(expr)

#define unlikely(expr)			(expr)

#define precache_r(addr)

#define precache_w(addr)

#endif

#endif

