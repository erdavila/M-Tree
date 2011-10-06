package wd;

import mtree.MTree;

class WordMTree extends MTree<String> {
	
	public WordMTree() {
		super(MTree.DEFAULT_MIN_NODE_CAPACITY, new WordDistanceFunction(), null);
	}
}