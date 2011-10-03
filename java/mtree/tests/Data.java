package mtree.tests;

import mtree.DistanceFunctions.NumberSequence;

class Data implements NumberSequence {
	
	private final int[] values;
	private final int hashCode;
	
	Data(int... values) {
		this.values = values;
		
		int hashCode = 1;
		for(int value : values) {
			hashCode = 31*hashCode + value;
		}
		this.hashCode = hashCode;
	}
	
	@Override
	public int size() {
		return values.length;
	}

	@Override
	public double get(int index) {
		return values[index];
	}
	
	@Override
	public int hashCode() {
		return hashCode;
	}
	
	@Override
	public boolean equals(Object obj) {
		if(obj instanceof Data) {
			Data that = (Data) obj;
			if(this.size() != that.size()) {
				return false;
			}
			for(int i = 0; i < this.size(); i++) {
				if(this.values[i] != that.values[i]) {
					return false;
				}
			}
			return true;
		} else {
			return false;
		}
	}
	
}