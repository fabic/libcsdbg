#ifndef _CSDBG_CALL
#define _CSDBG_CALL 1

/**
	@file include/call.hpp

	@brief Class csdbg::call definition
*/

#include "./symbol.hpp"

namespace csdbg {

/**
	@brief This class represents a program/library runtime function call
*/
class call: virtual public symbol
{
protected:

	/* Protected variables */

	mem_addr_t m_site;						/**< @brief Call site address */

public:

	/* Constructors, copy constructors and destructor */

	call(mem_addr_t, mem_addr_t, const i8*);

	call(const call&);

	virtual ~call();

	virtual call* clone() const;


	/* Accessor methods */

	virtual mem_addr_t site() const;


	/* Operator overloading methods */

	virtual call& operator=(const call&);
};

}

#endif

