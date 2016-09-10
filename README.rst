=======
Escrido
=======

:Authors: Gunnar Schulze
:Date: 2016-09-09
:Version: 0.3.0
:Contact: <gunnar.schulze@trinckle.com>
:Organization: trinckle 3D GmbH

What is this?
-------------

Escrido is a multi-language documentation generator. It is a tool that generates LaTeX and web documents of references and manuals for code and/or programming languages. It is strongly influenced by Doxygen and has a similar syntax. Nevertheless it differs from Doxygen in several points:

- Escrido is not bound to any programming language. This allows the markup text to be stored in any place, not only the original code file. In the same way elements that have no actual source code can be documented by Escrido.
- Its output can be controlled easily by the use of template files.
- It uses markup deliminators /\*#, \*#/ and //# that can neutrally coexist with Doxygen markup.
- Syntax is keept as similar to Doxygen as possible. Nevertheless there are certain deviations from the Doxygen syntax.
- Certain commands extend the functionality of Doxygen, like the \@table command.
- Some Doxygen functions are not supported.

How to compile?
---------------

The following accounts for a Linux enviroment with the GNU C++ compiler (g++) installed.

Prior to compilation you need to install the Blue Footed Booby (bb) command line interpreter. You can find it here:

  https://bitbucket.org/ubolosy/blue-footed-booby

To compile, run the "compile.sh" script:

 ./compile.sh

You can get a list of available options by the 'help' argument:

  ./compile.sh -help

Is there a manual?
------------------

Yes, but the manual is also the example. This means you have to create first. After compiling Escrido you can generate the manual with the following command:

  ./bin/escrido -f examples/manual/escridofile

Have fun!

  Gunnar Schulze <gunnar.schulze@gmx.de>