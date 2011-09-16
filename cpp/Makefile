CPPOPTS:=-Wall -std=c++0x -fmessage-length=0

ifeq ($(DEBUG),1)
 CPPOPTS+=-O0 -g3
else
 CPPOPTS+=-DNDEBUG
endif


.PHONY:
all:               \
	test_mtree     \
	word-distance  \
	stats


# Header dependencies
test_mtree  word-distance  stats  :  mtree.h  functions.h

word-distance  stats :  word-distance.h



# Building each executable
%: %.cpp
	g++ $(CPPOPTS)  $<  -o $@



.PHONY:
clean:
	rm -f test_mtree word-distance stats
