package wd;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.List;

import mtree.utils.Utils;

public class Stats {

	private static final int REPETITIONS = 3;
	private static final int WORD_LIMIT = 1000000;
	private static final int NUM_TEST_WORDS = 3;
	private static final int RATE = 2;
	private static final int TOP_MIN_CAPACITY = 2000;
	private static final int TOP_LIMIT = 500;
	
	
	static WordMTree createMTree(List<String> words, int minNodeCapacity) {
		System.err.println("Creating M-Tree with minNodeCapacity=" + minNodeCapacity);
		WordMTree mtree = new WordMTree(minNodeCapacity);
		System.err.print("Adding words...");
		Timer t = new Timer();
		for(int i = 0; i < words.size(); i++) {
			int n = i + 1;
			String word = words.get(i);
			mtree.add(word);
			if(n % 100 == 0) {
				System.err.print("\r" + n + " words added...");
			}
		}
		Timer.Times times = t.getTimes();
		System.err.println();
		System.out.println("CREATE-MTREE"
		     + "\t" + "minNodeCapacity" + "=" + minNodeCapacity
		     + "\t" + "realTime"        + "=" + times.real/1000000000.0
				);
		
		System.err.println("M-Tree created");
		return mtree;
	}

	
	
	static void test(WordMTree mtree, List<String> testWords, int minNodeCapacity, int limit) {
		System.err.println("Testing minNodeCapacity=" + minNodeCapacity + ", limit=" + limit);
		for(String testWord : testWords) {
			System.err.println("testWord=\"" + testWord + "\"");
			Timer.Times totalTimes = new Timer.Times(0);
			for(int _ = 0; _ < REPETITIONS; _++) {
				Timer t = new Timer();
				WordMTree.Query query = mtree.getNearestByLimit(testWord, limit);
				List<WordMTree.ResultItem> results = new ArrayList<WordMTree.ResultItem>();
				for(WordMTree.ResultItem ri : query) {
					results.add(ri);
				}
				Timer.Times times = t.getTimes();
				assert results.size() == limit;
				totalTimes.real += times.real;
			}
			
			double avgReal = totalTimes.real / (double)REPETITIONS;
			System.out.println("TEST"
			     + "\t" + "minNodeCapacity" + "="        + minNodeCapacity
			     + "\t" + "testWord"        + "=" + "\"" + testWord + "\""
			     + "\t" + "limit"           + "="        + limit
			     + "\t" + "avgReal"         + "="        + avgReal/1000000000.0
					);
		}
	}

	
	
	public static void main(String[] args) throws IOException {
		System.err.println("Loading words...");
		BufferedReader f = new BufferedReader(new InputStreamReader(new FileInputStream(WordDistance.DICT_FILE), Charset.forName("UTF-8")));
		List<String> words = new ArrayList<String>();
		while(true) {
			if(words.size() >= WORD_LIMIT) {
				break;
			}
	
			String word = f.readLine();
			if(word == null  ||  word.isEmpty()) {
				break;
			}
	
			if(word.charAt(0) != '%') {
				words.add(word);
			}
		}
		System.err.println(words.size() + " words loaded");
	
		List<String> testWords = Utils.randomSample(words, NUM_TEST_WORDS);
		System.err.print("Test words: ");
		for(int i = 0; i < testWords.size(); i++) {
			if(i > 0) {
				System.err.print(", ");
			}
			System.err.print('"' + testWords.get(i) + '"');
		}
		System.err.println();
		
		for(int minNodeCapacity = 2; minNodeCapacity < TOP_MIN_CAPACITY; minNodeCapacity *= RATE) {
			WordMTree mtree = createMTree(words, minNodeCapacity);
			
			for(int limit = 1; limit < TOP_LIMIT; limit *= RATE) {
				test(mtree, testWords, minNodeCapacity, limit);
			}
		}
	}

}
