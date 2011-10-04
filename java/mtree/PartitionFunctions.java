package mtree;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Set;

public final class PartitionFunctions {

	private PartitionFunctions() {}
	
	
	public static <DATA> PartitionFunction<DATA> getBalancedPartition(Class<DATA> cls) {
		return new PartitionFunction<DATA>() {
			@Override
			public void process(final DATA[] promoted,
					Set<DATA> firstPartition, Set<DATA> secondPartition,
					final DistanceFunction<? super DATA> distanceFunction)
			{
				List<DATA> queue1 = new ArrayList<DATA>(firstPartition);
				// Sort by distance to the first promoted data
				Collections.sort(queue1, new Comparator<DATA>() {
					@Override
					public int compare(DATA data1, DATA data2) {
						double distance1 = distanceFunction.calculate(data1, promoted[0]);
						double distance2 = distanceFunction.calculate(data2, promoted[0]);
						return Double.compare(distance1, distance2);
					}
				});
				
				List<DATA> queue2 = new ArrayList<DATA>(firstPartition);
				// Sort by distance to the second promoted data
				Collections.sort(queue2, new Comparator<DATA>() {
					@Override
					public int compare(DATA data1, DATA data2) {
						double distance1 = distanceFunction.calculate(data1, promoted[1]);
						double distance2 = distanceFunction.calculate(data2, promoted[1]);
						return Double.compare(distance1, distance2);
					}
				});
				
				firstPartition.clear();
				
				int index1 = 0;
				int index2 = 0;
		
				while(index1 < queue1.size()  ||  index2 != queue2.size()) {
					while(index1 < queue1.size()) {
						DATA data = queue1.get(index1++);
						if(!secondPartition.contains(data)) {
							firstPartition.add(data);
							break;
						}
					}
		
					while(index2 < queue2.size()) {
						DATA data = queue2.get(index2++);
						if(!firstPartition.contains(data)) {
							secondPartition.add(data);
							break;
						}
					}
				}
			}
		};
	}
}
