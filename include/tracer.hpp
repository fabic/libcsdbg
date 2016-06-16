#ifndef _CSDBG_TRACER
#define _CSDBG_TRACER 1

/**
	@file include/tracer.hpp

	@brief Class csdbg::tracer definition
*/

#include "./process.hpp"
#include "./string.hpp"
#ifdef CSDBG_WITH_PLUGIN
#include "./plugin.hpp"
#endif
#ifdef CSDBG_WITH_FILTER
#include "./filter.hpp"
#endif

namespace csdbg {

/**
	@brief
		A tracer object is the default interface to libcsdbg for the instrumentation
		functions and for the library user

	The public interface of the class is used by the instrumentation functions to
	create a call stack simulation for each executing thread. The library user can
	use the interface to produce and output traces for exceptions or for threads.
	The constructors of the class are protected so there is no way for the library
	user to instantiate a tracer object. The library constructor (on_lib_load)
	creates a global static tracer object to be used as interface to the library
	facilities. All public methods are thread safe
*/
class tracer: virtual public object
{
protected:

	/* Protected static variables */

	static tracer *m_iface;							/**< @brief Interface object */


	/* Protected variables */

	process *m_proc;										/**< @brief Process handle */

#ifdef CSDBG_WITH_PLUGIN
	chain<plugin> *m_plugins;						/**< @brief Instrumentation plugins */
#endif
#ifdef CSDBG_WITH_FILTER
	chain<filter> *m_filters;						/**< @brief Instrumentation filters */
#endif


	/* Protected static methods */

	static void on_lib_load() __attribute((constructor));

	static void on_lib_unload()	__attribute((destructor));

	static i32 on_dso_load(dl_phdr_info*, size_t, void*);

	static string& addr2line(string&, const i8*, mem_addr_t);


	/* Protected constructors, copy constructors and destructor */

	tracer();

	tracer(const tracer&);

	virtual	~tracer();

	virtual tracer* clone() const;


	/* Protected operator overloading methods */

	virtual tracer& operator=(const tracer&);


	/* Protected generic methods */

	virtual tracer& destroy();

public:

	/* Friend classes and functions */

	friend std::ostream& operator<<(std::ostream&, tracer&);


	/* Accessor methods */

	virtual process* proc() const;

	static tracer* interface();


	/* Generic methods */

	/* Trace producing methods */

	virtual tracer& trace(string&);

	virtual tracer& trace(string&, pthread_t) const;

	virtual tracer& unwind();

	virtual tracer& dump(string&) const;


	/* Plugin handling methods */

#ifdef CSDBG_WITH_PLUGIN
	virtual u32 plugin_count() const;

	virtual const plugin* add_plugin(const i8*, const i8* = NULL);

	virtual const plugin* add_plugin(modsym_t, modsym_t);

	virtual tracer& remove_plugin(const i8*);

	virtual tracer& remove_plugin(u32);

	virtual const plugin* get_plugin(const i8*) const;

	virtual const plugin* get_plugin(u32) const;
#endif


	/* Filter handling methods */

#ifdef CSDBG_WITH_FILTER
	virtual u32 filter_count() const;

	virtual filter* add_filter(const i8*, bool, bool = true);

	virtual tracer& remove_filter(u32);

	virtual filter* get_filter(u32) const;
#endif
};

}

#endif

