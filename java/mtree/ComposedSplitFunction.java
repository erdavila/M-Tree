package mtree;

import java.util.Set;

public class ComposedSplitFunction<DATA> implements SplitFunction<DATA> {

	private PromotionFunction<DATA> promotionFunction;
	private PartitionFunction<DATA> partitionFunction;

	public ComposedSplitFunction(PromotionFunction<DATA> promotionFunction,
			PartitionFunction<DATA> partitionFunction)
	{
		this.promotionFunction = promotionFunction;
		this.partitionFunction = partitionFunction;
	}

	@Override
	public DATA[] process(Set<DATA> firstPartition, Set<DATA> secondPartition, DistanceFunction<? super DATA> distanceFunction) {
		DATA[] promoted = promotionFunction.process(firstPartition, distanceFunction);
		partitionFunction.process(promoted, firstPartition, secondPartition, distanceFunction);
		return promoted;
	}

}
