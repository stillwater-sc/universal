# Makefile for Universal systems paper
# Requires: pdflatex, bibtex (texlive)
#
# Usage:
#   make          - build main.pdf
#   make clean    - remove build artifacts

MAIN = main

all: $(MAIN).pdf

$(MAIN).pdf: $(MAIN).tex references.bib
	pdflatex $(MAIN)
	bibtex $(MAIN)
	pdflatex $(MAIN)
	pdflatex $(MAIN)

clean:
	rm -f *.aux *.bbl *.blg *.log *.out *.pdf *.toc

.PHONY: all clean
