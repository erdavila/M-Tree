CPPOPTS:=-Wall -std=c++0x -fmessage-length=0

ifeq ($(DEBUG),1)
 CPPOPTS+=-O0 -g3
endif


.PHONY:
all:               \
	test_mtreebase \
	word-distance  \
	stats


# Header dependencies
test_mtreebase word-distance stats: \
	cpp/mtree.h \
	cpp/functions.h

word-distance stats: \
	cpp/word-distance.h



# Building each executable
%: cpp/%.cpp
	g++ $(CPPOPTS)  $<  -o $@



.PHONY:
clean:
	rm -f test_mtreebase word-distance stats
