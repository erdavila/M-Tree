#!/usr/bin/env python
import sys
import time
from mtree import MTree


#DICT_FILE = 'pt-br.dic'
DICT_FILE = 'en.dic'


class Timer(object):
	def __init__(self):
		self.clock = time.clock()
		self.time = time.time()
	
	def getTimes(self):
		c = time.clock()
		t = time.time()
		return (c - self.clock, t - self.time)



def word_distance(word1, word2):
	word1 = word1.lower()
	word2 = word2.lower()
	
	
	distance = [
		[None for _ in range(len(word2)+1)]
			for _ in range(len(word1)+1)
	]
	
	distance[0][0] = 0
	for len1 in range(1, len(word1)+1):
		distance[len1][0] = len1
	for len2 in range(1, len(word2)+1):
		distance[0][len2] = len2
	
	for len1 in range(1, len(word1)+1):
		for len2 in range(1, len(word2)+1):
			if word1[len1-1] == word2[len2-1]:
				distance[len1][len2] = distance[len1-1][len2-1]
			else:
				distance[len1][len2] = 1 + min([
						distance[len1-1][len2-1],
						distance[len1  ][len2-1],
						distance[len1-1][len2  ],
				])
	
	return distance[len(word1)][len(word2)]



def main():
	#print word_distance('gol', 'bola')
	#print word_distance(*sys.argv[1:])
	
	words_limit = int(sys.argv[1])
	
	mtree = MTree(distance_function=word_distance)
	
	loaded_words = 0
	t = Timer()
	print 'Indexing...', ; sys.stdout.flush()
	with open(DICT_FILE) as f:
		for line in f:
			if line[0] != '%':
				word = unicode(line.strip(), 'utf-8')
				#print "Adding %r (%s)" % (word, word)
				mtree.add(word)
				loaded_words += 1
				if loaded_words >= words_limit:
					break
				if loaded_words % 100 == 0:
					print '\r%d words indexed' % loaded_words, ; sys.stdout.flush()
	print '\r%d words indexed' % loaded_words
	times = t.getTimes()
	print 'TIMES: %0.2fuser %0.2ftotal' % times
	print 
	
	while True:
		word = unicode(raw_input("Type a word: "), 'utf-8')
		t = Timer()
		for near in mtree.get_nearest(word, limit=10):
			print '\t%d %s' % (near.distance, near.data)
		times = t.getTimes()
		print 'TIMES: %0.2fuser %0.2ftotal' % times
		print



if __name__ == '__main__':
	main()
