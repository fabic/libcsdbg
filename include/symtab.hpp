#ifndef _CSDBG_SYMTAB
#define _CSDBG_SYMTAB 1

/**
	@file include/symtab.hpp

	@brief Class csdbg::symtab definition
*/

#include "./chain.hpp"
#include "./symbol.hpp"

namespace csdbg {

/**
	@brief This class represents a program/library symbol table (symtab section)

	A symtab object can load code from executables or dynamic shared objects, with
	absolute addressing or position independent and of any binary format supported
	by the libbfd backends on the host (target) machine (elf, coff, ecoff e.t.c).
	To optimize lookups the symbol table (as structured in libbfd) is parsed, the
	non-function symbols are discarded and function symbols are demangled once and
	stored in simpler data structures. A symtab can be traversed using callbacks
	and method symtab::foreach. The access to a symtab is not thread safe, callers
	must implement thread synchronization
*/
class symtab: virtual public object
{
protected:

	/* Protected variables */

	i8 *m_path;											/**< @brief Objective code file path */

	mem_addr_t m_base;							/**< @brief Load base address */

	chain<symbol> *m_table;					/**< @brief Function symbol table */

public:

	/* Constructors, copy constructors and destructor */

	explicit symtab(const i8*, mem_addr_t = 0);

	symtab(const symtab&);

	virtual ~symtab();

	virtual symtab* clone() const;


	/* Accessor methods */

	virtual const i8* path() const;

	virtual mem_addr_t base() const;


	/* Operator overloading methods */

	virtual symtab& operator=(const symtab&);


	/* Generic methods */

	virtual u32 size() const;

	virtual const i8* lookup(mem_addr_t) const;

	virtual bool exists(mem_addr_t) const;

	virtual symtab& foreach(void (*)(u32, symbol*)) const;
};

}

#endif

