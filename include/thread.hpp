#ifndef _CSDBG_THREAD
#define _CSDBG_THREAD 1

/**
	@file include/thread.hpp

	@brief Class csdbg::thread definition
*/

#include "./stack.hpp"
#include "./call.hpp"

namespace csdbg {

/**
	@brief This class represents a thread of execution in the instrumented process

	An object of this class is an abstraction of the actual threading system. It
	stores the simulated call stack and other thread specific data and it is used
	to track a thread execution. The simulated call stack can be traversed using
	simple callbacks and method thread::foreach. Currently only POSIX threads are

	supported

	@todo Use std::thread (C++11) class for portability
*/
class thread: virtual public object
{
protected:

	/* Protected variables */

	i8 *m_name;									/**< @brief Thread name */

	pthread_t m_handle;					/**< @brief Thread handle */

	stack<call> *m_stack;				/**< @brief Simulated call stack */

	i32 m_lag;									/**< @brief
																	 The number of calls that must be popped off
																	 the simulated stack for it to match the real
																	 one */

public:

	/* Constructors, copy constructors and destructor */

	explicit thread(const i8* = NULL);

	thread(const thread&);

	virtual	~thread();

	virtual thread* clone() const;


	/* Accessor methods */

	virtual const i8* name() const;

	virtual	pthread_t handle() const;

	virtual i32 lag() const;

	virtual thread& set_name(const i8*);


	/* Operator overloading methods */

	virtual thread& operator=(const thread&);


	/* Generic methods */

	virtual bool is_current() const;

	virtual u32 call_depth() const;

	virtual const call* backtrace(u32) const;

	virtual thread& called(mem_addr_t, mem_addr_t, const i8*);

	virtual thread& returned();

	virtual thread& unwind();

	virtual thread& foreach(void (*)(u32, call*)) const;
};

}

#endif

