# 'Makefile'
MARKDOWN = pandoc --from markdown_github --to html --standalone 


all: ../pdf/ktools.pdf ../epub/ktools.epub


../epub/ktools.epub: $(patsubst %.md,../html/%.html,$(wildcard *.md)) ../html/README.html Makefile 
	wkhtmltopdf  ../html/Introduction.html ../html/Contents.html  ../html/Overview.html  ../html/Specification.html ../html/ReferenceModel.html ../html/Inputtools.html ../html/Outputtools.html ../html/PlannedWork.html ../html/RandomNumbers.html ../epub/ktools.epub
 
../pdf/ktools.pdf: $(patsubst %.md,../html/%.html,$(wildcard *.md)) ../html/README.html Makefile 
	wkhtmltopdf  ../html/Introduction.html ../html/Contents.html  ../html/Overview.html  ../html/Specification.html ../html/ReferenceModel.html ../html/Inputtools.html ../html/Outputtools.html ../html/PlannedWork.html ../html/RandomNumbers.html ../pdf/ktools.pdf
 

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
