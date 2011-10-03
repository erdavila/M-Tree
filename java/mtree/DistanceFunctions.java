package mtree;

import java.util.List;

public abstract class DistanceFunctions {
	private DistanceFunctions() {}
	
	
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
