
usage:
	@echo "usage:"
	@echo "make latex | xdvi | leaflet | pdf | pdf2 | clean | veryclean"

latex:
	latex main.tex

leaflet: latex
	dvips -t landscape -o main.ps main
	pstops '2:0@-1(21cm,29.7cm),1' main.ps > leaflet.ps
	rm -f main.ps

xdvi:
	xdvi -paper 29.7x21cm main.dvi

pdf:
	gs -dNOPAUSE -sDEVICE=pdfwrite -sOutputFile=leaflet.pdf -c quit leaflet.ps

# Fuehrt zu fehlerhaftem Layout im PDF!
pdf2:
	ps2pdf13 leaflet.ps leaflet.pdf

clean:
	rm -f *.aux *.log *.dvi *~ leaflet.ps leaflet.pdf

veryclean: clean
	rm -f *.ps	

