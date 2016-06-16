#ifndef _CSDBG_OBJECT
#define _CSDBG_OBJECT 1

/**
	@file include/object.hpp

	@brief Class csdbg::object definition
*/

#include "./config.hpp"

namespace csdbg {

/**
	@brief This abstract class serves as the root of the class hierarchy tree
*/
class object
{
public:

	/* Constructors, copy constructors and destructor */

	virtual ~object() = 0;											/**< @brief To be implemented */

	virtual object* clone() const = 0;					/**< @brief To be implemented */


	/* Generic methods */

	virtual const i8* class_name() const;
};

}

#endif

