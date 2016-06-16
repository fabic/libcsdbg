#include "../include/string.hpp"
#include "../include/util.hpp"
#if !defined CSDBG_WITH_PLUGIN && !defined CSDBG_WITH_HIGHLIGHT
#include "../include/exception.hpp"
#endif

/**
	@file src/string.cpp

	@brief Class csdbg::string method implementation
*/

namespace csdbg {

/**
 * @brief Allocate aligned memory, mandate a minimum buffer size
 *
 * @param[in] len the mandatory length (without the trailing \\0)
 *
 * @param[in] keep true to keep the current data
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 */
string& string::memalign(u32 len, bool keep)
{
	if ( unlikely(len < m_size) )
		return (keep) ? *this : clear();

	i8 *copy = NULL;
	if ( unlikely(keep) ) {
		__D_ASSERT(m_data != NULL);
		__D_ASSERT(strlen(m_data) == m_length);

		copy = new i8[m_length + 1];
		strcpy(copy, m_data);
	}

	/* Aligned size */
	m_size = (len + g_memblock_sz) / g_memblock_sz;
	m_size *= g_memblock_sz;

	try {
		delete[] m_data;
		m_data = NULL;
		m_data = new i8[m_size];
	}

	catch (...) {
		delete[] copy;
		throw;
	}

	if ( unlikely(keep) ) {
		strcpy(m_data, copy);
		delete[] copy;
		return *this;
	}

	return clear();
}


/**
 * @brief
 *	Fill with a printf-style format C-string expanded with the values of a
 *	variable argument list
 *
 * @param[in] fmt a printf-style format C-string
 *
 * @param[in] args a variable argument list (as a va_list variable)
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 * @throws csdbg::exception
 */
string& string::format(const i8 *fmt, va_list args)
{
	__D_ASSERT(fmt != NULL);
	if ( unlikely(fmt == NULL) ) {
		va_end(args);
		return memalign(0);
	}

	try {
		va_list cpargs;
		va_copy(cpargs, args);
		u32 len = util::va_size(fmt, cpargs);
		memalign(len);

		util::va_format(m_data, fmt, args);
		m_length = len;
		return *this;
	}

	catch (...) {
		va_end(args);
		throw;
	}
}


/**
 * @brief Stream insertion operator for csdbg::string objects
 *
 * @param[in] lval the output stream
 *
 * @param[in] rval the object to output
 *
 * @returns its first argument
 */
std::ostream& operator<<(std::ostream &lval, const string &rval)
{
	return lval << rval.m_data;
}


/**
 * @brief Object constructor
 *
 * @param[in] sz the minimum mandated buffer size
 *
 * @throws std::bad_alloc
 */
string::string(u32 sz):
m_data(NULL),
m_length(0),
m_size(0)
{
	memalign(sz);
}


/**
 * @brief Object constructor
 *
 * @param[in] fmt a printf-style format C-string
 *
 * @param[in] ... a variable argument list
 *
 * @throws std::bad_alloc
 * @throws csdbg::exception
 *
 * @note
 *	This constructor is also an implicit type conversion operator between i8*
 *	and csdbg::string
 */
string::string(const i8 *fmt, ...):
m_data(NULL),
m_length(0),
m_size(0)
{
	__D_ASSERT(fmt != NULL);
	if ( unlikely(fmt == NULL) )
		memalign(0);

	else {
		va_list args;
		va_start(args, fmt);
		format(fmt, args);
	}
}


/**
 * @brief Object copy constructor
 *
 * @param[in] src the source object
 *
 * @throws std::bad_alloc
 */
string::string(const string &src):
m_data(NULL),
m_length(0),
m_size(0)
{
	*this = src;
}


/**
 * @brief Object destructor
 */
string::~string()
{
	delete[] m_data;
	m_data = NULL;
}


/**
 * @brief Object virtual copy constructor
 *
 * @returns the object copy (heap allocated)
 *
 * @throws std::bad_alloc
 */
inline string* string::clone() const
{
	return new string(*this);
}


/**
 * @brief Get the C-string equivalent
 *
 * @returns this->m_data
 */
inline const i8* string::cstr() const
{
	return m_data;
}


/**
 * @brief Get the character count
 *
 * @returns this->m_length
 */
inline u32 string::length() const
{
	return m_length;
}


/**
 * @brief Get the buffer size
 *
 * @returns this->m_size
 */
inline u32 string::bufsize() const
{
	return m_size;
}


/**
 * @brief Get/set the character at an offset
 *
 * @param[in] i the offset
 *
 * @returns &this->m_data[i]
 *
 * @throws csdbg::exception
 */
inline i8& string::at(u32 i)
{
	if ( unlikely(i >= m_length) )
		throw exception("offset out of string bounds (%d >= %d)", i, m_length);

	return m_data[i];
}


/**
 * @brief
 *	Fill with a printf-style format C-string expanded with the values of a
 *	variable argument list
 *
 * @param[in] fmt a printf-style format C-string
 *
 * @param[in] ... a variable argument list
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 * @throws csdbg::exception
 */
string& string::set(const i8 *fmt, ...)
{
	__D_ASSERT(fmt != NULL);
	if ( unlikely(fmt == NULL) )
		return memalign(0);

	__D_ASSERT(fmt != m_data);
	if ( unlikely(fmt == m_data) )
		return *this;

	va_list args;
	va_start(args, fmt);
	return format(fmt, args);
}


/**
 * @brief Copy a string
 *
 * @param[in] src the source string
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 */
string& string::set(const string &src)
{
	if ( unlikely(this == &src) )
		return *this;

	memalign(src.m_length);
	strcpy(m_data, src.m_data);
	m_length = src.m_length;
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
inline string& string::operator=(const string &rval)
{
	return set(rval);
}


/**
 * @brief Compound addition-assignment operator (append)
 *
 * @param[in] rval the appended string
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 */
inline string& string::operator+=(const string &rval)
{
	return append(rval);
}


/**
 * @brief Subscript operator
 *
 * @param[in] i the index
 *
 * @returns &this->m_data[i]
 *
 * @throws csdbg::exception
 */
inline i8& string::operator[](u32 i)
{
	return at(i);
}


/**
 * @brief
 *	Get the available buffer size, the number of characters that can be appended
 *	without reallocation
 *
 * @returns this->m_size - (this->m_length + 1)
 */
inline u32 string::available() const
{
	return m_size - m_length - 1;
}


/**
 * @brief Fill the whole buffer with a constant byte
 *
 * @param[in] ch the constant byte
 *
 * @returns *this
 *
 * @attention No matter how the string is shred, it stays valid (cleared)
 */
inline string& string::shred(u8 ch)
{
	util::memset(m_data, ch, m_size);
	return clear();
}


/**
 * @brief Clear contents
 *
 * @returns *this
 */
inline string& string::clear()
{
	m_data[0] = '\0';
	m_length = 0;
	return *this;
}


/**
 * @brief Append a string
 *
 * @param[in] tail the appended string
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 */
string& string::append(const string &tail)
{
	u32 len = m_length + tail.m_length;
	memalign(len, true);
	strcpy(m_data + m_length, tail.m_data);
	m_length = len;
	return *this;
}


/**
 * @brief
 *	Append a printf-style format C-string expanded with the values of a variable
 *	argument list
 *
 * @param[in] fmt a printf-style format C-string
 *
 * @param[in] ... a variable argument list
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 * @throws csdbg::exception
 */
string& string::append(const i8 *fmt, ...)
{
	__D_ASSERT(fmt != NULL);
	if ( unlikely(fmt == NULL) )
		return *this;

	string tmp;
	va_list args;
	va_start(args, fmt);
	tmp.format(fmt, args);

	return append(tmp);
}


/**
 * @brief Append a character
 *
 * @param[in] ch the appended character
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 * @throws csdbg::exception
 */
inline string& string::append(i8 ch)
{
	return append("%c", ch);
}


/**
 * @brief Compare to another string
 *
 * @param[in] rval the compared string
 *
 * @param[in] icase true to ignore case sensitivity
 *
 * @returns
 *	<0, zero, or >0 if this is respectively less than, equal, or greater than
 *	the compared string, lexicographically
 */
inline i32 string::cmp(const string &rval, bool icase) const
{
	if ( unlikely(icase) )
		return strcasecmp(m_data, rval.m_data);

	return strcmp(m_data, rval.m_data);
}


/**
 * @brief Match against a POSIX extended regular expression
 *
 * @param[in] exp the regular expression
 *
 * @param[in] icase true to ignore case sensitivity
 *
 * @returns true if there is a match, false otherwise
 *
 * @throws csdbg::exception
 */
bool string::match(const string &exp, bool icase) const
{
	i32 flags = REG_EXTENDED | REG_NOSUB;
	if ( unlikely(icase) )
		flags |= REG_ICASE;

	/* Compile the regular expression and perform the matching */
	regex_t regexp;
	i32 retval = regcomp(&regexp, exp.cstr(), flags);
	if ( likely(retval == 0) ) {
		retval = regexec(&regexp, m_data, 0, NULL, 0);
		regfree(&regexp);
		return !retval;
	}

	/* If the expression compilation failed */
	i32 len = regerror(retval, &regexp, NULL, 0);
	i8 errbuf[len];
	regerror(retval, &regexp, errbuf, len);
	regfree(&regexp);

	throw exception(
		"failed to compile regexp '%s' (regex errno %d - %s)",
		exp.cstr(),
		retval,
		errbuf
	);
}


#if defined CSDBG_WITH_PLUGIN || defined CSDBG_WITH_HIGHLIGHT
/**
 * @brief Remove leading and/or trailing whitespace characters
 *
 * @param[in] which
 *	<0 to trim the leading, >0 for the trailing and 0 for both (default)
 *
 * @returns *this
 */
string& string::trim(i32 which)
{
	if ( likely(which <= 0) ) {
		/* Estimate the number of leading whitespace characters */
		u32 i;
		for (i = 0; likely(i < m_length); i++)
			if ( likely(!isspace(m_data[i])) )
				break;

		/* Remove them */
		if ( unlikely(i > 0 && i < m_length) ) {
			strcpy(m_data, m_data + i);
			m_length -= i;
		}

		/* If the string is filled with whitespace characters */
		else if ( unlikely(i == m_length) )
			return clear();
	}

	if ( likely(which >= 0) ) {
		/* Estimate the number of trailing whitespace characters */
		i32 i;
		for (i = m_length - 1; likely(i >= 0); i--)
			if ( likely(!isspace(m_data[i])) )
				break;

		m_data[++i] = '\0';
		m_length = i;
	}

	return *this;
}


/**
 * @brief Insert a string at a specified position
 *
 * @param[in] pos the insertion position
 *
 * @param[in] rval the inserted text
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 */
string& string::insert(u32 pos, const string &rval)
{
	if ( unlikely(pos >= m_length) )
		return append(rval);

	u32 len = m_length + rval.m_length;
	memalign(len, true);

	/* Shift the string to make place for the inserted text */
	u32 i = m_length;
	u32 j = len;
	i32 tail = m_length - pos;
	while ( likely(tail-- >= 0) )
		m_data[j--] = m_data[i--];

	strncpy(m_data + pos, rval.m_data, rval.m_length);
	m_length = len;
	return *this;
}


/**
 * @brief
 *	Insert a printf-style format C-string expanded with the values of a variable
 *	argument list, at a specified position
 *
 * @param[in] pos the insertion position
 *
 * @param[in] fmt a printf-style format C-string
 *
 * @param[in] ... a variable argument list
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 * @throws csdbg::exception
 */
string& string::insert(u32 pos, const i8 *fmt, ...)
{
	__D_ASSERT(fmt != NULL);
	if ( unlikely(fmt == NULL) )
		return *this;

	/* Format a temporary string */
	string tmp;
	va_list args;
	va_start(args, fmt);
	tmp.format(fmt, args);

	return insert(pos, tmp);
}


/**
 * @brief Tokenize using a POSIX extended regular expression
 *
 * @param[in] exp the delimiter expression
 *
 * @param[in] imatch false to include the actual matches in the result
 *
 * @param[in] icase true to ignore case sensitivity
 *
 * @returns the list of tokens (heap allocated)
 *
 * @throws std::bad_alloc
 * @throws csdbg::exception
 */
chain<string>* string::split(const string &exp, bool imatch, bool icase) const
{
	chain<string> *tokens = NULL;
	string *word = NULL;
	regex_t regexp;

	/* If an exception occurs, release resources and rethrow it */
	try {
		tokens = new chain<string>;

		/* Compile the regular expression */
		i32 flags = REG_EXTENDED;
		if ( unlikely(icase) )
			flags |= REG_ICASE;

		i32 retval = regcomp(&regexp, exp.cstr(), flags);
		if ( unlikely(retval != 0) ) {
			i32 len = regerror(retval, &regexp, NULL, 0);
			i8 errbuf[len];
			regerror(retval, &regexp, errbuf, len);

			throw exception(
				"failed to compile regexp '%s' (regex errno %d - %s)",
				exp.cstr(),
				retval,
				errbuf
			);
		}

		regmatch_t match;
		regoff_t offset = 0;
		i32 len = m_length;
		do {
			bool found = !regexec(&regexp, m_data + offset, 1, &match, 0);

			/*
			 * The delimiter pattern is found. The left token is from the beginning of
			 * the text plus an offset, to the beginning of the matched text. The
			 * right token is from the end of the matched text to the end of the text
			 * or to the beginning of the next matched text. This will be evaluated on
			 * the next loop pass
			 */
			if ( likely(found) ) {
				i32 bgn = match.rm_so;
				i32 end = match.rm_eo;
				if ( unlikely(end == 0) )
					throw exception("logic error in regular expression '%s'", exp.cstr());

				word = new string("%.*s", bgn, m_data + offset);
				tokens->add(word);
				word = NULL;

				if ( unlikely(!imatch) ) {
					word = new string("%.*s", end - bgn, m_data + offset + bgn);
					tokens->add(word);
					word = NULL;
				}

				offset += end;
				if ( unlikely(offset > len) )
					break;
			}

			/*
			 * The pattern isn't found. That means that either the delimiter was never
			 * in the text, so the whole text is the one and only token, or there is
			 * some text after the last delimiter. In that case this trailing text is
			 * the last token
			 */
			else if ( likely(offset <= len) ) {
				word = new string(m_data + offset);
				tokens->add(word);
				word = NULL;
				break;
			}

			/* No more tokens */
			else
				break;
		}

		while ( likely(true) );

		regfree(&regexp);
		return tokens;
	}

	catch (...) {
		delete tokens;
		delete word;
		regfree(&regexp);
		throw;
	}
}
#endif

}

