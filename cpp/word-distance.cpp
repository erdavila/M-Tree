#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <ctime>
#include <sys/times.h>
#include "mtree.h"
using namespace std;


//const char DICT_FILE[] = "pt-br.dic";
const char DICT_FILE[] = "en.dic";


struct Times {
	double real;
	double user;
	double sys;
};

struct Timer {
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
		return (end - begin) / double(CLOCKS_PER_SEC);
	}
};



size_t wordDistance(string word1, string word2) {
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
				distanceTable[len1][len2] = 1 + min(min(
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



class WordMTree : public mtree::MTreeBase<string> {
	double distanceFunction(const string& word1, const string& word2) const {
		return wordDistance(word1, word2);
	}
};



int main(int argc, const char* argv[]) {
	//cout << wordDistance("gol", "bola") << endl;
	//cout << wordDistance(argv[1], argv[2]); return 0;

	size_t wordsLimit = (argc > 1) ? atoi(argv[1]) : 1000000;

	WordMTree mtree;

	size_t loadedWords = 0;
	Timer t;
	cout << "Indexing..." << flush;
	ifstream f(DICT_FILE);
	while(! f.eof()) {
		string line;
		getline(f, line);

		if(!line.empty()  &&  line[0] != '%') {
			//word = unicode(line.strip(), 'utf-8')	TODO: review
			auto word = line;
			mtree.add(word);
			loadedWords++;
			if(loadedWords >= wordsLimit) {
				break;
			}
			if(loadedWords % 100 == 0) {
				cout << '\r' << loadedWords << " words indexed" << flush;
			}
		}
	}
	cout << '\r' << loadedWords << " words indexed" << endl;

	Times times = t.getTimes();
	printf("TIMES: %0.2fuser %0.02fsys %0.2freal\n\n", times.user, times.sys, times.real);

	while(true) {
		//word = unicode(raw_input("Type a word: "), 'utf-8')	TODO: review
		cout << "Type a word: ";
		string word;
		getline(cin, word);
		if(word == "") {
			break;
		}

		Timer t;
		for(auto i = mtree.getNearestByLimit(word, 10); i != mtree.resultsEnd(); ++i) {
			cout << "\t" << i->distance << " " << i->data << endl;
		}
		Times times = t.getTimes();
		printf("TIMES: %0.2fuser %0.02fsys %0.2freal\n\n", times.user, times.sys, times.real);
	}
}
