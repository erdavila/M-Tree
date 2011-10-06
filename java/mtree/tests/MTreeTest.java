package mtree.tests;

import static org.junit.Assert.*;

import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.NoSuchElementException;
import java.util.Set;

import mtree.ComposedSplitFunction;
import mtree.DistanceFunction;
import mtree.DistanceFunctions;
import mtree.MTree;
import mtree.PartitionFunctions;
import mtree.PromotionFunction;
import mtree.utils.Pair;
import mtree.utils.Utils;

import org.junit.Test;


class MTreeClass extends MTree<Data> {
	
	private static final PromotionFunction<Data> nonRandomPromotion = new PromotionFunction<Data>() {
		@Override
		public Pair<Data> process(Set<Data> dataSet, DistanceFunction<? super Data> distanceFunction) {
			return Utils.minMax(dataSet);
		}
	};
	
	
	MTreeClass() {
		super(2, DistanceFunctions.EUCLIDEAN, 
			new ComposedSplitFunction<Data>(
				nonRandomPromotion,
				new PartitionFunctions.BalancedPartition<Data>()
			)
		);
	}

	public void add(Data data) {
		super.add(data);
		_check();
	}

	public boolean remove(Data data) {
		boolean result = super.remove(data);
		_check();
		return result;
	}
	
	DistanceFunction<? super Data> getDistanceFunction() {
		return distanceFunction;
	}
};



public class MTreeTest {

	
	@Test
	public void testEmpty() {
		_checkNearestByRange(new Data(1, 2, 3), 4);
		_checkNearestByLimit(new Data(1, 2, 3), 4);
	}
	
	@Test public void test01() { _test("f01"); }
	@Test public void test02() { _test("f02"); }
	@Test public void test03() { _test("f03"); }
	@Test public void test04() { _test("f04"); }
	@Test public void test05() { _test("f05"); }
	@Test public void test06() { _test("f06"); }
	@Test public void test07() { _test("f07"); }
	@Test public void test08() { _test("f08"); }
	@Test public void test09() { _test("f09"); }
	@Test public void test10() { _test("f10"); }
	@Test public void test11() { _test("f11"); }
	@Test public void test12() { _test("f12"); }
	@Test public void test13() { _test("f13"); }
	@Test public void test14() { _test("f14"); }
	@Test public void test15() { _test("f15"); }
	@Test public void test16() { _test("f16"); }
	@Test public void test17() { _test("f17"); }
	@Test public void test18() { _test("f18"); }
	@Test public void test19() { _test("f19"); }
	@Test public void test20() { _test("f20"); }
	
	@Test public void testLots() { _test("fLots"); }

	
	@Test
	public void testRemoveNonExisting() {
		// Empty
		assert !mtree.remove(new Data(99, 77));

		// With some items
		mtree.add(new Data(4, 44));
		assert(!mtree.remove(new Data(99, 77)));

		mtree.add(new Data(95, 43));
		assert(!mtree.remove(new Data(99, 77)));

		mtree.add(new Data(76, 21));
		assert(!mtree.remove(new Data(99, 77)));

		mtree.add(new Data(64, 53));
		assert(!mtree.remove(new Data(99, 77)));

		mtree.add(new Data(47, 3));
		assert(!mtree.remove(new Data(99, 77)));

		mtree.add(new Data(26, 11));
		assert(!mtree.remove(new Data(99, 77)));
	}

	
	@Test public void testGeneratedCase01() { _test("fG01"); }
	@Test public void testGeneratedCase02() { _test("fG02"); }

	@Test
	public void testNotRandom() {
		/*
		 * To generate a random test, execute the following commands:
		 * 		py/mtree/tests/fixtures/generator.py -a500 -r0.2 > py/mtree/tests/fixtures/fNotRandom.py
		 * 		cpp/convert-fixture-to-cpp.py fNotRandom > cpp/tests/fixtures/fNotRandom.txt
		 */

		String fixtureName = "fNotRandom";
		String fixtureFileName = Fixture.path(fixtureName);
		File fixtureFile = new File(fixtureFileName);
		if(!fixtureFile.exists()) {
			throw new RuntimeException("The file " + fixtureFile + " does not exist");
		}
		_test(fixtureName);
	}


	private void assertIterator(
			int expectedData,
			double expectedDistance,
			boolean expectedHasNext,
			MTree<Integer>.ResultItem ri,
			Iterator<MTree<Integer>.ResultItem> i
		)
	{
		assertEquals(expectedData, ri.data.intValue());
		assertEquals(expectedDistance, ri.distance, 0.0);
		assertEquals(expectedHasNext, i.hasNext());
		if(!expectedHasNext) {
			try {
				i.next();
				fail();
			} catch(NoSuchElementException e) {
				assertTrue(true);
			}
		}
	}
	
	
	@Test
	public void testIterators() {
		MTree<Integer> mt = new MTree<Integer>(
				new DistanceFunction<Integer>() {
					@Override
					public double calculate(Integer data1, Integer data2) {
						return Math.abs(data1 - data2);
					}
				},
				null
			);

		mt.add(1);
		mt.add(2);
		mt.add(3);
		mt.add(4);

		MTree<Integer>.Query query = mt.getNearest(0);

		// The first iterator
		Iterator<MTree<Integer>.ResultItem> i1 = query.iterator();
		assertTrue(i1.hasNext());
		MTree<Integer>.ResultItem ri1 = i1.next();
		/*     1  2  3  4  e
		 * i1: *
		 */
		assertIterator(1, 1, true, ri1, i1);
		assertTrue(i1.hasNext());

		// Advance the iterator
		ri1 = i1.next();
		/*     1  2  3  4  e
		 * i1:    *
		 */
		assertIterator(2, 2, true, ri1, i1);

		// Advance again
		ri1 = i1.next();
		/*     1  2  3  4  e
		 * i1:       *
		 */
		assertIterator(3, 3, true, ri1, i1);

		// Begin another iteration
		Iterator<MTree<Integer>.ResultItem> i2 = query.iterator();
		assertTrue(i2.hasNext());
		MTree<Integer>.ResultItem ri2 = i2.next();
		/*     1  2  3  4  e
		 * i1:       *
		 * i2: *
		 */
		assertIterator(1, 1, true, ri2, i2);
		// The first iterator must not have been affected
		assertIterator(3, 3, true, ri1, i1);

		// Advance the second iterator
		ri2 = i2.next();
		/*     1  2  3  4  e
		 * i1:       *
		 * i2:    *
		 */
		assertIterator(2, 2, true, ri2, i2);
		// The first iterator must not have been affected
		assertIterator(3, 3, true, ri1, i1);

		// Now continue until the iterators reach the end
		ri1 = i1.next();
		/*     1  2  3  4  e
		 * i1:          *
		 * i2:    *
		 */
		assertIterator(4, 4, false, ri1, i1);
		assertIterator(2, 2, true,  ri2, i2);

		ri2 = i2.next();
		/*     1  2  3  4  e
		 * i1:          *
		 * i2:       *
		 */
		assertIterator(4, 4, false, ri1, i1);
		assertIterator(3, 3, true,  ri2, i2);

		ri2 = i2.next();
		/*     1  2  3  4  e
		 * i1:          *
		 * i2:          *
		 */
		assertIterator(4, 4, false, ri1, i1);
		assertIterator(4, 4, false, ri2, i2);
	}


	
	private MTreeClass mtree = new MTreeClass();
	private Set<Data> allData = new HashSet<Data>();

	
	private void _test(String fixtureName) {
		Fixture fixture = Fixture.load(fixtureName);
		_testFixture(fixture);
	}


	private void _testFixture(Fixture fixture) {
		for(Fixture.Action action : fixture.actions) {
			switch(action.cmd) {
			case 'A':
				allData.add(action.data);
				mtree.add(action.data);
				break;
			case 'R':
				allData.remove(action.data);
				boolean removed = mtree.remove(action.data);
				assert removed;
				break;
			default:
				throw new RuntimeException(Character.toString(action.cmd));
			}

			_checkNearestByRange(action.queryData, action.radius);
			_checkNearestByLimit(action.queryData, action.limit);
		}
	}

	private void _checkNearestByRange(Data queryData, double radius) {
		List<MTreeClass.ResultItem> results = new ArrayList<MTreeClass.ResultItem>();
		Set<Data> strippedResults = new HashSet<Data>();
		MTreeClass.Query query = mtree.getNearestByRange(queryData, radius);

		for(MTreeClass.ResultItem ri : query) {
			results.add(ri);
			strippedResults.add(ri.data);
		}

		double previousDistance = 0;

		for(MTreeClass.ResultItem result : results) {
			// Check if increasing distance
			assertTrue(previousDistance <= result.distance);
			previousDistance = result.distance;
			
			// Check if every item in the results came from the generated queryData
			assertTrue(allData.contains(result.data));

			// Check if every item in the results is within the range
			assertTrue(result.distance <= radius);
			assertEquals(mtree.getDistanceFunction().calculate(result.data, queryData), result.distance, 0.0);
		}

		for(Data data : allData) {
			double distance = mtree.getDistanceFunction().calculate(data, queryData);
			if(distance <= radius) {
				assertTrue(strippedResults.contains(data));
			} else {
				assertFalse(strippedResults.contains(data));
			}
		}
	}


	private void _checkNearestByLimit(Data queryData, int limit) {
		List<MTreeClass.ResultItem> results = new ArrayList<MTreeClass.ResultItem>();
		Set<Data> strippedResults = new HashSet<Data>();
		MTreeClass.Query query = mtree.getNearestByLimit(queryData, limit);

		for(MTreeClass.ResultItem ri : query) {
			results.add(ri);
			strippedResults.add(ri.data);
		}
		
		if(limit <= allData.size()) {
			assertEquals(limit, results.size());
		} else {
			// limit > allData.size()
			assertEquals(allData.size(), results.size());
		}
		
		double farthest = 0.0;
		double previousDistance = 0.0;
		for(MTreeClass.ResultItem ri : results) {
			// Check if increasing distance
			assertTrue(previousDistance <= ri.distance);
			previousDistance = ri.distance;

			// Check if every item in the results came from the generated queryData
			assertTrue(allData.contains(ri.data));

			// Check if items are not repeated
			assertEquals(1, Collections.frequency(strippedResults, ri.data));

			double distance = mtree.getDistanceFunction().calculate(ri.data, queryData);
			assertEquals(distance, ri.distance, 0.0);
			farthest = Math.max(farthest, distance);
		}
		for(Data data : allData) {
			double distance = mtree.getDistanceFunction().calculate(data, queryData);
			if(distance < farthest) {
				assertTrue(strippedResults.contains(data));
			} else if(distance > farthest) {
				assertFalse(strippedResults.contains(data));
			} else {
				// distance == farthest
			}
		}
	}
}
