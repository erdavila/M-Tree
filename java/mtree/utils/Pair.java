package mtree.utils;

public class Pair<T> {
	
	public T first;
	public T second;
	
	public Pair() {}
	
	public Pair(T first, T second) {
		this.first = first;
		this.second = second;
	}

	public T get(int i) {
		switch(i) {
		case 0: return first;
		case 1: return second;
		default: throw new IllegalArgumentException();
		}
	}

}
