package mtree;

import java.util.List;
import java.util.Set;

import mtree.utils.Pair;
import mtree.utils.Utils;

public final class PromotionFunctions {

	private PromotionFunctions() {}
	
	public static class RandomPromotion<DATA> implements PromotionFunction<DATA> {
		
		@Override
		public Pair<DATA> process(Set<DATA> dataSet,
				DistanceFunction<? super DATA> distanceFunction)
		{
			List<DATA> promotedList = Utils.randomSample(dataSet, 2);
			return new Pair<DATA>(promotedList.get(0), promotedList.get(1));
		}

	}
	
}
