#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include "word-distance.h"
#include "mtree.h"
using namespace std;


//const char DICT_FILE[] = "pt-br.dic";
const char DICT_FILE[] = "en.dic";



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
			//word = unicode(line.strip(), 'utf-8')	TODO: implement encoding
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

	Timer::Times times = t.getTimes();
	printf("TIMES: %0.2fuser %0.02fsys %0.2freal\n\n", times.user, times.sys, times.real);

	while(true) {
		//word = unicode(raw_input("Type a word: "), 'utf-8')	TODO: implement encoding
		cout << "Type a word: ";
		string word;
		getline(cin, word);
		if(word == "") {
			break;
		}

		Timer t;
		auto query = mtree.get_nearest_by_limit(word, 10);
		for(auto i = query.begin(); i != query.end(); ++i) {
			cout << "\t" << i->distance << " " << i->data << endl;
		}
		Timer::Times times = t.getTimes();
		printf("TIMES: %0.2fuser %0.02fsys %0.2freal\n\n", times.user, times.sys, times.real);
	}
}
