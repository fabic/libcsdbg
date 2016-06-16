#include "../include/tracer.hpp"
#include "../include/util.hpp"

/**
	@file src/tracer.cpp

	@brief Class csdbg::tracer method implementation
*/

namespace csdbg {

/* Static member variable definition */

tracer *tracer::m_iface = NULL;


/* Link the instrumentation functions with C-style linking */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief
 *	In code compiled with -finstrument-functions, g++ injects code to call this
 *	function at the beginning of instrumented functions. By implementing this
 *	function (and __cyg_profile_func_exit), libcsdbg simulates the call stack of
 *	each thread
 *
 * @param[in] this_fn the address of the called function
 *
 * @param[in] call_site the address where the function was called
 *
 * @note If an exception occurs, the process exits
 */
void __cyg_profile_func_enter(void *this_fn, void *call_site)
{
	__D_ASSERT(this_fn != NULL);
	__D_ASSERT(call_site != NULL);

	util::lock();
	tracer *iface = tracer::interface();

	__D_ASSERT(iface != NULL);
	if ( unlikely(iface == NULL) ) {
		util::unlock();
		return;
	}

#ifdef CSDBG_WITH_PLUGIN
	/* Call all plugin enter functions in the order they were registered */
	for (u32 i = 0, sz = iface->plugin_count(); likely(i < sz); i++)
		try {
			iface->get_plugin(i)->begin(this_fn, call_site);
		}

		catch (exception &x) {
			std::cerr << x;
		}

		catch (std::exception &x) {
			std::cerr << x;
		}

		catch (...) {
			util::header(std::cerr, "x");
			std::cerr << "plugin " << std::dec << i << ": unidentified exception\r\n";
		}
#endif

	try {
		mem_addr_t addr = reinterpret_cast<mem_addr_t> (this_fn);
		mem_addr_t site = reinterpret_cast<mem_addr_t> (call_site);
		process *proc = iface->proc();

#ifdef CSDBG_WITH_FILTER
		mem_addr_t base = 0;
		const i8 *path = proc->ilookup(addr, base);

		/* Call all the module filters in the order they were registered */
		if ( likely(path != NULL) )
			for (u32 i = 0, sz = iface->filter_count(); likely(i < sz); i++) {
				filter *filt = iface->get_filter(i);
				if ( likely(filt->mode()) )
					continue;

				if ( unlikely(filt->apply(path)) ) {
					util::unlock();
					return;
				}
			}
#endif

		/*
		 * Lookup the process namespace to resolve the called function symbol. If it
		 * gets resolved update the simulated call stack of the current thread
		 */
		const i8 *nm = proc->lookup(addr);
		if ( likely(nm != NULL) ) {
#ifdef CSDBG_WITH_FILTER
			/* Call all the symbol filters in the order they were registered */
			for (u32 i = 0, sz = iface->filter_count(); likely(i < sz); i++) {
				filter *filt = iface->get_filter(i);
				if ( likely(!filt->mode()) )
					continue;

				if ( unlikely(filt->apply(nm)) ) {
					util::unlock();
					return;
				}
			}
#endif

			proc->current_thread()->called(addr, site, nm);
		}

		util::unlock();
		return;
	}

	catch (exception &x) {
		std::cerr << x;
	}

	catch (std::exception &x) {
		std::cerr << x;
	}

	util::unlock();
	exit(EXIT_FAILURE);
}


/**
 * @brief
 *	In code compiled with -finstrument-functions, g++ injects code to call this
 *	function at the end of instrumented functions. By implementing this function
 *	(and __cyg_profile_func_enter), libcsdbg simulates the call stack of each
 *	thread
 *
 * @param[in] this_fn the address of the returning function
 *
 * @param[in] call_site the address that the program counter will return to
 *
 * @note If an exception occurs, the process exits
 */
void __cyg_profile_func_exit(void *this_fn, void *call_site)
{
	__D_ASSERT(this_fn != NULL);
	__D_ASSERT(call_site != NULL);

	util::lock();
	tracer *iface = tracer::interface();

	__D_ASSERT(iface != NULL);
	if ( unlikely(iface == NULL) ) {
		util::unlock();
		return;
	}

#ifdef CSDBG_WITH_PLUGIN
	/* Call all plugin exit functions in the reverse order they were registered */
	for (i32 i = iface->plugin_count() - 1; likely(i >= 0); i--)
		try {
			iface->get_plugin(i)->end(this_fn, call_site);
		}

		catch (exception &x) {
			std::cerr << x;
		}

		catch (std::exception &x) {
			std::cerr << x;
		}

		catch (...) {
			util::header(std::cerr, "x");
			std::cerr << "plugin " << std::dec << i << ": unidentified exception\r\n";
		}
#endif

	try {
		mem_addr_t addr = reinterpret_cast<mem_addr_t> (this_fn);
		process *proc = iface->proc();

#ifdef CSDBG_WITH_FILTER
		mem_addr_t base = 0;
		const i8 *path = proc->ilookup(addr, base);

		/* Call all the module filters in the order they were registered */
		if ( likely(path != NULL) )
			for (u32 i = 0, sz = iface->filter_count(); likely(i < sz); i++) {
				filter *filt = iface->get_filter(i);
				if ( likely(filt->mode()) )
					continue;

				if ( unlikely(filt->apply(path)) ) {
					util::unlock();
					return;
				}
			}
#endif

		/*
		 * Lookup the process namespace to resolve the returning function symbol. If
		 * it gets resolved update the simulated call stack of the current thread
		 */
		const i8 *nm = proc->lookup(addr);
		if ( likely(nm != NULL) ) {
#ifdef CSDBG_WITH_FILTER
			/* Call all the symbol filters in the order they were registered */
			for (u32 i = 0, sz = iface->filter_count(); likely(i < sz); i++) {
				filter *filt = iface->get_filter(i);
				if ( likely(!filt->mode()) )
					continue;

				if ( unlikely(filt->apply(nm)) ) {
					util::unlock();
					return;
				}
			}
#endif

			proc->current_thread()->returned();
		}

		util::unlock();
		return;
	}

	catch (exception x) {
		std::cerr << x;
	}

	catch (std::exception x) {
		std::cerr << x;
	}

	util::unlock();
	exit(EXIT_FAILURE);
}

#ifdef __cplusplus
}
#endif


/**
 * @brief Library constructor
 *
 * @note If an exception occurs, the process exits
 */
void tracer::on_lib_load()
{
	/* Initialize libbfd internals */
	bfd_init();

	try {
		m_iface = new tracer;

		/* Load the symbol table of the executable */
		const i8 *path = util::exec_path();
		m_iface->m_proc->add_module(path, 0);
		delete[] path;

		/* Load the symbol tables of the selected DSO */
		chain<string> *libs = util::getenv(g_libs_env);
		dl_iterate_phdr(on_dso_load, libs);
		delete libs;

		util::dbg_info("libcsdbg.so.%d.%d initialized", g_major, g_minor);
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
 * @brief Library destructor
 */
void tracer::on_lib_unload()
{
	delete m_iface;
	m_iface = NULL;
	util::dbg_info("libcsdbg.so.%d.%d finalized", g_major, g_minor);
}


/**
 * @brief
 *	This is a dl_iterate_phdr (libdl) callback, called for each linked shared
 *	object. It loads the symbol table of the DSO (if it's not filtered out) to
 *	tracer::m_iface->m_proc
 *
 * @param[in] dso
 *	a dl_phdr_info struct (libdl) that describes the shared object (file path,
 *	load address e.t.c)
 *
 * @param[in] sz the sizeof dso
 *
 * @param[in] arg
 *	a chain of POSIX extended regular expressions used to select the shared
 *	objects that will participate in the call stack simulation. The absolute
 *	path of each DSO is matched against each regexp. If NULL, all linked DSO
 *	symbol tables will be loaded. If not NULL but empty, all DSO are filtered
 *	out from instrumentation
 *
 * @returns 0
 *
 * @note
 *	If an exception occurs, it's caught and handled. 0 is returned, signaling to
 *	the iterator (dl_iterate_phdr) to continue with the next DSO
 */
i32 tracer::on_dso_load(dl_phdr_info *dso, size_t sz, void *arg)
{
	try {
		if ( unlikely(dso == NULL) )
			throw exception("invalid argument: dso (=%p)", dso);

		/* If the DSO path is undefined */
		string path(dso->dlpi_name);
		if ( unlikely(path.length() == 0) )
			throw exception("undefined DSO path");

		/* If the DSO has no segments */
		if ( unlikely(dso->dlpi_phnum == 0) )
			throw exception("'%s' has 0 segments", path.cstr());

		/* Check if the DSO is filtered out */
		bool found = false;
		if ( likely(arg != NULL) ) {
			chain<string> *filters = static_cast<chain<string>*> (arg);

			for (u32 i = 0, sz = filters->size(); likely(i < sz); i++) {
				string *filt = filters->at(i);
				if ( unlikely(path.match(*filt)) ) {
					found = true;
					break;
				}
			}
		}
		else
			found = true;

		if ( likely(!found) ) {
			util::dbg_warn("filtered out '%s'", path.cstr());
			return 0;
		}

		/* Load the DSO symbol table */
		mem_addr_t base = dso->dlpi_addr + dso->dlpi_phdr[0].p_vaddr;
		m_iface->m_proc->add_module(path.cstr(), base);
	}

	catch (exception &x) {
		util::dbg_error("in tracer::%s(): %s", __FUNCTION__, x.msg());
	}

	catch (std::exception &x) {
		util::dbg_error("in tracer::%s(): %s", __FUNCTION__, x.what());
	}

	return 0;
}


/**
 * @brief
 *	Given an address in an objective code file, extract from the gdb-related
 *	debug information, the equivalent source code file name and line and append
 *	it to a string buffer
 *
 * @param[in,out] dst the destination string
 *
 * @param[in] path the path of the objective code file
 *
 * @param[in] addr the address
 *
 * @returns the first argument
 *
 * @note
 *	If the addr2line program fails to retreive the debug information, or if any
 *	other error or exception occurs, nothing is appended to the destination
 *	string
 *
 * @see man addr2line
 * @see man g++ (-g family options)
 */
string& tracer::addr2line(string &dst, const i8 *path, mem_addr_t addr)
{
	__D_ASSERT(path != NULL);
	if ( unlikely(path == NULL) )
		return dst;

	FILE *pipe = NULL;
	try {
		/* Command to be executed by the child process */
		string cmd("addr2line -se %s 0x%x", path, addr);

		/* Open a readonly pipe to the child process */
		pipe = popen(cmd.cstr(), "r");
		if ( unlikely(pipe == NULL) )
			throw exception(
				"failed to open pipe for command '%s' (errno %d - %s)",
				cmd.cstr(),
				errno,
				strerror(errno)
			);

		/* Read a line of output from the pipe to a buffer */
		string buf;
		i8 ch = fgetc(pipe);
		while ( likely(ch != '\n' && ch != EOF) ) {
			buf.append(ch);
			ch = fgetc(pipe);
		}

		if ( unlikely(ferror(pipe) != 0) )
			throw exception("failed to read pipe for command '%s'", cmd.cstr());

		if ( likely(buf.cmp("??:0") != 0) )
			dst.append(" (%s)", buf.cstr());
	}

	catch (exception &x) {
		util::dbg_error("in tracer::%s(): %s", __FUNCTION__, x.msg());
	}

	catch (std::exception &x) {
		util::dbg_error("in tracer::%s(): %s", __FUNCTION__, x.what());
	}

	if ( likely(pipe != NULL) )
		pclose(pipe);

	return dst;
}


/**
 * @brief Object default constructor
 *
 * @throws std::bad_alloc
 */
tracer::tracer()
try:
m_proc(NULL)
#ifdef CSDBG_WITH_PLUGIN
,m_plugins(NULL)
#endif
#ifdef CSDBG_WITH_FILTER
,m_filters(NULL)
#endif
{
#ifdef CSDBG_WITH_PLUGIN
	m_plugins = new chain<plugin>;
#endif
#ifdef CSDBG_WITH_FILTER
	m_filters = new chain<filter>;
#endif

	m_proc = new process;
}

catch (...) {
	destroy();
}


/**
 * @brief Object copy constructor
 *
 * @param[in] src the source object
 *
 * @throws std::bad_alloc
 */
tracer::tracer(const tracer &src)
try:
m_proc(NULL)
#ifdef CSDBG_WITH_PLUGIN
,m_plugins(NULL)
#endif
#ifdef CSDBG_WITH_FILTER
,m_filters(NULL)
#endif
{
#ifdef CSDBG_WITH_PLUGIN
	m_plugins = src.m_plugins->clone();
#endif
#ifdef CSDBG_WITH_FILTER
	m_filters = new chain<filter>;
#endif

	m_proc = src.m_proc->clone();
}

catch (...) {
	destroy();
}


/**
 * @brief Object destructor
 */
tracer::~tracer()
{
	destroy();
}


/**
 * @brief Object virtual copy constructor
 *
 * @returns the object copy (heap allocated)
 *
 * @throws std::bad_alloc
 */
inline tracer* tracer::clone() const
{
	return new tracer(*this);
}


/**
 * @brief Assignment operator
 *
 * @param[in] rval the assigned object
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 */
tracer& tracer::operator=(const tracer &rval)
{
	if ( unlikely(this == &rval) )
		return *this;

#ifdef CSDBG_WITH_PLUGIN
	*m_plugins = *rval.m_plugins;
#endif

	*m_proc = *rval.m_proc;
	return *this;
}


/**
 * @brief Release object resources
 *
 * @returns *this
 */
tracer& tracer::destroy()
{
#ifdef CSDBG_WITH_PLUGIN
	delete m_plugins;
	m_plugins = NULL;
#endif
#ifdef CSDBG_WITH_FILTER
	delete m_filters;
	m_filters = NULL;
#endif

	delete m_proc;
	m_proc = NULL;
	return *this;
}


/**
 * @brief Stream insertion operator for csdbg::tracer objects
 *
 * @param[in] lval the output stream
 *
 * @param[in] rval the object to output
 *
 * @returns its first argument
 */
std::ostream& operator<<(std::ostream &lval, tracer &rval)
{
	util::lock();
	try {
		string buf;
		rval.trace(buf);
		lval << buf;

		util::unlock();
		return lval;
	}

	catch (exception &x) {
		lval << x;
	}

	catch (std::exception &x) {
		lval << x;
	}

	rval.unwind();
	util::unlock();
	return lval;
}


/**
 * @brief Get the process handle
 *
 * @returns this->m_proc
 */
inline process* tracer::proc() const
{
	return m_proc;
}


/**
 * @brief Get the interface object
 *
 * @returns tracer::m_iface if the interface object is enabled, NULL otherwise
 */
tracer* tracer::interface()
{
	if ( unlikely(m_iface == NULL || m_iface->m_proc == NULL) )
		return NULL;

	else if ( unlikely(m_iface->m_proc->module_count() == 0) )
		return NULL;

	return m_iface;
}


/**
 * @brief
 *	Create an exception stack trace using the simulated call stack of the
 *	current thread. The trace is appended to a string and the simulated stack is
 *	unwinded
 *
 * @param[in] dst the destination string
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 * @throws csdbg::exception
 *
 * @attention
 *	The simulated call stack is <b>unwinded even if the method fails, in any way
 *	to produce a trace</b>
 */
tracer& tracer::trace(string &dst)
{
	/* If an exception occurs, unwind, unlock and rethrow it */
	try {
		util::lock();
		thread *thr = m_proc->current_thread();

		const i8 *nm = thr->name();
		if ( likely(nm == NULL) )
			nm = "anonymous";
		dst.append("at %s thread (0x%lx) {\r\n", nm, thr->handle());

		/* For each function call */
		for (i32 i = thr->lag(); likely(i >= 0); i--) {
			const call *cur = thr->backtrace(i);
			dst.append("  at %s", cur->name());

			/* Append addr2line debug information */
			u32 prev = i + 1;
			if ( likely (prev < thr->call_depth()) ) {
				const call *caller = thr->backtrace(prev);
				mem_addr_t base = 0;

				const i8 *path = m_proc->ilookup(caller->addr(), base);
				addr2line(dst, path, cur->site() - base);
			}

			dst.append("\r\n");
		}

		dst.append("}\r\n");
		thr->unwind();
		util::unlock();
		return *this;
	}

	catch (...) {
		unwind();
		util::unlock();
		throw;
	}
}


/**
 * @brief
 *	Create the stack trace of a thread indexed by its ID and append it to a
 *	string
 *
 * @param[in] dst the destination string
 *
 * @param[in] id the thread ID
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 * @throws csdbg::exception
 */
tracer& tracer::trace(string &dst, pthread_t id) const
{
	/* If an exception occurs, unlock and rethrow it */
	try {
		util::lock();
		thread *thr = m_proc->get_thread(id);
		if ( unlikely(thr == NULL) ) {
			util::unlock();
			return const_cast<tracer&> (*this);
		}

		const i8 *nm = thr->name();
		if ( likely(nm == NULL) )
			nm = "anonymous";
		dst.append("at %s thread (0x%lx) {\r\n", nm, thr->handle());

		/* For each function call */
		for (i32 i = thr->call_depth() - 1; likely(i >= 0); i--) {
			const call *cur = thr->backtrace(i);
			dst.append("  at %s", cur->name());

			/* Append addr2line debug information */
			u32 prev = i + 1;
			if ( likely (prev < thr->call_depth()) ) {
				const call *caller = thr->backtrace(prev);
				mem_addr_t base = 0;

				const i8 *path = m_proc->ilookup(caller->addr(), base);
				addr2line(dst, path, cur->site() - base);
			}

			dst.append("\r\n");
		}

		dst.append("}\r\n");
		util::unlock();
		return const_cast<tracer&> (*this);
	}

	catch (...) {
		util::unlock();
		throw;
	}
}


/**
 * @brief Unwind the simulated call stack of the current thread
 *
 * @returns *this
 *
 * @throw std::bad_alloc
 *
 * @attention
 *	If an exception trace is not produced, before a new exception occurs, you
 *	must perform an explicit simulated call stack unwinding, to discard the
 *	current exception trace. If you don't properly unwind the simulated stack,
 *	the stored trace will mess with the next attempt to obtain a stack trace.
 *	Nevertheless, if the trace was actually created, a call to unwind doesn't
 *	affect the tracer object state at all (nothing to dispose), so it is not an
 *	error to call it once or even more times even when the trace was produced
 */
tracer& tracer::unwind()
{
	try {
		util::lock();
		m_proc->current_thread()->unwind();
		util::unlock();
		return *this;
	}

	catch (...) {
		util::unlock();
		throw;
	}
}


/**
 * @brief
 *	Create multiple stack traces using the simulated call stack of each thread.
 *	The traces are appended to a string. The stacks are not unwinded
 *
 * @param[in] dst the destination string
 *
 * @returns *this
 *
 * @throw std::bad_alloc
 * @throw csdbg::exception
 */
tracer& tracer::dump(string &dst) const
{
	try {
		util::lock();

		for (u32 i = 0, sz = m_proc->thread_count(); likely(i < sz); i++) {
			thread *thr = m_proc->get_thread(i);
			trace(dst, thr->handle());

			if ( likely(i < sz - 1) )
				dst.append("\r\n");
		}

		util::unlock();
		return const_cast<tracer&> (*this);
	}

	catch (...) {
		util::unlock();
		throw;
	}
}


#ifdef CSDBG_WITH_PLUGIN
/**
 * @brief Get the number of registered plugins
 *
 * @returns this->m_plugins->size()
 */
inline u32 tracer::plugin_count() const
{
	return m_plugins->size();
}


/**
 * @brief Register a plugin module (DSO)
 *
 * @param[in] path the path of the module file
 *
 * @param[in] scope the full scope of the plugin callbacks
 *
 * @returns the new plugin
 *
 * @throws std::bad_alloc
 * @throws csdbg::exception
 */
const plugin* tracer::add_plugin(const i8 *path, const i8 *scope)
{
	plugin *retval = NULL;
	try {
		util::lock();
		retval = new plugin(path, scope);
		m_plugins->add(retval);
		util::unlock();
		return retval;
	}

	catch (...) {
		delete retval;
		util::unlock();
		throw;
	}
}


/**
 * @brief Register a plugin
 *
 * @param[in] bgn the instrumentation starting callback
 *
 * @param[in] end the instrumentation ending callback
 *
 * @returns the new plugin
 *
 * @throws std::bad_alloc
 */
const plugin* tracer::add_plugin(modsym_t bgn, modsym_t end)
{
	plugin *retval = NULL;
	try {
		util::lock();
		retval = new plugin(bgn, end);
		m_plugins->add(retval);
		util::unlock();
		return retval;
	}

	catch (...) {
		delete retval;
		util::unlock();
		throw;
	}
}


/**
 * @brief Unregister a plugin module (DSO)
 *
 * @param[in] path the path of the module file
 *
 * @returns *this
 */
tracer& tracer::remove_plugin(const i8 *path)
{
	__D_ASSERT(path != NULL);
	if ( unlikely(path == NULL) )
		return *this;

	util::lock();
	for (u32 i = 0, sz = m_plugins->size(); likely(i < sz); i++) {
		const plugin *plg = m_plugins->at(i);

		/* If this is an inline plugin */
		if ( unlikely(plg->path() == NULL) )
			continue;

		if ( unlikely(strcmp(plg->path(), path) == 0) ) {
			m_plugins->remove(i);
			break;
		}
	}

	util::unlock();
	return *this;
}


/**
 * @brief Unregister a plugin
 *
 * @param[in] i the plugin registration index
 *
 * @returns *this
 *
 * @throws csdbg::exception
 */
tracer& tracer::remove_plugin(u32 i)
{
	try {
		util::lock();
		m_plugins->remove(i);
		util::unlock();
		return *this;
	}

	catch (...) {
		util::unlock();
		throw;
	}
}


/**
 * @brief Get a registered plugin module (DSO)
 *
 * @param[in] path the path of the module file
 *
 * @returns the plugin or NULL if no such plugin module is registered
 */
const plugin* tracer::get_plugin(const i8 *path) const
{
	__D_ASSERT(path != NULL);
	if ( unlikely(path == NULL) )
		return NULL;

	util::lock();
	for (u32 i = 0, sz = m_plugins->size(); likely(i < sz); i++) {
		const plugin *plg = m_plugins->at(i);

		/* If this is an inline plugin */
		if ( unlikely(plg->path() == NULL) )
			continue;

		if ( unlikely(strcmp(plg->path(), path) == 0) ) {
			util::unlock();
			return plg;
		}
	}

	util::unlock();
	return NULL;
}


/**
 * @brief Get a registered plugin
 *
 * @param[in] i the plugin registration index
 *
 * @returns this->m_plugins->at(i)
 *
 * @throws csdbg::exception
 */
const plugin* tracer::get_plugin(u32 i) const
{
	try {
		util::lock();
		const plugin *retval = m_plugins->at(i);
		util::unlock();
		return retval;
	}

	catch (...) {
		util::unlock();
		throw;
	}
}
#endif


#ifdef CSDBG_WITH_FILTER
/**
 * @brief Get the number of registered filters
 *
 * @returns this->m_plugins->size()
 */
inline u32 tracer::filter_count() const
{
	return m_filters->size();
}


/**
 * @brief Register a filter
 *
 * @param[in] expr the filter expression
 *
 * @param[in] icase true to ignore case on filtering, false otherwise
 *
 * @param[in] mode true to create a symbol filter, false to filter modules
 *
 * @returns the new filter
 *
 * @throws std::bad_alloc
 * @throws csdbg::exception
 */
filter* tracer::add_filter(const i8 *expr, bool icase, bool mode)
{
	filter *retval = NULL;
	try {
		retval = new filter(expr, icase, mode);
		m_filters->add(retval);
		return retval;
	}

	catch (...) {
		delete retval;
		throw;
	}
}


/**
 * @brief Unregister a filter
 *
 * @param[in] i the filter registration index
 *
 * @returns *this
 *
 * @throws csdbg::exception
 */
inline tracer& tracer::remove_filter(u32 i)
{
	m_filters->remove(i);
	return *this;
}


/**
 * @brief Get a registered filter
 *
 * @param[in] i the filter registration index
 *
 * @returns this->m_filters->at(i)
 *
 * @throws csdbg::exception
 */
inline filter* tracer::get_filter(u32 i) const
{
	return m_filters->at(i);
}
#endif
}

