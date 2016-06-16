#include "../include/process.hpp"
#include "../include/util.hpp"

/**
	@file src/process.cpp

	@brief Class csdbg::process method implementation
*/

namespace csdbg {

/**
 * @brief Add a symbol to the lookup cache
 *
 * @param[in] addr the symbol address
 *
 * @param[in] nm the symbol name (NULL if the address is unresolved)
 *
 * @returns *this
 */
process& process::cache_add(mem_addr_t addr, const i8 *nm)
{
	util::lock();
	symbol *sym = NULL;
	try {
		sym = new symbol(addr, nm);
		m_symcache->add(sym);
	}

	catch (std::exception &x) {
		delete sym;
		util::dbg_error("in process::%s(): %s", __FUNCTION__, x.what());
	}

	util::unlock();
	return *this;
}


/**
 * @brief Perform a cache lookup
 *
 * @param[in] addr the address to lookup
 *
 * @returns the cached symbol or NULL if no such lookup is cached
 */
const symbol* process::cache_lookup(mem_addr_t addr) const
{
	util::lock();

	/*
	 * Search the cache starting from the last entry (the latest added) to exploit
	 * locality of reference
	 */
	for (i32 i = m_symcache->size() - 1; likely(i >= 0); i--) {
		symbol *sym = m_symcache->at(i);
		if ( unlikely(sym->addr() == addr) ) {
			util::unlock();
			return sym;
		}
	}

	util::unlock();
	return NULL;
}


/**
 * @brief Object default constructor
 *
 * @throws std::bad_alloc
 */
process::process()
try:
m_pid(getpid()),
m_threads(NULL),
m_modules(NULL),
m_symcache(NULL)
{
	m_threads = new chain<thread>;
	m_modules = new chain<symtab>;
	m_symcache = new chain<symbol>;
}

catch (...) {
	delete m_threads;
	delete m_modules;
	m_threads = NULL;
	m_modules = NULL;
}


/**
 * @brief Object copy constructor
 *
 * @param[in] src the source object
 *
 * @throws std::bad_alloc
 */
process::process(const process &src)
try:
m_pid(src.m_pid),
m_threads(NULL),
m_modules(NULL),
m_symcache(NULL)
{
	util::lock();
	m_threads = src.m_threads->clone();
	m_modules = src.m_modules->clone();
	m_symcache = src.m_symcache->clone();
	util::unlock();
}

catch (...) {
	delete m_threads;
	delete m_modules;
	m_threads = NULL;
	m_modules = NULL;
	util::unlock();
}


/**
 * @brief Object destructor
 */
process::~process()
{
	util::lock();
	delete m_threads;
	delete m_modules;
	delete m_symcache;

	m_threads = NULL;
	m_modules = NULL;
	m_symcache = NULL;
	util::unlock();
}


/**
 * @brief Object virtual copy constructor
 *
 * @returns the object copy (heap allocated)
 *
 * @throws std::bad_alloc
 */
inline process* process::clone() const
{
	return new process(*this);
}


/**
 * @brief Get the process ID
 *
 * @returns this->m_pid
 */
inline pid_t process::pid() const
{
	return m_pid;
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
process& process::operator=(const process &rval)
{
	if ( unlikely(this == &rval) )
		return *this;

	util::lock();
	try {
		m_pid = rval.m_pid;
		*m_threads = *rval.m_threads;
		*m_modules = *rval.m_modules;
		*m_symcache = *rval.m_symcache;

		util::unlock();
		return *this;
	}

	catch (...) {
		util::unlock();
		throw;
	}
}


/**
 * @brief Get the number of symbols
 *
 * @returns the sum of the loaded symbol table sizes
 */
u32 process::symbol_count() const
{
	u32 cnt = 0;
	for (i32 i = m_modules->size() - 1; likely(i >= 0); i--)
		cnt += m_modules->at(i)->size();

	return cnt;
}


/**
 * @brief Get the number of modules
 *
 * @returns this->m_modules->size()
 */
inline u32 process::module_count() const
{
	return this->m_modules->size();
}


/**
 * @brief
 *	Add a symbol table to the namespace. The symbol table is loaded from a non
 *	stripped objective code file (executable or DSO library)
 *
 * @param[in] path the path of the objective code file
 *
 * @param[in] base the load base address
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 * @throws csdbg::exception
 */
process& process::add_module(const i8 *path, mem_addr_t base)
{
	util::lock();
	symtab *tbl = NULL;
	try {
		tbl = new symtab(path, base);
		m_modules->add(tbl);
		util::unlock();
		return *this;
	}

	catch (...) {
		delete tbl;
		util::unlock();
		throw;
	}
}


/**
 * @brief Lookup an address to resolve a symbol
 *
 * @param[in] addr the address
 *
 * @returns the demangled symbol or NULL if the address is unresolved
 *
 * @note
 *	If demangling failed upon symbol table loading/parsing the decorated symbol
 *	is returned
 */
const i8* process::lookup(mem_addr_t addr)
{
	const symbol *sym = cache_lookup(addr);
	if ( likely(sym != NULL) )
		return sym->name();

	for (u32 i = 0, sz = m_modules->size(); likely(i < sz); i++) {
		const	i8 *retval = m_modules->at(i)->lookup(addr);

		if ( unlikely(retval != NULL) ) {
			cache_add(addr, retval);
			return retval;
		}
	}

	/* The address was not resolved */
	cache_add(addr, NULL);
	return NULL;
}


/**
 * @brief
 *	Inverse lookup. Find the module (executable or DSO library) that defines a
 *	symbol and return its path and load base address
 *
 * @param[in] addr the symbol address
 *
 * @param[out] base the load base address of the module
 *
 * @returns the path of the module or NULL if the address is unresolved
 *
 * @see tracer::addr2line
 */
const i8* process::ilookup(mem_addr_t addr, mem_addr_t &base) const
{
	for (u32 i = 0, sz = m_modules->size(); likely(i < sz); i++) {
		const symtab *tbl = m_modules->at(i);

		if ( unlikely(tbl->exists(addr)) ) {
			base = tbl->base();
			return tbl->path();
		}
	}

	base = 0;
	return NULL;
}


/**
 * @brief Get the active thread count
 *
 * @returns this->m_threads->size()
 */
inline u32 process::thread_count() const
{
	return m_threads->size();
}


/**
 * @brief Get the currently executing thread
 *
 * @returns the csdbg::thread object that tracks the actual current thread
 *
 * @throws std::bad_alloc
 *
 * @note
 *	When an actual thread is created the m_threads chain is populated with an
 *	entry for the equivalent csdbg::thread object when the thread executes its
 *	first <b>instrumented</b> function
 */
thread* process::current_thread()
{
	util::lock();
	for (u32 i = 0, sz = m_threads->size(); likely(i < sz); i++) {
		thread *thr = m_threads->at(i);

		if ( unlikely(thr->is_current()) ) {
			util::unlock();
			return thr;
		}
	}

	thread *retval = NULL;
	try {
		retval = new thread;
		m_threads->add(retval);
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
 * @brief Get a thread by ID
 *
 * @param[in] id the thread ID
 *
 * @returns
 *	the csdbg::thread object that tracks the actual thread (with the given ID)
 *	or NULL if no such thread is found
 */
thread* process::get_thread(pthread_t id) const
{
	util::lock();
	for (u32 i = 0, sz = m_threads->size(); likely(i < sz); i++) {
		thread *thr = m_threads->at(i);

		if ( unlikely(thr->is_current()) ) {
			util::unlock();
			return thr;
		}
	}

	util::unlock();
	return NULL;
}


/**
 * @brief Get a thread by name
 *
 * @param[in] nm the name
 *
 * @returns
 *	the csdbg::thread object that tracks the actual thread (with the given name)
 *	or NULL if no such thread is found
 */
thread* process::get_thread(const i8 *nm) const
{
	__D_ASSERT(nm != NULL);
	if ( unlikely(nm == NULL) )
		return NULL;

	util::lock();
	for (u32 i = 0, sz = m_threads->size(); likely(i < sz); i++) {
		thread *thr = m_threads->at(i);

		if ( unlikely(strcmp(thr->name(), nm) == 0) ) {
			util::unlock();
			return thr;
		}
	}

	util::unlock();
	return NULL;
}


/**
 * @brief Get a thread by its offset in the active thread enumerator
 *
 * @param[in] i the offset
 *
 * @returns this->m_threads->at(i)
 *
 * @throws csdbg::exception
 */
thread* process::get_thread(u32 i) const
{
	try {
		util::lock();
		thread *retval = m_threads->at(i);
		util::unlock();
		return retval;
	}

	catch (...) {
		util::unlock();
		throw;
	}
}


/**
 * @brief Cleanup libcsdbg-related thread resources upon thread cancellation
 *
 * @param[in] id the thread ID
 *
 * @returns *this
 *
 * @attention
 *	This method should be called from thread cancellation handlers to release
 *	resources. If you don't cleanup the thread handle though it becomes useless
 *	when the actual thread has exited, it continues to occupy memory and will
 *	also inject junk, empty traces in dumps or in explicit trace requests
 *
 * @see man pthread_cleanup_push, pthread_cleanup_pop
 */
process& process::cleanup_thread(pthread_t id)
{
	util::lock();
	for (u32 i = 0, sz = m_threads->size(); likely(i < sz); i++) {
		thread *thr = m_threads->at(i);

		if ( unlikely(thr->is_current()) ) {
			m_threads->remove(i);
			util::unlock();
			break;
		}
	}

	util::unlock();
	return *this;
}

}

