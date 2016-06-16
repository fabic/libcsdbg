#ifndef _CSDBG_STACK
#define _CSDBG_STACK 1

/**
	@file include/stack.hpp

	@brief Class csdbg::stack definition and method implementation
*/

#include "./node.hpp"
#include "./exception.hpp"

namespace csdbg {

/**
	@brief Lightweight, templated, singly-linked LIFO queue (stack)

	The stack supports shared data (multiple stacks can point to the same data)
	but it is not thread safe, callers should synchronize thread access. This
	implementation doesn't allow a node with a NULL or a duplicate data pointer.
	A stack can be traversed using simple callbacks and method stack::foreach.
	Apart from the legacy push/pop functions, node data can be accessed using
	stack offsets, just like a singly-linked list

	@see csdbg::node
*/
template <class T>
class stack: virtual public object
{
protected:

	/* Protected variables */

	node<T> *m_top;									/**< @brief Stack top */

	u32 m_size;											/**< @brief Node count */


	/* Protected generic methods */

	virtual node<T>* node_at(u32 i) const;

	virtual node<T>* node_with(const T*) const;

public:

	/* Constructors, copy constructors and destructor */

	stack();

	stack(const stack&);

	virtual	~stack();

	virtual stack* clone() const;


	/* Accessor methods */

	virtual	u32 size() const;


	/* Operator overloading methods */

	virtual stack& operator=(const stack&);

	virtual T* operator[](u32) const;


	/* Generic methods */

	virtual stack& push(T*);

	virtual stack& pop();

	virtual stack& clear();

	virtual T* peek(u32) const;

	virtual stack& foreach(void (*)(u32, T*)) const;
};


/**
 * @brief Get the node at a stack offset
 *
 * @param[in] i the offset
 *
 * @returns the i-th node
 *
 * @throws csdbg::exception
 */
template <class T>
node<T>* stack<T>::node_at(u32 i) const
{
	if ( unlikely(i >= m_size) )
		throw exception("offset out of stack bounds (%d >= %d)", i, m_size);

	node<T> *n = m_top;
	while ( likely(i-- > 0) )
		n = n->m_link;

	return n;
}


/**
 * @brief Get the node with m_data == d
 *
 * @param[in] d the data pointer searched
 *
 * @returns the node with m_data == d or NULL if there's no such node
 */
template <class T>
node<T>* stack<T>::node_with(const T *d) const
{
	__D_ASSERT(d != NULL);
	if ( unlikely(d == NULL) )
		return NULL;

	for (node<T> *n = m_top; likely(n != NULL); n = n->m_link)
		if ( unlikely(n->m_data == d) )
			return n;

	return NULL;
}


/**
 * @brief Object default constructor
 */
template <class T>
inline stack<T>::stack():
m_top(NULL),
m_size(0)
{
}


/**
 * @brief Object copy constructor
 *
 * @param[in] src the source object
 *
 * @throws std::bad_alloc
 */
template <class T>
inline stack<T>::stack(const stack &src)
try:
m_top(NULL),
m_size(0)
{
	*this = src;
}

catch (...) {
	clear();
}


/**
 * @brief Object destructor
 */
template <class T>
inline stack<T>::~stack()
{
	clear();
}


/**
 * @brief Object virtual copy constructor
 *
 * @returns the object copy (heap allocated)
 *
 * @throws std::bad_alloc
 */
template <class T>
inline stack<T>* stack<T>::clone() const
{
	return new stack(*this);
}


/**
 * @brief Get the stack size (node count)
 *
 * @returns this->m_size
 */
template <class T>
inline u32 stack<T>::size() const
{
	return m_size;
}


/**
 * @brief Assignment operator
 *
 * @param[in] rval the assigned object
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 *
 * @note
 *	Automatically resolves collisions when the stacks overlap (when they have
 *	nodes with the same data pointer)
 */
template <class T>
stack<T>& stack<T>::operator=(const stack &rval)
{
	if ( unlikely(this == &rval) )
		return *this;

	/* Check if the stacks overlap and detach shared data pointers */
	for (node<T> *n = m_top; likely(n != NULL); n = n->m_link)
		if ( unlikely(rval.node_with(n->m_data) != NULL) )
			n->detach();

	clear();
	for (node<T> *n = rval.m_top; likely(n != NULL); n = n->m_link) {
		T *copy = NULL;
		try {
			copy = new T(*n->m_data);
			push(copy);
		}

		catch (...) {
			delete copy;
			throw;
		}
	}

	return *this;
}


/**
 * @brief Subscript operator
 *
 * @param[in] i the index
 *
 * @returns the i-th node data pointer
 *
 * @throws csdbg::exception
 */
template <class T>
inline T* stack<T>::operator[](u32 i) const
{
	return peek(i);
}


/**
 * @brief Push a node on the stack
 *
 * @param[in] d the new node data pointer
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 * @throws csdbg::exception
 */
template <class T>
stack<T>& stack<T>::push(T *d)
{
	if ( unlikely(d == NULL) )
		throw exception("invalid argument: d (=%p)", d);

	/* If the data pointer already exists in the stack */
	if ( unlikely(node_with(d) != NULL) )
		throw exception("stack @ %p has a node with data @ %p", this, d);

	node<T> *n = new node<T>(d);
	n->m_link = m_top;
	m_top = n;
	m_size++;

	return *this;
}


/**
 * @brief Remove the top stack node
 *
 * @returns *this
 */
template <class T>
stack<T>& stack<T>::pop()
{
	__D_ASSERT(m_size > 0);
	if ( likely(m_size != 0) ) {
		node<T> *n = m_top;
		m_top = m_top->m_link;
		m_size--;
		delete n;
	}

	return *this;
}


/**
 * @brief Empty the stack
 *
 * @returns *this
 */
template <class T>
stack<T>& stack<T>::clear()
{
	node<T> *n = m_top;
	while ( likely(n != NULL) ) {
		node<T> *tmp = n->m_link;
		delete n;
		n = tmp;
	}

	m_top = NULL;
	m_size = 0;
	return *this;
}


/**
 * @brief Get the node data pointer at a stack offset
 *
 * @param[in] i the offset
 *
 * @returns the i-th node data pointer
 *
 * @throws csdbg::exception
 */
template <class T>
inline T* stack<T>::peek(u32 i) const
{
	return node_at(i)->m_data;
}


/**
 * @brief Traverse the stack with a callback for each node
 *
 * @param[in] pfunc the callback
 *
 * @returns *this
 */
template <class T>
stack<T>& stack<T>::foreach(void (*pfunc)(u32, T*)) const
{
	__D_ASSERT(pfunc != NULL);
	if ( unlikely(pfunc == NULL) )
		return const_cast<stack<T>&> (*this);

	u32 i = 0;
	for (node<T> *n = m_top; likely(n != NULL); n = n->m_link)
		pfunc(i++, n->m_data);

	return const_cast<stack<T>&> (*this);
}

}

#endif

