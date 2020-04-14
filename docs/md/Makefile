# 'Makefile'
MARKDOWN = pandoc --from markdown_github --to html --standalone 


all: ../pdf/ktools.pdf


../pdf/ktools.pdf: $(patsubst %.md,../html/%.html,$(wildcard *.md)) ../html/README.html Makefile 
	wkhtmltopdf  ../html/Introduction.html ../html/Contents.html  ../html/Overview.html  ../html/Specification.html ../html/ReferenceModelOverview.html ../html/CoreComponents.html ../html/OutputComponents.html ../html/DataConversionComponents.html ../html/StreamConversionComponents.html  ../html/ValidationComponents.html ../html/FinancialModule.html ../html/Workflows.html ../html/RandomNumbers.html ../html/fmprofiles.html ../html/MultiPeril.html ../pdf/ktools.pdf
 

clean:
	rm -f $(patsubst %.md,../html/%.html,$(wildcard *.md))
	rm -f *.bak *~
	rm -f ../pdf/ktools.pdf
	rm -f ../html/README.html

../html/%.html: %.md
	$(MARKDOWN) $< --output $@
	sed -i 's/.md/.html/g' $@ 

../html/README.html: ../../README.md
	$(MARKDOWN) $< --output $@
	sed -i 's/.md/.html/g' $@ 
