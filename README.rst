=======
Escrido
=======

:Authors: Gunnar Schulze
:Date: 2020-08-17
:Version: 1.6.1
:Contact: <gunnar.schulze@gmx.com>

.. image:: logo/Escrido_logo_362x117.png

What is this?
-------------

Escrido is a multi-language documentation generator. It is a tool that generates HTML and LaTeX documents of references and manuals for code and/or programming languages. It is influenced by Doxygen and has a similar syntax. Nevertheless it differs from Doxygen in several points:

- Escrido is not bound to any programming language. This allows the markup text to be stored in any place, not only the original code file. In the same way elements that have no actual source code can be documented by Escrido.
- Its output can be controlled easily by the use of template files.
- It uses markup deliminators /\*#, \*#/ and //# that can neutrally coexist with Doxygen markup.
- Syntax is keept as similar to Doxygen as possible. Nevertheless there are certain deviations from the Doxygen syntax.
- Certain commands extend the functionality of Doxygen, like the ``table`` command.
- Some Doxygen functions are not supported.

Precompiled version (Windows)
-----------------------------

The respository contains an already compiled Windows version of Escrido in folder "winbuilds".

How to compile?
---------------

This compiling was tested on a Linux system. The following software must be installed:

- GNU C++ compiler (g++) (version 4.8 or higher)
- Make build automation tool (make)
- flex and bison lexical analyzer and parser generator (lex and yacc)

Under Debian Linux these three can be installed via the command::

 sudo apt-get install build-essential flex bison

To compile, run the makefile from within the Escrido directory::

 make

or (for Windows)::

 make win

Static compiling
----------------

If you wish for compiling statically (i.e. to not avoid shipping .dll files), add a modification of the linker arguments to make::

 make LINKFLAGS="-std=c++11 -static -pthread -static-libgcc -static-libstdc++"

Compiling issues
----------------

One common problem during compiling is an error that occurs in files "precomp.h", "precomp.l", "mcomp.h" and "mcomp.l" and that is caused by a different version of the lexical scanner tool "lex". If you encounter this issue, try the "lex compatibility mode" for compiling by calling make as follows::

 make ARG="-D LEX_COMPAT_MODE"

or for Windows::

 make win ARG="-D LEX_COMPAT_MODE"

Is there a manual?
------------------

Yes, and the manual is also one example for using Escrido. Its sources are in folder "examples/manual". To create the HTML version, compile Escrido first and call it by the following command::

  ./bin/escrido -f examples/manual/escridofile

Have fun!

  Gunnar Schulze <gunnar.schulze@gmx.de>
