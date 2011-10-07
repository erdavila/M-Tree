package mtree;

import java.util.Set;

import mtree.utils.Pair;

public interface PartitionFunction<DATA> {
	
	void process(Pair<DATA> promoted, Set<DATA> firstPartition, Set<DATA> secondPartition, DistanceFunction<? super DATA> distanceFunction);
	
}
