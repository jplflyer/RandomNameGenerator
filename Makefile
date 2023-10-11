#----------------------------------------------------------------------
# The default target.
#----------------------------------------------------------------------
all: directories makelib makeprograms

include /usr/local/etc/Makefile-Base
ifneq (,$(wildcard ${DEPDIR}/*))
include .d/*
endif

#----------------------------------------------------------------------
# Rules.
#----------------------------------------------------------------------
TEST_SRC=tests
VPATH := ${SRCDIR}:${TEST_SRC}

CXXFLAGS += -I${SRCDIR}

SRC := $(shell find ${SRCDIR} -name "*.cpp" \
	| egrep  -v 'NameGen' \
  | sort \
)

SRC_NODIR := $(notdir ${SRC})
OBJ := $(patsubst %.cpp,${OBJDIR}/%.o,${SRC_NODIR})

LIBNAME=name-gen${MACAPPEND}
LIB=lib${LIBNAME}.a
LIB_ARGS= -cvrU
ifeq ($(UNAME), Darwin)
    LIB_ARGS= -cvr
endif

INSTALL_BASE=/usr/local


# This is to get the ShowLib.
LIBSHOW=show${MACAPPEND}

LDFLAGS += -L. -l${LIBNAME} -l${LIBSHOW}

#----------------------------------------------------------------------
# This installs magic_enum -> /usr/local/include.
#
# Sudo this command
#----------------------------------------------------------------------
install-magic-enum:
	git clone https://github.com/Neargye/magic_enum.git /tmp/magic_enum
	mkdir -p /usr/local/include/magic_enum
	cp /tmp/magic_enum/include/* /usr/local/include/magic_enum
	rm -rf /tmp/magic-enum

#----------------------------------------------------------------------
# Create directories.
#----------------------------------------------------------------------
# Clean the contents of the subdirs.
.PHONY: clean
clean:
	rm -f ${DEPDIR}/* ${OBJDIR}/* ${LIB} .generated

#======================================================================
# Making the library.
#======================================================================
.PHONY: makelib
makelib:
	@$(MAKE) ${THREADING_ARG} --output-sync=target --no-print-directory lib

lib: ${LIB}

${LIB}: ${OBJ}
	@mkdir -p lib
	ar ${LIB_ARGS} ${LIB} ${OBJ}
	ranlib ${LIB}

#======================================================================
# Making any programs.
#======================================================================
directories: ${BINDIR}

.PHONY: makeprograms
makeprograms:
	@$(MAKE) ${THREADING_ARG} --output-sync=target --no-print-directory programs

programs: ${BINDIR}/NameGen

${BINDIR}/%: ${OBJDIR}/%.o
	$(CXX) $^ ${LDFLAGS} ${LIB_DIRS} ${LIBS} $(OUTPUT_OPTION)
