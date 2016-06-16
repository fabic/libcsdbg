#ifndef _CSDBG_UTIL
#define _CSDBG_UTIL 1

/**
	@file include/util.hpp

	@brief Class csdbg::util definition
*/

#include "./string.hpp"
#if !defined CSDBG_WITH_PLUGIN && !defined CSDBG_WITH_HIGHLIGHT
#include "./chain.hpp"
#endif

namespace csdbg {

/**
	@brief This class provides various low level utility and portability methods
*/
class util: virtual public object
{
protected:

	/* Protected static variables */

	static pthread_mutex_t m_lock;				/**< @brief Global access mutex */

	static chain<string> *m_config;				/**< @brief Runtime configuration */


	/* Protected static methods */

	static void on_lib_load()	__attribute((constructor));

	static void on_lib_unload()	__attribute((destructor));

public:

	/* Generic methods */

	/* Library compile time and runtime configuration handling methods */

	static void version(u16*, u16*);

	static const i8* prefix();

	static const i8* exec_path();

	static chain<string>* getenv(const i8*);

	static void init(i32&, i8**);

	static u32 argc();

	static const string* argv(u32);

	static const i8* type_name(const std::type_info&);


	/* Various methods (with portability issues) */

	static u32 min(u32, u32, u32);

	static void* memset(void*, u8, u32);

	static void* memcpy(void*, const void*, u32);

	static void* memswap(void*, u32);

	static void lock();

	static void unlock();

	static bool is_regular(const fileinfo_t&);

	static bool is_chardev(const fileinfo_t&);

	static bool is_readable(const fileinfo_t&);

	static bool is_writable(const fileinfo_t&);


	/* Output and debug methods */

	static i32 va_size(const i8*, va_list);

	static i8* va_format(const i8*, va_list);

	static i8* va_format(i8*, const i8*, va_list);

	static void header(std::ostream&, const i8*);

	static void dbg(const i8*, const i8*, va_list);

	static void dbg_info(const i8*, ...);

	static void dbg_warn(const i8*, ...);

	static void dbg_error(const i8*, ...);
};

}

#endif

