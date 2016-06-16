#ifndef _CSDBG_SYMBOL
#define _CSDBG_SYMBOL 1

/**
	@file include/symbol.hpp

	@brief Class csdbg::symbol definition
*/

#include "./object.hpp"

namespace csdbg {

/**
	@brief This class represents a program/library function symbol
*/
class symbol: virtual public object
{
protected:

	/* Protected variables */

	mem_addr_t m_addr;									/**< @brief Symbol address */

	i8 *m_name;													/**< @brief Symbol name */

public:

	/* Constructors, copy constructors and destructor */

	symbol(mem_addr_t, const i8*);

	symbol(const symbol&);

	virtual ~symbol();

	virtual symbol* clone() const;


	/* Accessor methods */

	virtual mem_addr_t addr() const;

	virtual const i8* name() const;


	/* Operator overloading methods */

	virtual symbol& operator=(const symbol&);
};

}

#endif

