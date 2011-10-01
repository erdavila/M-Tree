#ifndef WORD_DISTANCE_H_
#define WORD_DISTANCE_H_


#include <algorithm>
#include <string>
#include <ctime>
#include <unistd.h>
#include <sys/times.h>
#include "mtree.h"


size_t wordDistance(std::string word1, std::string word2) {
	transform(word1.begin(), word1.end(), word1.begin(), ::tolower);
	transform(word2.begin(), word2.end(), word2.begin(), ::tolower);

	size_t distanceTable[word1.size() + 1][word2.size() + 1];

	distanceTable[0][0] = 0;
	for(size_t len1 = 1; len1 <= word1.size(); ++len1) {
		distanceTable[len1][0] = len1;
	}
	for(size_t len2 = 1; len2 <= word2.size(); ++len2) {
		distanceTable[0][len2] = len2;
	}

	for(size_t len1 = 1; len1 <= word1.size(); ++len1) {
		for(size_t len2 = 1; len2 <= word2.size(); ++len2) {
			if(word1[len1-1] == word2[len2-1]) {
				distanceTable[len1][len2] = distanceTable[len1-1][len2-1];
			} else {
				distanceTable[len1][len2] = 1 + std::min(std::min(
						distanceTable[len1-1][len2-1],
						distanceTable[len1  ][len2-1]),
						distanceTable[len1-1][len2  ]
				);
			}
		}
	}

	size_t distance = distanceTable[word1.size()][word2.size()];
	return distance;
}



typedef mt::mtree<std::string, size_t(*)(std::string,std::string)> MTree;
class WordMTree : public MTree {
public:
	WordMTree(WordMTree&&);

	WordMTree(size_t minNodeCapacity = MTree::DEFAULT_MIN_NODE_CAPACITY)
		: MTree(minNodeCapacity, -1, wordDistance)
		{}
};



class Timer {
public:
	struct Times {
		double real;
		double user;
		double sys;
	};

	
	tms timesBegin;
	clock_t timeBegin;

	Timer() : timeBegin(times(&timesBegin)) {}

	Times getTimes() {
		tms timesEnd;
		clock_t timeEnd = times(&timesEnd);
		return Times {
			clockDiff(timeBegin, timeEnd),
			clockDiff(timesBegin.tms_utime, timesEnd.tms_utime),
			clockDiff(timesBegin.tms_stime, timesEnd.tms_stime),
		};
	}
	
	
private:
	static double clockDiff(clock_t begin, clock_t end) {
		static long clockTicksPerSecond = sysconf(_SC_CLK_TCK);
		return double(end - begin) / clockTicksPerSecond;
	}
};


#endif /* WORD_DISTANCE_H_ */
