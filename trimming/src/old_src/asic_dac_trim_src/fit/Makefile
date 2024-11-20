#!/usr/bin/make
#
# execute this script to extract the examples from the MINPACK
# documentation and compile and run them.

PGM= tlmdif1
CPGM= tclmdif1
SRC= tlmdif1.f
CSRC= tlmdif1.c

all: ctest 

ctest: ${CPGM}
	for x in ${CPGM}; do echo $$x; ./$$x; done

ftest: ${PGM} 
	for x in ${PGM}; do echo $$x; ./$$x; done

tlmdif1: tlmdif1.f
	g77 -o $@ $< -lminpack

tclmdif1: tlmdif1.c
	g77 -o $@ $< -lminpack -lm
