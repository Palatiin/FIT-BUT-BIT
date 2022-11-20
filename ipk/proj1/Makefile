FILE=xremen01
PROG=hinfosvc
COMP=g++ -std=c++11

.PHONY: all clean pack

$(FILE): $(FILE).cpp
	$(COMP) $(FILE).cpp -o $(PROG)

pack:
	zip $(FILE).zip $(FILE).cpp Makefile Readme.md

clean:
	rm $(PROG)
