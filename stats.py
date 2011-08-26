#!/usr/bin/env python
import random
import sys
import time
from mtree import MTreeBase
import word_distance



REPETITIONS = 3
NUM_TEST_WORDS = 1
RATE = 2

WORD_LIMIT = None #or 1000



def rate(start, end, rate_):
	r = start
	while r < end:
		yield r
		r *= rate_



def create_mtree(words, min_node_capacity):
	print >>sys.stderr, "Creating M-Tree with min_node_capacity=%r" % min_node_capacity
	mtree = MTreeBase(min_node_capacity=min_node_capacity, distance_function = word_distance.word_distance)
	print >>sys.stderr, "Adding words...",
	for n, word in enumerate(words if (WORD_LIMIT is None) else words[:WORD_LIMIT], 1):
		mtree.add(word)
		if n % 100 == 0:
			print >>sys.stderr, "\r%r words added..." % n,
	
	print >>sys.stderr, "M-Tree created"
	return mtree



def test(mtree, test_words, min_node_capacity, limit):
	print >>sys.stderr, "Testing min_node_capacity=%r, limit=%r" % (min_node_capacity, limit)
	for test_word in test_words:
		print >>sys.stderr, "test_word=%r" % test_word
		total_time = 0
		for _ in range(REPETITIONS):
			b = time.clock()
			results = list(mtree.get_nearest(test_word, limit=limit))
			e = time.clock()
			assert len(results) == limit
			total_time += (e - b)
		
		avg_time = total_time / REPETITIONS
		
		print "\t".join([
				"min_node_capacity=%d" % min_node_capacity,
				"test_word=%r" % test_word,
				"limit=%d" % limit,
				"avg_time=%r" % avg_time
		])



def main():
	print >>sys.stderr, 'Loading words...'
	with open(word_distance.DICT_FILE) as f:
		words = [line.strip() for line in f.readlines() if line[0] != '%']
	print >>sys.stderr, '%d words loaded' % len(words)
	
	test_words = random.sample(words, NUM_TEST_WORDS)
	print >>sys.stderr, 'Test words:', test_words
	
	for min_node_capacity in (int(r) for r in rate(2, 2000, RATE)):
		mtree = create_mtree(words, min_node_capacity)
		
		for limit in (int(r) for r in rate(1, 500, RATE)):
			test(mtree, test_words, min_node_capacity, limit)



if __name__ == "__main__":
	main()
