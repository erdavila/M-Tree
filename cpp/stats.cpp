#include <ext/algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>
#include <cassert>
#include "word-distance.h"

using namespace std;


//const char DICT_FILE[] = "pt-br.dic";
const char DICT_FILE[] = "en.dic";

enum {
	REPETITIONS = 3,
	WORD_LIMIT = 1000000,
	NUM_TEST_WORDS = 3,
	RATE = 2,
	TOP_MIN_CAPACITY = 2000,
	TOP_LIMIT = 500,
};


WordMTree createMTree(const vector<string>& words, size_t minNodeCapacity) {
	cerr << "Creating M-Tree with minNodeCapacity=" << minNodeCapacity << endl;
	WordMTree mtree(minNodeCapacity);
	cerr << "Adding words...";
	Timer t;
	for(size_t i = 0; i < words.size(); ++i) {
		size_t n = i + 1;
		const string& word = words[i];
		mtree.add(word);
		if(n % 100 == 0) {
			cerr << "\r" << n << " words added...";
		}
	}
	Timer::Times times = t.getTimes();
	cerr << endl;
	cout <<      "CREATE-MTREE"
	        "\t" "minNodeCapacity" "=" << minNodeCapacity
	     << "\t" "userTime"        "=" << times.user
	     << "\t" "sysTime"         "=" << times.sys
	     << "\t" "realTime"        "=" << times.real
	     << endl;
	
	cerr << "M-Tree created" << endl;
	return mtree;
}



void test(const WordMTree& mtree, const vector<string>& testWords, size_t minNodeCapacity, size_t limit) {
	cerr << "Testing minNodeCapacity=" << minNodeCapacity << ", limit=" << limit << endl;
	for(auto i = testWords.begin(); i != testWords.end(); ++i) {
		auto testWord = *i;
		cerr << "testWord=\"" << testWord << "\"" << endl;
		Timer::Times totalTimes = {0, 0, 0};
		for(size_t _ = 0; _ < REPETITIONS; ++_) {
			Timer t;
			auto query = mtree.get_nearest_by_limit(testWord, limit);
			vector<WordMTree::query::result_item> results(query.begin(), query.end());
			Timer::Times times = t.getTimes();
			assert(results.size() == limit);
			totalTimes.real += times.real;
			totalTimes.sys  += times.sys;
			totalTimes.user += times.user;
		}
		
		double avgReal = totalTimes.real / double(REPETITIONS);
		double avgUser = totalTimes.user / double(REPETITIONS);
		double avgSys  = totalTimes.sys  / double(REPETITIONS);
		cout <<      "TEST"
		        "\t" "minNodeCapacity" "="      << minNodeCapacity
		     << "\t" "testWord"        "=" "\"" << testWord << "\""
		        "\t" "limit"           "="      << limit
		     << "\t" "avgReal"         "="      << avgReal
		     << "\t" "avgUser"         "="      << avgUser
		     << "\t" "avgSys"          "="      << avgSys
		     << endl;
	}
}





int main() {
	srand(time(NULL));

	cerr << "Loading words..." << endl;
	ifstream f(DICT_FILE);
	vector<string> words;
	while(!f.eof()) {
		if(words.size() >= WORD_LIMIT) {
			break;
		}

		string word;
		getline(f, word);
		if(word == "") {
			break;
		}

		if(word[0] != '%') {
			words.push_back(word);
		}
	}
	cerr << words.size() << " words loaded" << endl;

	vector<string> testWords;
	random_sample_n(words.begin(), words.end(), inserter(testWords, testWords.begin()), int(NUM_TEST_WORDS));
	cerr << "Test words: ";
	for(auto i = testWords.begin(); i != testWords.end(); ++i) {
		if(i != testWords.begin()) {
			cerr << ", ";
		}
		cerr << '"' << *i << '"';
	}
	cerr << endl;
	
	for(size_t minNodeCapacity = 2; minNodeCapacity < TOP_MIN_CAPACITY; minNodeCapacity *= RATE) {
		WordMTree mtree = createMTree(words, minNodeCapacity);
		
		for(size_t limit = 1; limit < TOP_LIMIT; limit *= RATE) {
			test(mtree, testWords, minNodeCapacity, limit);
		}
	}
}
