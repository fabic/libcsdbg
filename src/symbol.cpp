#include "../include/symbol.hpp"

/**
	@file src/symbol.cpp

	@brief Class csdbg::symbol method implementation
*/

namespace csdbg {

/**
 * @brief Object constructor
 *
 * @param[in] addr the symbol address
 *
 * @param[in] nm the symbol name (NULL if the symbol is unresolved)
 *
 * @throws std::bad_alloc
 */
symbol::symbol(mem_addr_t addr, const i8 *nm):
m_addr(addr),
m_name(NULL)
{
	__D_ASSERT(nm != NULL);
	if ( likely(nm != NULL) ) {
		m_name = new i8[strlen(nm) + 1];
		strcpy(m_name, nm);
	}
}


/**
 * @brief Object copy constructor
 *
 * @param[in] src the source object
 *
 * @throws std::bad_alloc
 */
symbol::symbol(const symbol &src):
m_addr(src.m_addr),
m_name(NULL)
{
	i8 *buf = src.m_name;
	if ( likely(buf != NULL) ) {
		m_name = new i8[strlen(buf) + 1];
		strcpy(m_name, buf);
	}
}


/**
 * @brief Object destructor
 */
symbol::~symbol()
{
	delete[] m_name;
	m_name = NULL;
}


/**
 * @brief Object virtual copy constructor
 *
 * @returns the object copy (heap allocated)
 *
 * @throws std::bad_alloc
 */
inline symbol* symbol::clone() const
{
	return new symbol(*this);
}


/**
 * @brief Get the symbol address
 *
 * @returns this->m_addr
 */
inline mem_addr_t symbol::addr() const
{
	return m_addr;
}


/**
 * @brief Get the symbol name
 *
 * @returns this->m_name
 */
inline const i8* symbol::name() const
{
	return m_name;
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
symbol& symbol::operator=(const symbol &rval)
{
	if ( unlikely(this == &rval) )
		return *this;

	m_addr = rval.m_addr;

	i8 *buf = rval.m_name;
	if ( unlikely(buf == NULL) ) {
		delete[] m_name;
		m_name = NULL;
		return *this;
	}

	u32 len = strlen(buf);
	if ( unlikely(m_name == NULL || len > strlen(m_name)) ) {
		delete[] m_name;
		m_name = NULL;
		m_name = new i8[len + 1];
	}

	strcpy(m_name, buf);
	return *this;
}

}

