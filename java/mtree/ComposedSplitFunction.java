package mtree;

import java.util.Set;

import mtree.utils.Pair;

public class ComposedSplitFunction<DATA> implements SplitFunction<DATA> {

	private PromotionFunction<DATA> promotionFunction;
	private PartitionFunction<DATA> partitionFunction;

	public ComposedSplitFunction(
			PromotionFunction<DATA> promotionFunction,
			PartitionFunction<DATA> partitionFunction
		)
	{
		this.promotionFunction = promotionFunction;
		this.partitionFunction = partitionFunction;
	}

	
	@Override
	public Pair<DATA> process(Set<DATA> firstPartition, Set<DATA> secondPartition, DistanceFunction<? super DATA> distanceFunction) {
		Pair<DATA> promoted = promotionFunction.process(firstPartition, distanceFunction);
		partitionFunction.process(promoted, firstPartition, secondPartition, distanceFunction);
		return promoted;
	}

}
