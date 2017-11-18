=======
Escrido
=======

:Authors: Gunnar Schulze
:Date: 2017-11-18
:Version: 0.3.2
:Contact: <gunnar.schulze@trinckle.com>
:Organization: trinckle 3D GmbH

.. image:: logo/Escrido_logo_362x117.png

What is this?
-------------

Escrido is a multi-language documentation generator. It is a tool that generates LaTeX and web documents of references and manuals for code and/or programming languages. It is strongly influenced by Doxygen and has a similar syntax. Nevertheless it differs from Doxygen in several points:

- Escrido is not bound to any programming language. This allows the markup text to be stored in any place, not only the original code file. In the same way elements that have no actual source code can be documented by Escrido.
- Its output can be controlled easily by the use of template files.
- It uses markup deliminators /\*#, \*#/ and //# that can neutrally coexist with Doxygen markup.
- Syntax is keept as similar to Doxygen as possible. Nevertheless there are certain deviations from the Doxygen syntax.
- Certain commands extend the functionality of Doxygen, like the ``table`` command.
- Some Doxygen functions are not supported.

How to compile?
---------------

This compiling was tested on a Linux system. The following software must be installed:

- GNU C++ compiler (g++) (version 4.8 or higher)
- Make build automation tool (make)
- flex and bison lexical analyzer and parser generator (lex and yacc)

Under Debian Linux these three can be installed via the command

 sudo apt-get install build-essential flex bison

To compile start the makefile from within the Escrido directory:

 make

Is there a manual?
------------------

Yes, but the manual is also the example. This means you have to create first. After compiling Escrido you can generate the manual with the following command:

  ./bin/escrido -f examples/manual/escridofile

Have fun!

  Gunnar Schulze <gunnar.schulze@gmx.de>