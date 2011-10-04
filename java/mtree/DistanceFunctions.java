package mtree;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

public abstract class DistanceFunctions {
	private DistanceFunctions() {}
	
	
	public static <Data> DistanceFunction<Data> cached(final DistanceFunction<Data> distanceFunction) {
		return new DistanceFunction<Data>() {
			class Pair {
				Data data1;
				Data data2;
				
				public Pair(Data data1, Data data2) {
					this.data1 = data1;
					this.data2 = data2;
				}

				@Override
				public int hashCode() {
					return data1.hashCode() ^ data2.hashCode();
				}
				
				@Override
				public boolean equals(Object arg0) {
					if(arg0 instanceof Pair) {
						Pair that = (Pair) arg0;
						return this.data1.equals(that.data1)
						    && this.data2.equals(that.data2);
					} else {
						return false;
					}
				}
			}
			
			private final Map<Pair, Double> cache = new HashMap<Pair, Double>();
			
			@Override
			public double calculate(Data data1, Data data2) {
				Pair pair1 = new Pair(data1, data2);
				Double distance = cache.get(pair1);
				if(distance != null) {
					return distance;
				}
				
				Pair pair2 = new Pair(data2, data1);
				distance = cache.get(pair2);
				if(distance != null) {
					return distance;
				}
				
				distance = distanceFunction.calculate(data1, data2);
				cache.put(pair1, distance);
				cache.put(pair2, distance);
				return distance;
			}
		};
	}
	
	
	
	public interface NumberSequence {
		int size();
		double get(int index);
	}
	
	
	public static double euclidean(NumberSequence seq1, NumberSequence seq2) {
		int size = Math.min(seq1.size(), seq2.size());
		double distance = 0;
		for(int i = 0; i < size; i++) {
			double diff = seq1.get(i) - seq2.get(i);
			distance += diff * diff;
		}
		distance = Math.sqrt(distance);
		return distance;
	}


	public static final DistanceFunction<NumberSequence> EUCLIDEAN = new DistanceFunction<DistanceFunctions.NumberSequence>() {
		@Override
		public double calculate(NumberSequence data1, NumberSequence data2) {
			return DistanceFunctions.euclidean(data1, data2);
		}
	};
	
	
	public static final DistanceFunction<List<Integer>> EUCLIDEAN_INTEGER_LIST = new DistanceFunction<List<Integer>>() {
		@Override
		public double calculate(List<Integer> data1, List<Integer> data2) {
			class IntegerListNumberSequence implements NumberSequence {
				List<Integer> list;
				public IntegerListNumberSequence(List<Integer> list) { this.list = list; }
				@Override public int size() { return list.size(); }
				@Override public double get(int index) { return list.get(index); }
			};
			IntegerListNumberSequence seq1 = new IntegerListNumberSequence(data1);
			IntegerListNumberSequence seq2 = new IntegerListNumberSequence(data2);
			return DistanceFunctions.euclidean(seq1, seq2);
		}
	};
	
	public static final DistanceFunction<List<Double>> EUCLIDEAN_DOUBLE_LIST = new DistanceFunction<List<Double>>() {
		@Override
		public double calculate(List<Double> data1, List<Double> data2) {
			class DoubleListNumberSequence implements NumberSequence {
				List<Double> list;
				public DoubleListNumberSequence(List<Double> list) { this.list = list; }
				@Override public int size() { return list.size(); }
				@Override public double get(int index) { return list.get(index); }
			};
			DoubleListNumberSequence seq1 = new DoubleListNumberSequence(data1);
			DoubleListNumberSequence seq2 = new DoubleListNumberSequence(data2);
			return DistanceFunctions.euclidean(seq1, seq2);
		}
	};
}
