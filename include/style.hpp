#ifndef _CSDBG_STYLE
#define _CSDBG_STYLE 1

/**
	@file include/style.hpp

	@brief Class csdbg::style definition
*/

#include "./string.hpp"

namespace csdbg {

/**
	@brief A set of formatting attributes for VT100 (and compatible) terminals

	@see csdbg::parser
	@see <a href="index.html#sec5_7"><b>5.7 Using the stack trace parser (syntax highlighter)</b></a>
*/
class style: virtual public object
{
protected:

	/* Protected variables */

	i8 *m_name;										/**< @brief Style name */

	color_t m_fgcolor;						/**< @brief Foreground (text) color */

	color_t m_bgcolor;						/**< @brief Background color */

	attrset_t m_attributes;				/**< @brief Text formatting attribute bitmask */

public:

	/* Constructors, copy constructors and destructor */

	style(const i8*, color_t = WHITE, color_t = CLEAR, attrset_t = 0);

	style(const style&);

	virtual ~style();

	virtual style* clone() const;


	/* Accessor methods */

	virtual const i8* name() const;

	virtual color_t fgcolor() const;

	virtual color_t bgcolor() const;

	virtual attrset_t attributes() const;

	virtual style& set_name(const i8*);

	virtual style& set_fgcolor(color_t);

	virtual style& set_bgcolor(color_t);

	virtual style& set_attributes(attrset_t);


	/* Operator overloading methods */

	virtual style& operator=(const style&);


	/* Generic methods */

	virtual bool is_attr_enabled(attrset_t) const;

	virtual style& set_attr_enabled(attrset_t, bool);

	virtual style& to_string(string&) const;

	virtual style& apply(string&) const;


	/* Public static variables */

	/**
		@brief Text formatting attributes of VT100 terminals
	*/
	static const enum {

		BOLD				= 0x01,		DIM					= 0x02,		UNDERLINED 	= 0x04,

		BLINKING		= 0x08,		INVERTED		= 0x10,		HIDDEN			= 0x20

	} vt100_attributes;

	/**
		@brief Basic palette of VT100 terminals
	*/
	static const enum {

		CLEAR				=	0x00,		GRAY				= 0x08,		RED					= 0x09,

		GREEN				= 0x0A,		YELLOW			= 0x0B,		BLUE				= 0x0C,

		MAGENTA			= 0x0D,		CYAN				= 0x0E,		WHITE				= 0x0F,

		BLACK			 	= 0x10

	} vt100_palette;
};

}

#endif

