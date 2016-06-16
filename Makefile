# Project definition
PROJECT			=		libcsdbg
LIBNAME			=		$(PROJECT).so
VERSION			=		1.28
TARGET			=		$(LIBNAME).$(VERSION)
PREFIX			=		/usr/local
PLATFORM		=


# Tools
CPP					=		$(PLATFORM)g++
STRIP				=		$(PLATFORM)strip -s
MKDIR				=		mkdir -p
TOUCH				=		touch
CP					=		cp -R
LN					=		ln -sf
RM					=		rm -rf
MV					=		mv
CD					=		cd
ECHO				=		/bin/echo -e
ID					=		id -u
TAR					=		tar -cjpf
MAKE				=		make
LDCONFIG		=		ldconfig
GREP				=		grep
DOXYGEN			=		doxygen


# Additional paths to search for header files
IPATHS			=


# Thread safety
DOPTS				=		_REENTRANT

# Include debugging code
DOPTS				+=	CSDBG_WITH_DEBUG

# Include support for color terminals (VT100)
DOPTS				+=	CSDBG_WITHOUT_COLOR_TERM

# Include code for buffered output streams
DOPTS				+=	CSDBG_WITH_STREAMBUF

ifneq (, $(findstring CSDBG_WITH_STREAMBUF, $(DOPTS)))
# Include code for buffered file output streams
DOPTS				+=	CSDBG_WITH_STREAMBUF_FILE

# Include code for buffered TCP/IP socket output streams
DOPTS				+=	CSDBG_WITH_STREAMBUF_TCP

# Include code for buffered serial tty output streams
DOPTS				+=	CSDBG_WITH_STREAMBUF_STTY
endif

# Include code for instrumentation plugins
DOPTS				+=	CSDBG_WITH_PLUGIN

# Include code for trace C++ syntax highlighting
DOPTS				+=	CSDBG_WITH_HIGHLIGHT

# Include code for instrumentation filters
DOPTS				+=	CSDBG_WITH_FILTER


# -f options
FOPTS				=		PIC
FOPTS				+=	no-enforce-eh-specs
FOPTS				+=	strict-aliasing

# -W options
WOPTS				=		all
WOPTS				+=	abi
WOPTS				+=	ctor-dtor-privacy
WOPTS				+=	non-virtual-dtor
WOPTS				+=	format-security
WOPTS				+=	init-self
WOPTS				+=	missing-include-dirs
WOPTS				+=	switch-enum
WOPTS				+=	cast-align
WOPTS				+=	cast-qual
WOPTS				+=	clobbered
WOPTS				+=	empty-body
WOPTS				+=	sign-compare
WOPTS				+=	logical-op
WOPTS				+=	missing-field-initializers
WOPTS				+=	packed
WOPTS				+=	redundant-decls
WOPTS				+=	overlength-strings
WOPTS				+=	disabled-optimization
WOPTS				+=	missing-noreturn
WOPTS				+=	type-limits
# WOPTS			+=	old-style-cast

# Generic options
GOPTS				=		O2
GOPTS				+=	rdynamic
GOPTS				+=	march=native
GOPTS				+=	std=gnu++0x


# Compiler flag setup
CFLAGS			=		$(foreach o, $(GOPTS), -$(o))
CFLAGS			+=	$(foreach w, $(WOPTS), -W$(w))
CFLAGS			+=	$(foreach f, $(FOPTS), -f$(f))
CFLAGS			+=	$(foreach d, $(DOPTS), -D$(d))
CFLAGS			+=	$(foreach p, $(IPATHS), -I$(p))


# Library modules
MODS				=		object
MODS				+=	util
MODS				+=	exception
MODS				+=	string
MODS				+=	symbol
MODS				+=	call
MODS				+=	node
MODS				+=	chain
MODS				+=	stack
MODS				+=	symtab
MODS				+=	thread
MODS				+=	process
MODS				+=	tracer

ifneq (, $(findstring CSDBG_WITH_STREAMBUF, $(DOPTS)))
MODS				+=	streambuf

ifneq (, $(findstring CSDBG_WITH_STREAMBUF_FILE, $(DOPTS)))
MODS				+=	filebuf
endif

ifneq (, $(findstring CSDBG_WITH_STREAMBUF_TCP, $(DOPTS)))
MODS				+=	tcpsockbuf
endif

ifneq (, $(findstring CSDBG_WITH_STREAMBUF_STTY, $(DOPTS)))
MODS				+=	sttybuf
endif
endif

ifneq (, $(findstring CSDBG_WITH_PLUGIN, $(DOPTS)))
MODS				+=	plugin
endif

ifneq (, $(findstring CSDBG_WITH_HIGHLIGHT, $(DOPTS)))
MODS				+=	style
MODS				+=	dictionary
MODS				+=	parser
endif

ifneq (, $(findstring CSDBG_WITH_FILTER, $(DOPTS)))
MODS				+=	filter
endif


# Documentation generating configurations
DOCGEN			=		docgen_html
DOCGEN			+=	docgen_tex

# Documentation folders
DOCPATH			=		api-ref-$(PROJECT)-$(VERSION)
DOCPATH			+=	user-manual-$(PROJECT)-$(VERSION)


# Do not edit beyond this line
.PHONY: default
default:
	$(ECHO) -n > .deps
	$(MKDIR) .build

	# Compute library module dependencies
	$(foreach m, $(MODS),																												\
		$(CPP) -MM -MT .build/$(m).o -Iinclude src/$(m).cpp >> .deps;							\
		$(ECHO) -n '\t$$(CPP) $$(CFLAGS) -c -o .build/$(m).o ' >> .deps;					\
		$(ECHO) 'src/$(m).cpp\n' >> .deps;																				\
	)

	$(MAKE) $(TARGET)


$(eval $(shell if [ -e '.deps' ]; then echo 'include .deps'; fi))
.PHONY: .deps
.deps:
	$(TOUCH) .deps


$(TARGET): $(foreach m, $(MODS), .build/$(m).o)
	$(CPP) $(CFLAGS) -shared -o .build/$@ $(foreach m, $(MODS), .build/$(m).o)
	$(STRIP) .build/$@


.PHONY: header
header:
	$(ECHO) '#ifndef _CSDBG' > .build/csdbg.hpp
	$(ECHO) '#define _CSDBG 1\n' >> .build/csdbg.hpp
	$(foreach d, $(DOPTS), $(ECHO) '#define $(d)' >> .build/csdbg.hpp;)
	$(ECHO) '\n#include "csdbg/config.hpp"' >> .build/csdbg.hpp

	# Include library module header files
	$(foreach m, $(MODS),																											\
		$(ECHO) '#include "csdbg/$(m).hpp"' >> .build/csdbg.hpp;								\
	)

	$(ECHO) '\n#endif\r\n' >> .build/csdbg.hpp


.PHONY: install
install:
	$(MAKE) header
	$(MAKE) $(PROJECT).pc

	$(MKDIR) $(PREFIX)/include/csdbg
	$(MKDIR) $(PREFIX)/lib/pkgconfig
	$(MKDIR) $(PREFIX)/lib/modules/libcsdbg

	$(CP) .build/$(TARGET) $(PREFIX)/lib
	$(CP) .build/$(PROJECT).pc $(PREFIX)/lib/pkgconfig
	$(CP) .build/csdbg.hpp $(PREFIX)/include
	$(CP) include/*.hpp  $(PREFIX)/include/csdbg

ifneq (, $(findstring CSDBG_WITH_HIGHLIGHT, $(DOPTS)))
	$(MKDIR) $(PREFIX)/bin
	$(MKDIR) $(PREFIX)/etc
	$(CP) extra/*.dict $(PREFIX)/etc
	$(CP) extra/vtcolors $(PREFIX)/bin
endif

	$(LN) $(PREFIX)/lib/$(TARGET) $(PREFIX)/lib/$(LIBNAME)
	if [ `$(ID)` -eq 0 ]; then $(LDCONFIG); fi


.PHONY: uninstall
uninstall:
	-$(RM) $(PREFIX)/lib/$(LIBNAME)*
	-$(RM) $(PREFIX)/lib/pkgconfig/$(PROJECT).pc
	-$(RM) $(PREFIX)/include/csdbg*

ifneq (, $(findstring CSDBG_WITH_HIGHLIGHT, $(DOPTS)))
	-$(RM) $(PREFIX)/etc/*.dict
	-$(RM) $(PREFIX)/bin/vtcolors
endif

	if [ `$(ID)` -eq 0 ]; then $(LDCONFIG); fi


.PHONY: clean
clean:
	-$(RM) .build
	-$(RM) .deps
	-$(RM) *.trace


.PHONY: distclean
distclean:
	$(MAKE) clean
	$(foreach p, $(DOCPATH), $(RM) doc/$(p); $(RM) doc/$(p).tar.bz2;)


.PHONY: doc
doc:
	$(foreach p, $(DOCPATH),	$(RM) doc/$(p); $(RM) doc/$(p).tar.bz2;)
	$(foreach f, $(DOCGEN),																											\
		$(CD) doc;																																\
		$(GREP) -v "^#" $(f) | $(GREP) -v "^$$" > tmp;														\
		$(MV) tmp $(f);																														\
		$(DOXYGEN) $(f);																													\
		$(CD) ..;																																	\
	)

	-$(CP) doc/*.css doc/api-ref-$(PROJECT)-$(VERSION)
	-$(CP) doc/img doc/api-ref-$(PROJECT)-$(VERSION)
	$(foreach p, $(DOCPATH), $(CD) doc; $(TAR) $(p).tar.bz2 $(p); $(CD) ..;)
	$(foreach p, $(DOCPATH), $(CD) doc/$(p); $(MAKE); $(CD) ../..;)


# Paths excluded from function instrumentation
XPATHS			=		/usr/include
XPATHS			+=	iostream
XPATHS			+=	ios
XPATHS			+=	istream
XPATHS			+=	ostream
XPATHS			+=	$(PREFIX)/include/csdbg

PC_CFLAGS		=		-I$(PREFIX)/include
PC_CFLAGS		+=	$(foreach p, $(XPATHS),																			\
									-finstrument-functions-exclude-file-list=$(p)							\
								)

PC_LFLAGS		=		-L$(PREFIX)/lib -lcsdbg -ldl -lbfd -lpthread

.PHONY: $(PROJECT).pc
$(PROJECT).pc:
	$(ECHO) 'prefix=$(PREFIX)' > .build/$(PROJECT).pc
	$(ECHO) 'exec_prefix=$(PREFIX)' >> .build/$(PROJECT).pc
	$(ECHO) 'libdir=$${exec_prefix}/lib' >> .build/$(PROJECT).pc
	$(ECHO) 'includedir=$${prefix}/include\n' >> .build/$(PROJECT).pc

	$(ECHO) 'Name: $(PROJECT)' >> .build/$(PROJECT).pc
	$(ECHO) 'Description: Call stack debugging library' >> .build/$(PROJECT).pc
	$(ECHO) 'Version: $(VERSION)' >> .build/$(PROJECT).pc
	$(ECHO) 'Author: Tasos Parisinos\n' >> .build/$(PROJECT).pc

	$(ECHO) 'Libs: $(PC_LFLAGS)' >> .build/$(PROJECT).pc

	$(ECHO) -n 'Cflags: -fPIC' >> .build/$(PROJECT).pc
	$(ECHO) -n ' -g -finstrument-functions $(PC_CFLAGS)' >> .build/$(PROJECT).pc

