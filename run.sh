#!/bin/bash

./bin/escrido -f example/escridofile
cd latex
pdflatex latex.tex
cd ..