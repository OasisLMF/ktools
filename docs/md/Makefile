# 'Makefile'
MARKDOWN = pandoc --from markdown_github --to html --standalone 
all: $(patsubst %.md,../html/%.html,$(wildcard *.md)) Makefile

clean:
	rm -f $(patsubst %.md,../html/%.html,$(wildcard *.md))
	rm -f *.bak *~

../html/%.html: %.md
	$(MARKDOWN) $< --output $@
	sed -i 's/.md/.html/g' $@ 

