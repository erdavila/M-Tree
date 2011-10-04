package mtree;

import java.util.Set;

public interface PartitionFunction <DATA> {
	
	void process(DATA[] promoted, Set<DATA> firstPartition, Set<DATA> secondPartition, DistanceFunction<? super DATA> distanceFunction);
	
}
