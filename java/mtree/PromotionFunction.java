package mtree;

import java.util.Set;

public interface PromotionFunction<DATA> {
	
	DATA[] process(Set<DATA> dataSet, DistanceFunction<? super DATA> distanceFunction);
	
}
