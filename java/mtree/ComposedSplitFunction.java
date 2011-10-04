package mtree;

import java.util.Set;

import mtree.utils.Pair;

/**
 * A {@linkplain SplitFunction split function} that is defined by composing
 * a {@linkplain PromotionFunction promotion function} and a
 * {@linkplain PartitionFunction partition function}.
 *
 * @param <DATA> The type of the data objects.
 */
public class ComposedSplitFunction<DATA> implements SplitFunction<DATA> {

	private PromotionFunction<DATA> promotionFunction;
	private PartitionFunction<DATA> partitionFunction;

	/**
	 * The constructor of a {@link SplitFunction} composed by a
	 * {@link PromotionFunction} and a {@link PartitionFunction}.
	 */
	public ComposedSplitFunction(
			PromotionFunction<DATA> promotionFunction,
			PartitionFunction<DATA> partitionFunction
		)
	{
		this.promotionFunction = promotionFunction;
		this.partitionFunction = partitionFunction;
	}

	
	@Override
	public SplitResult<DATA> process(Set<DATA> dataSet, DistanceFunction<? super DATA> distanceFunction) {
		Pair<DATA> promoted = promotionFunction.process(dataSet, distanceFunction);
		Pair<Set<DATA>> partitions = partitionFunction.process(promoted, dataSet, distanceFunction);
		return new SplitResult<DATA>(promoted, partitions);
	}

}
