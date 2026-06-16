# ====================
# Display introduction
# ====================

$(info escrido makefile 1.1)
$(info Copyright (C) 2017 Gunnar Schulze)
$(info )

# ===============================
# Compiler and linker executables
# ===============================

CPP := g++
LINK := g++

# =====================================
# Lexical scanner and parser executable
# =====================================

LEX := lex
YACC := yacc

# =========================
# Compiler and linker flags
# =========================

# Compiler flag options:
# For useing valgrind use here: CPPFLAGS="-std=c++11 -O0 -g -w"
CPPFLAGS := -std=c++11 -O3 -w

# Linker flag options:
LINKFLAGS := -std=c++11

# Additional compiler flag (can be overwritten by calling user):
ARG :=

# Define for OS selection in module 'filesys.h'.
# This needs to be changed for targeting different OSs:
OSDEF := -D FILESYS_UNIX

# macOS overrides (must come after the baseline variable definitions above):
# - Apple 'yacc' is an Xcode shim requiring full Xcode; use Homebrew bison (>= 2.5
#   required for %define parse.error verbose) in yacc-compatibility mode instead.
# - Apple flex uses size_t for the YY_INPUT result parameter instead of int; define
#   LEX_COMPAT_MODE to select the matching typedef in grammar/escrido.l.
# - __gnu_cxx::stdio_filebuf is a GCC/libstdc++ extension unavailable in clang/libc++;
#   filesys.h includes a compatibility shim (gnu_ext_stdio_filebuf.h) on __APPLE__.
ifeq ($(shell uname -s),Darwin)
    HOMEBREW_BISON := $(shell ls /opt/homebrew/opt/bison/bin/bison 2>/dev/null || ls /usr/local/opt/bison/bin/bison 2>/dev/null)
    ifneq ($(HOMEBREW_BISON),)
        YACC := $(HOMEBREW_BISON) -y
    else
        YACC := bison -y
    endif
    OSDEF := -D FILESYS_UNIX -D LEX_COMPAT_MODE
endif

# ===========
# Directories
# ===========

# Binary directory:
BINDIR := bin

# Target directory for .o and .a files:
LIBDIR := lib

# Directory for generic source files:
GENDIR := generic

# List of directories of all source files:
SRCDIRS := src \
           src/independent \
           $(GENDIR)

# Generate a list of include directories for the compiler call:
INCLUDEDIRS := $(addprefix -I./, $(SRCDIRS))

# Generate lists of all source files:
SRCFILES_C   := $(foreach directory, $(SRCDIRS), $(wildcard $(directory)/*.c))
SRCFILES_CPP := $(foreach directory, $(SRCDIRS), $(wildcard $(directory)/*.cpp))

# Generate a list of all object files:
OBJFILES := $(addprefix $(LIBDIR)/, $(notdir $(SRCFILES_C:.c=.o) $(SRCFILES_CPP:.cpp=.o)))
OBJFILES := $(OBJFILES) $(LIBDIR)/lescrido.o $(LIBDIR)/yescrido.o
#$(info $(OBJFILES))

# =================
# Build main target
# =================

# Main target created by linking.
$(BINDIR)/escrido: $(OBJFILES)
	@echo "Linking all modules ..."
	$(LINK) $(LINKFLAGS) -o $(BINDIR)/escrido $(OBJFILES)
	@echo

# Modelling dependency of module 'main' from the generic header files
# 'interpargs.h' (of the command line argument parser) and 'yescrido.h' (of the
# yacc parser)
$(LIBDIR)/main.o: $(GENDIR)/interpargs.h $(GENDIR)/yescrido.h

# =========================
# Build object file targets
# =========================

# Reminder: input of the rule is '$<', output is '$@'

# Pattern templates for compiling a single module.
# (This is packed into a 'define' statement to generate a pattern for
# each source directory of SRCDIRS.)
# (Double dollar signs '$' are required to work inside 'define'
# environment.)

define C_COMPILE_RULE_TEMPLATE
$$(LIBDIR)/%.o: $(1)/%.c
	@echo "Compiling module '$$<' ..."
	$$(CPP) $$(INCLUDEDIRS) $$(CPPFLAGS) $$(ARG) $$(OSDEF) -c -o $$@ $$<
	@echo
endef

define CPP_COMPILE_RULE_TEMPLATE
$$(LIBDIR)/%.o: $(1)/%.cpp
	@echo "Compiling module '$$<' ..."
	$$(CPP) $$(INCLUDEDIRS) $$(CPPFLAGS) $$(ARG) $$(OSDEF) -c -o $$@ $$<
	@echo
endef

# Generate pattern for each source directory out of SRCDIRS.
$(foreach directory, $(sort $(SRCDIRS)), $(eval $(call C_COMPILE_RULE_TEMPLATE,$(directory))))
$(foreach directory, $(sort $(SRCDIRS)), $(eval $(call CPP_COMPILE_RULE_TEMPLATE,$(directory))))

# ============================
# Build command line arguments
# ============================

# Check that blue-footed-booby is available and eventually compile it.
tools/bb/bb: tools/bb/sources/bb.cpp
	@echo "Compiling blue-footed booby ..."
	$(CPP) -std=c++11 -O3 tools/bb/sources/bb.cpp -o tools/bb/bb
	@echo

# Generate command line file using the blue-footed-booby.
$(GENDIR)/interpargs.h: tools/bb/bb grammar/escrido.bb
	@echo "Generating command line parser ..."
	tools/bb/bb grammar/escrido.bb $(GENDIR)/interpargs.h
	@echo

# =================
# Build yacc parser
# =================

# Rule for building the yacc parser output files yescrido.h and
# yescrido.c from input file escrido.y:
$(GENDIR)/yescrido.h \
$(GENDIR)/yescrido.c: grammar/escrido.y
	@echo "Building yacc parser ..."
	$(YACC) -d -o $(GENDIR)/yescrido.c grammar/escrido.y
	@echo

# Rule for building the lexer output files lescrido.c from input files
# files yescrido.h and escrido.l:
$(GENDIR)/lescrido.c: $(GENDIR)/yescrido.h grammar/escrido.l
	@echo "Building lexical analyzer ..."
	$(LEX) -o $(GENDIR)/lescrido.c grammar/escrido.l
	@echo

# ==============
# "clean" target
# ==============

.PHONY: clean
clean:
	rm -f $(BINDIR)/escrido
	rm -f $(LIBDIR)/*.o
	rm -f $(GENDIR)/*
	rm -f tools/bb/bb

# ============
# "win" target
# ============

.PHONY: win
win: OSDEF := -D FILESYS_WINDOWS
win: CPPFLAGS := -std=gnu++11 -O3 -w
win: LINKFLAGS := -std=gnu++11 -pthread -static-libgcc -static-libstdc++
win: $(BINDIR)/escrido
