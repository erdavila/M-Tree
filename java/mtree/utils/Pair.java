package mtree.utils;

/**
 * A pair of objects of the same type.
 *
 * @param <T> The type of the objects.
 */
public class Pair<T> {
	
	/**
	 * The first object.
	 */
	public T first;
	
	
	/**
	 * The second object.
	 */
	public T second;
	
	/**
	 * Creates a pair of {@code null} objects.
	 */
	public Pair() {}
	
	/**
	 * Creates a pair with the objects specified in the arguments.
	 * @param first  The first object.
	 * @param second The second object.
	 */
	public Pair(T first, T second) {
		this.first = first;
		this.second = second;
	}

	/**
	 * Accesses an object by its index. The {@link #first} object has index
	 * {@code 0} and the {@link #second} object has index {@code 1}.
	 * @param index The index of the object to be accessed.
	 * @return The {@link #first} object if {@code index} is {@code 0}; the
	 *         {@link #second} object if {@code index} is {@code 1}.
	 * @throws IllegalArgumentException If {@code index} is neither {@code 0}
	 *         or {@code 1}.
	 */
	public T get(int index) throws IllegalArgumentException {
		switch(index) {
		case 0: return first;
		case 1: return second;
		default: throw new IllegalArgumentException();
		}
	}

}
