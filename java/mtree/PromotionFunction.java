package mtree;

import java.util.Set;

import mtree.utils.Pair;

public interface PromotionFunction<DATA> {
	
	Pair<DATA> process(Set<DATA> dataSet, DistanceFunction<? super DATA> distanceFunction);
	
}
