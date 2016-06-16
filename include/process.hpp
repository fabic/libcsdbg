#ifndef _CSDBG_PROCESS
#define _CSDBG_PROCESS 1

/**
	@file include/process.hpp

	@brief Class csdbg::process definition
*/

#include "./thread.hpp"
#include "./symtab.hpp"

namespace csdbg {

/**
	@brief This class represents a process, its entire namespace and thread group

	An object of this class is an abstraction of the actual debugged process. It
	stores the whole instrumented namespace and the details of all the simulated
	threads and their stacks. The namespace consists of a number of symbol tables,
	one for each objective code module (executable and selected DSO libraries). A
	process object offers methods to perform batch symbol lookups, inverse lookups
	(given a resolved symbol find the module that defines it) and thread handling.
	A lookup cache is used internally to optimize symbol resolving. Access to the
	process object <b>is thread safe</b>

	@todo Create an object mutex
*/
class process: virtual public object
{
protected:

	/* Protected variables */

	pid_t m_pid;												/**< @brief Process ID */

	chain<thread> *m_threads;						/**< @brief Instrumented thread list */

	chain<symtab> *m_modules;						/**< @brief Symbol table list */

	chain<symbol> *m_symcache;					/**< @brief Lookup cache */


	/* Protected generic methods */

	virtual process& cache_add(mem_addr_t, const i8*);

	virtual const symbol* cache_lookup(mem_addr_t) const;

public:

	/* Constructors, copy constructors and destructor */

	process();

	process(const process&);

	virtual	~process();

	virtual process* clone() const;


	/* Accessor methods */

	virtual pid_t pid() const;


	/* Operator overloading methods */

	virtual process& operator=(const process&);


	/* Generic methods */

	/* Module (symtab) handling methods */

	virtual u32 symbol_count() const;

	virtual u32 module_count() const;

	virtual process& add_module(const i8*, mem_addr_t);

	virtual const i8* lookup(mem_addr_t);

	virtual const i8* ilookup(mem_addr_t, mem_addr_t&) const;


	/* Thread handling methods */

	virtual u32 thread_count() const;

	virtual thread* current_thread();

	virtual thread* get_thread(pthread_t) const;

	virtual thread* get_thread(const i8*) const;

	virtual thread* get_thread(u32) const;

	virtual process& cleanup_thread(pthread_t);
};

}

#endif

