#include "../include/thread.hpp"

/**
	@file src/thread.cpp

	@brief Class csdbg::thread method implementation
*/

namespace csdbg {

/**
 * @brief Object constructor
 *
 * @param[in] nm the thread name (it can be NULL)
 *
 * @throws std::bad_alloc
 */
thread::thread(const i8 *nm)
try:
m_name(NULL),
m_handle(pthread_self()),
m_stack(NULL),
m_lag(0)
{
	if ( unlikely(nm != NULL) ) {
		m_name = new i8[strlen(nm) + 1];
		strcpy(m_name, nm);
	}

	m_stack = new stack<call>;
}

catch (...) {
	delete[] m_name;
	m_name = NULL;
}


/**
 * @brief Object copy constructor
 *
 * @param[in] src the source object
 *
 * @throws std::bad_alloc
 */
thread::thread(const thread &src)
try:
m_name(NULL),
m_handle(src.m_handle),
m_stack(NULL),
m_lag(src.m_lag)
{
	const i8 *nm = src.m_name;
	if ( unlikely(nm != NULL) ) {
		m_name = new i8[strlen(nm) + 1];
		strcpy(m_name, nm);
	}

	m_stack = src.m_stack->clone();
}

catch (...) {
	delete[] m_name;
	m_name = NULL;
}


/**
 * @brief Object destructor
 */
thread::~thread()
{
	delete[] m_name;
	delete m_stack;
	m_name = NULL;
	m_stack = NULL;
}


/**
 * @brief Object virtual copy constructor
 *
 * @returns the object copy (heap allocated)
 *
 * @throws std::bad_alloc
 */
inline thread* thread::clone() const
{
	return new thread(*this);
}


/**
 * @brief Get the thread name
 *
 * @returns this->m_name
 */
inline const i8* thread::name() const
{
	return m_name;
}


/**
 * @brief Get the thread handle
 *
 * @returns this->m_handle
 */
inline pthread_t thread::handle() const
{
	return m_handle;
}


/**
 * @brief
 *	Get the size (call depth) of the simulated call stack with respect to the
 *	real call stack, the number of calls that must be popped off the simulated
 *	stack to match the real one
 *
 * @returns this->m_lag
 */
inline i32 thread::lag() const
{
	return m_lag;
}


/**
 * @brief Set the thread name
 *
 * @param[in] nm the new name (NULL to unset)
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 */
thread& thread::set_name(const i8 *nm)
{
	if ( unlikely(nm == NULL) ) {
		delete[] m_name;
		m_name = NULL;
		return *this;
	}

	u32 len = strlen(nm);
	if ( likely(m_name == NULL || len > strlen(m_name)) ) {
		delete[] m_name;
		m_name = NULL;
		m_name = new i8[len + 1];
	}

	strcpy(m_name, nm);
	return *this;
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
thread& thread::operator=(const thread &rval)
{
	if ( unlikely(this == &rval) )
		return *this;

	*m_stack = *rval.m_stack;
	m_handle = rval.m_handle;
	m_lag = rval.m_lag;

	return set_name(rval.m_name);
}


/**
 * @brief Check if this is the currently executing thread
 *
 * @returns true if this is the active thread, false otherwise
 */
inline bool thread::is_current() const
{
	return pthread_equal(m_handle, pthread_self()) != 0;
}


/**
 * @brief Get the size (call depth) of the simulated call stack
 *
 * @returns this->m_stack->size()
 */
inline u32 thread::call_depth() const
{
	return m_stack->size();
}


/**
 * @brief Peek at the simulated call stack
 *
 * @param[in] i the offset
 *
 * @returns the i-th function call
 *
 * @throws csdbg::exception
 */
inline const call* thread::backtrace(u32 i) const
{
	return m_stack->peek(i);
}


/**
 * @brief Simulate a function call
 *
 * @param[in] addr the function address
 *
 * @param[in] site the call site address
 *
 * @param[in] nm the function name
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 */
thread& thread::called(mem_addr_t addr, mem_addr_t site, const i8 *nm)
{
	/*
	 * If the function is called while an exception is unwinding the stack, keep
	 * track of the call depth difference between the simulated and the real call
	 * stack (the 'lag')
	 */
	if ( unlikely(std::uncaught_exception()) ) {
		m_lag--;
		return *this;
	}

	call *c = NULL;
	try {
		__D_ASSERT(nm != NULL);
		c = new call(addr, site, nm);
		m_stack->push(c);
		return *this;
	}

	catch (...) {
		delete c;
		throw;
	}
}


/**
 * @brief Simulate a function return
 *
 * @returns *this
 */
thread& thread::returned()
{
	/*
	 * If the function returned because an exception is propagating, unwinding the
	 * stack, keep track of the call depth difference between the simulated and
	 * the real call stack (the 'lag')
	 */
	if ( unlikely(std::uncaught_exception()) )
		m_lag++;
	else
		m_stack->pop();

	return *this;
}


/**
 * @brief Unwind the simulated call stack to meet the real call stack
 *
 * @returns *this
 */
thread& thread::unwind()
{
	while ( likely(m_lag > 0) ) {
		m_stack->pop();
		m_lag--;
	}

	return *this;
}


/**
 * @brief Traverse the simulated stack with a callback for each call
 *
 * @param[in] pfunc the callback
 *
 * @returns *this
 */
inline thread& thread::foreach(void (*pfunc)(u32, call*)) const
{
	m_stack->foreach(pfunc);
	return const_cast<thread&> (*this);
}

}

