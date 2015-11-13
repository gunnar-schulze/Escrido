#!/bin/bash

# ============================================
# Text formatting (colors and font appearance)
# ============================================

txtstd=$(tput sgr0)              # default mode
txtbld=$(tput bold)              # bold
bldred=${txtbld}$(tput setaf 1)  # bold red
bldgre=${txtbld}$(tput setaf 2)  # bold green
bldyel=${txtbld}$(tput setaf 3)  # bold yellow
bldblu=${txtbld}$(tput setaf 4)  # bold blue
bldpnk=${txtbld}$(tput setaf 5)  # bold pink
bldcyn=${txtbld}$(tput setaf 6)  # bold cyan

# ====================
# Display introduction
# ====================

echo
echo -e ${txtbld}"ESCRIDO compilation script."${txtstd}

# ===============
# Parse arguments
# ===============

fHelp="no"
fValgrind="no"
fClear="no"
while [ "$1" != "" ]
do
   case $1
   in
     -help)                       fHelp="yes";
                                  shift;;
     -valgrind)                   fValgrind="yes";
                                  shift;;
     -c)                          fClear="yes";
                                  shift;;
     *)                           echo -e ${bldred}"WARNING unknown option $1"${txtstd};
                                  echo;
                                  exit;;
   esac
done

# ============
# Display help
# ============

if [[ $fHelp == [Yy]* ]];  then
  echo
  echo -e ${txtbld}"The following options are available:"${txtstd}
  echo
  echo -e ${txtbld}"  [no argument]  : Compile normally"${txtstd}
  echo -e ${txtbld}"  -help          : Show this text"${txtstd}
  echo -e ${txtbld}"  -valgrind      : Optimized for valgrind analysis"${txtstd}
  echo -e ${txtbld}"  -c             : Clear console before compiling"${txtstd}
  echo
  exit
fi

# =========================================
# Eventually clear console before compiling
# =========================================

if [[ $fClear == [Yy]* ]];  then
  clear
fi

# ===========
# Directories
# ===========

sDirInclude=" -I./src/ -I./generic -I./src/independent/"

# ===========================
# Additional compiler options
# ===========================

sAddOptions="-O3 -w"
if [[ $fValgrind == [Yy]* ]];  then
  sAddOptions="-O0 -g -w"
fi

echo -e ${txtbld}"Using additional compiler options: $sAddOptions"${txtstd}
echo

# ============================
# Build command line arguments
# ============================

echo -e ${bldblu}"Step 1: Building command line arguments ..."${txtstd}

# Building the command line interpreter file interpargs.h from input file escrido.bb:
bb ./grammar/escrido.bb ./generic/interpargs.h

echo

# ==============================
# Build lexer and parser objects
# ==============================

echo -e ${bldblu}"Step 2: Building lexer and parser ..."${txtstd}

# Building the precompiler-parser files:
yacc --verbose -d -o ./generic/yescrido.c grammar/escrido.y

# Building the lexer files:
lex -o ./generic/lescrido.c grammar/escrido.l

# Compiling the lexer and parser:
g++ -std=c++11 -c -o ./lib/lescrido.o $sDirInclude $sAddOptions ./generic/lescrido.c
g++ -std=c++11 -c -o ./lib/yescrido.o $sDirInclude $sAddOptions ./generic/yescrido.c

echo

# =================
# Build main object
# =================

echo -e ${bldblu}"Step 3: Building objects ..."${txtstd}

# Compile other single compilation units:
g++ -std=c++11 -c -o ./lib/main.o $sDirInclude $sAddOptions ./src/main.cpp
g++ -std=c++11 -c -o ./lib/reftable.o $sDirInclude $sAddOptions ./src/reftable.cpp
g++ -std=c++11 -c -o ./lib/content-unit.o $sDirInclude $sAddOptions ./src/content-unit.cpp
g++ -std=c++11 -c -o ./lib/escrido-doc.o $sDirInclude $sAddOptions ./src/escrido-doc.cpp

echo

# =======
# Linking
# =======

echo -e ${bldblu}"Step 4: Linking ..."${txtstd}

# Linking to executable (normal static):
echo -e "\tLinking normally ..."
g++ -std=c++11 -o ./bin/escrido \
                  ./lib/main.o \
                  ./lib/reftable.o \
                  ./lib/content-unit.o \
                  ./lib/escrido-doc.o \
                  ./lib/lescrido.o \
                  ./lib/yescrido.o

