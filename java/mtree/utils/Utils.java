package mtree.utils;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.Random;

public final class Utils {

	private Utils() {}

	
	public static <T extends Comparable<T>> Pair<T> minMax(Iterable<T> items) {
		Iterator<T> iterator = items.iterator();
		if(!iterator.hasNext()) {
			return null;
		}
		
		T min = iterator.next();
		T max = min;
		
		while(iterator.hasNext()) {
			T item = iterator.next();
			if(item.compareTo(min) < 0) {
				min = item;
			}
			if(item.compareTo(max) > 0) {
				max = item;
			}
		}
		
		return new Pair<T>(min, max);
	}
	
	
	public static <T> List<T> randomSample(Collection<T> collection, int n) {
		List<T> list = new ArrayList<T>(collection);
		List<T> sample = new ArrayList<T>(n);
		Random random = new Random();
		while(n > 0  &&  !list.isEmpty()) {
			int index = random.nextInt(list.size());
			sample.add(list.get(index));
			int indexLast = list.size() - 1;
			T last = list.remove(indexLast);
			if(index < indexLast) {
				list.set(index, last);
			}
			n--;
		}
		return sample;
	}
	
}
