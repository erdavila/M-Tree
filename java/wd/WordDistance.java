package wd;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.Charset;


public class WordDistance {
	
	//static final String DICT_FILE = "pt-br.dic";
	static final String DICT_FILE = "en.dic";
	
	static int wordDistance(String word1, String word2) {
		word1 = word1.toLowerCase();
		word2 = word2.toLowerCase();
	
		int distanceTable[][] = new int[word1.length() + 1][word2.length() + 1];
	
		distanceTable[0][0] = 0;
		for(int len1 = 1; len1 <= word1.length(); ++len1) {
			distanceTable[len1][0] = len1;
		}
		for(int len2 = 1; len2 <= word2.length(); ++len2) {
			distanceTable[0][len2] = len2;
		}
	
		for(int len1 = 1; len1 <= word1.length(); ++len1) {
			for(int len2 = 1; len2 <= word2.length(); ++len2) {
				if(word1.charAt(len1-1) == word2.charAt(len2-1)) {
					distanceTable[len1][len2] = distanceTable[len1-1][len2-1];
				} else {
					distanceTable[len1][len2] = 1 + Math.min(Math.min(
							distanceTable[len1-1][len2-1],
							distanceTable[len1  ][len2-1]),
							distanceTable[len1-1][len2  ]
					);
				}
			}
		}
		
		int distance = distanceTable[word1.length()][word2.length()];
		return distance;
	}

	
	public static void main(String[] args) throws IOException {
		//System.out.println(wordDistance("gol", "bola"));
		//System.out.println(wordDistance(args[0], args[1]));
	
		int wordsLimit = (args.length > 0) ? Integer.parseInt(args[0]) : 1000000;
	
		WordMTree mtree = new WordMTree();
	
		int loadedWords = 0;
		Timer t = new Timer();
		System.out.println("Indexing...");
		BufferedReader f = new BufferedReader(new InputStreamReader(new FileInputStream(DICT_FILE), Charset.forName("UTF-8")));
		while(true) {
			String line = f.readLine();
			if(line == null) {
				break;
			}
	
			if(!line.isEmpty()  &&  line.charAt(0) != '%') {
				String word = line;
				mtree.add(word);
				loadedWords++;
				if(loadedWords >= wordsLimit) {
					break;
				}
				if(loadedWords % 100 == 0) {
					System.out.print("\r" + loadedWords + " words indexed");
				}
			}
		}
		System.out.println("\r" + loadedWords + " words indexed");
	
		Timer.Times times = t.getTimes();
		System.out.printf("TIMES: %.2freal\n\n", times.real / 1000000000.0);
	
		BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
		while(true) {
			System.out.print("Type a word: ");
			String word = in.readLine();
			if(word == null  ||  word.isEmpty()) {
				break;
			}
	
			t = new Timer();
			WordMTree.Query query = mtree.getNearestByLimit(word, 10);
			for(WordMTree.ResultItem ri : query) {
				System.out.println("\t" + ri.distance + " " + ri.data);
			}
			times = t.getTimes();
			System.out.printf("TIMES: %.2freal\n\n", times.real / 1000000000.0);
		}
	}
}

