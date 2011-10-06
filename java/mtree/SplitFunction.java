package mtree;

import java.util.Set;

import mtree.utils.Pair;

public interface SplitFunction<DATA> {
	
	Pair<DATA> process(Set<DATA> firstPartition, Set<DATA> secondPartition, DistanceFunction<? super DATA> cachedDistanceFunction);
	
}
