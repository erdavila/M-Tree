CPPOPTS:=-std=c++0x -fmessage-length=0 -O0 -g3 -Wall


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



# Building 
%: cpp/%.cpp
	g++ $(CPPOPTS)  $<  -o $@



.PHONY:
clean:
	rm test_mtreebase word-distance stats
