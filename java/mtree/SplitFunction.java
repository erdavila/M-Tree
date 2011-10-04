package mtree;

import java.util.Set;

public interface SplitFunction<DATA> {
	
	DATA[] process(Set<DATA> firstPartition, Set<DATA> secondPartition, DistanceFunction<? super DATA> cachedDistanceFunction);
	
}
