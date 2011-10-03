package mtree.tests;

import static org.junit.Assert.*;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import mtree.DistanceFunction;
import mtree.DistanceFunctions;
import mtree.MTree;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;



/*
typedef vector<int> Data;
typedef set<Data> DataSet;
typedef mt::functions::cached_distance_function<Data, mt::functions::euclidean_distance> CachedDistanceFunction;
typedef pair<Data, Data>(*PromotionFunction)(const DataSet&, CachedDistanceFunction&);

PromotionFunction nonRandomPromotion =
	[](const DataSet& dataSet, CachedDistanceFunction&) -> pair<Data, Data> {
		vector<Data> dataObjects(dataSet.begin(), dataSet.end());
		sort(dataObjects.begin(), dataObjects.end());
		return {dataObjects.front(), dataObjects.back()};
	};


typedef mt::mtree<
		Data,
		mt::functions::euclidean_distance,
		mt::functions::split_function<
				PromotionFunction,
				mt::functions::balanced_partition
			>
	>
	MTree;

*/

class MTreeClass extends MTree<Data> {

	/*
public:
	// Turning the member public
	using MTree::distance_function;
	 */

	MTreeClass() {
		super(2, DistanceFunctions.EUCLIDEAN);
	}

	public void add(Data data) {
		try {
			super.add(data);
		} finally {
			_check();
		}
	}

	public boolean remove(Data data) {
		try {
			return super.remove(data);
		} finally {
			_check();
		}
	}
	
	DistanceFunction<? super Data> getDistanceFunction() {
		return distanceFunction;
	}
};



public class MTreeTest {

	@Before
	public void setUp() throws Exception {
	}

	@After
	public void tearDown() throws Exception {
	}

	@Test
	public void testEmpty() {
		_checkNearestByRange(new Data(1, 2, 3), 4);
		_checkNearestByLimit(new Data(1, 2, 3), 4);
	}
	
	@Test public void test01() { _test("f01"); }

/*

public:
	void test02() { _test("f02"); }
	void test03() { _test("f03"); }
	void test04() { _test("f04"); }
	void test05() { _test("f05"); }
	void test06() { _test("f06"); }
	void test07() { _test("f07"); }
	void test08() { _test("f08"); }
	void test09() { _test("f09"); }
	void test10() { _test("f10"); }
	void test11() { _test("f11"); }
	void test12() { _test("f12"); }
	void test13() { _test("f13"); }
	void test14() { _test("f14"); }
	void test15() { _test("f15"); }
	void test16() { _test("f16"); }
	void test17() { _test("f17"); }
	void test18() { _test("f18"); }
	void test19() { _test("f19"); }
	void test20() { _test("f20"); }
	void testLots() { _test("fLots"); }


	void testRemoveNonExisting() {
		// Empty
		assert(!mtree.remove({99, 77}));

		// With some items
		mtree.add({4, 44});
		assert(!mtree.remove({99, 77}));

		mtree.add({95, 43});
		assert(!mtree.remove({99, 77}));

		mtree.add({76, 21});
		assert(!mtree.remove({99, 77}));

		mtree.add({64, 53});
		assert(!mtree.remove({99, 77}));

		mtree.add({47, 3});
		assert(!mtree.remove({99, 77}));

		mtree.add({26, 11});
		assert(!mtree.remove({99, 77}));
	}


	void testGeneratedCase01() { _test("fG01"); }
	void testGeneratedCase02() { _test("fG02"); }

	void testNotRandom() {
		/*
		 * To generate a random test, execute the following commands:
		 * 		py/mtree/tests/fixtures/generator.py -a500 -r0.2 > py/mtree/tests/fixtures/fNotRandom.py
		 * 		cpp/convert-fixture-to-cpp.py fNotRandom > cpp/tests/fixtures/fNotRandom.txt
		 * /

		const string fixtureName = "fNotRandom";
		string fixtureFileName = Fixture::path(fixtureName);
		if(!ifstream(fixtureFileName)) {
			cout << "\tskipping..." << endl;
			return;
		}
		_test(fixtureName.c_str());
	}


	void testIterators() {
		struct DistanceFunction {
			size_t operator()(int a, int b) const {
				return std::abs(a - b);
			}
		};

		mt::mtree<int, DistanceFunction> mt;

		mt.add(1);
		mt.add(2);
		mt.add(3);
		mt.add(4);

		auto query = mt.get_nearest(0);

#define assertBeginEnd(ITER, BEGIN, END);   \
        assert(ITER BEGIN query.begin());   \
        assert(ITER END   query.end());

#define assertIter(ITER, BEGIN, END, DATA, DIST)   \
		assertBeginEnd(ITER, BEGIN, END);          \
        assertEqual(ITER->data, DATA);             \
        assertEqual(ITER->distance, DIST)

#define assertCompareIters(I1, C12, I2, C23, I3, C31, I1_)   \
        assert(I1 C12 I2);                                   \
        assert(I2 C23 I3);                                   \
        assert(I3 C31 I1_)

		// The first iterator
		auto i1 = query.begin();
		/*     1  2  3  4  e
		 * i1: *
		 * /
		assertIter(i1, ==, !=, 1, 1);

		// Advance the iterator
		i1++;
		/*     1  2  3  4  e
		 * i1:    *
		 * /
		assertIter(i1, !=, !=, 2, 2);

		// Advance again
		++i1;
		/*     1  2  3  4  e
		 * i1:       *
		 * /
		assertIter(i1, !=, !=, 3, 3);

		// Begin another iteration
		auto i2 = query.begin();
		/*     1  2  3  4  e
		 * i1:       *
		 * i2: *
		 * /
		assertIter(i2, ==, !=, 1, 1);
		assert(i2 != i1);
		// The first iterator must not have been affected
		assertIter(i1, !=, !=, 3, 3);

		// Copy the first iterator
		auto i3 = i1;
		/*     1  2  3  4  e
		 * i1:       *
		 * i2: *
		 * i3:       *
		 * /
		assertIter(i3, !=, !=, 3, 3);
		// The first iterator must not have been affected
		assertIter(i1, !=, !=, 3, 3);
		// The second iterator must not have been affected
		assertIter(i2, ==, !=, 1, 1);
		// Compare the iterators
		assertCompareIters(i1, !=, i2, !=, i3, ==, i1);

		// Now continue until all the iterators reach the end
		++i2;
		/*     1  2  3  4  e
		 * i1:       *
		 * i2:    *
		 * i3:       *
		 * /
		assertIter(i1, !=, !=, 3, 3);
		assertIter(i2, !=, !=, 2, 2);
		assertIter(i3, !=, !=, 3, 3);
		assertCompareIters(i1, !=, i2, !=, i3, ==, i1);

		i1++;
		/*     1  2  3  4  e
		 * i1:          *
		 * i2:    *
		 * i3:       *
		 * /
		assertIter(i1, !=, !=, 4, 4);
		assertIter(i2, !=, !=, 2, 2);
		assertIter(i3, !=, !=, 3, 3);
		assertCompareIters(i1, !=, i2, !=, i3, !=, i1);

		i2++;
		/*     1  2  3  4  e
		 * i1:          *
		 * i2:       *
		 * i3:       *
		 * /
		assertIter(i1, !=, !=, 4, 4);
		assertIter(i2, !=, !=, 3, 3);
		assertIter(i3, !=, !=, 3, 3);
		assertCompareIters(i1, !=, i2, ==, i3, !=, i1);

		++i3;
		/*     1  2  3  4  e
		 * i1:          *
		 * i2:       *
		 * i3:          *
		 * /
		assertIter(i1, !=, !=, 4, 4);
		assertIter(i2, !=, !=, 3, 3);
		assertIter(i3, !=, !=, 4, 4);
		assertCompareIters(i1, !=, i2, !=, i3, ==, i1);

		i3++;
		/*     1  2  3  4  e
		 * i1:          *
		 * i2:       *
		 * i3:             *
		 * /
		assertIter(i1, !=, !=, 4, 4);
		assertIter(i2, !=, !=, 3, 3);
		assertBeginEnd(i3, !=, ==);
		assertCompareIters(i1, !=, i2, !=, i3, !=, i1);

		++i2;
		/*     1  2  3  4  e
		 * i1:          *
		 * i2:          *
		 * i3:             *
		 * /
		assertIter(i1, !=, !=, 4, 4);
		assertIter(i2, !=, !=, 4, 4);
		assertBeginEnd(i3, !=, ==);
		assertCompareIters(i1, ==, i2, !=, i3, !=, i1);

		++i2;
		/*     1  2  3  4  e
		 * i1:          *
		 * i2:             *
		 * i3:             *
		 * /
		assertIter(i1, !=, !=, 4, 4);
		assertBeginEnd(i2, !=, ==);
		assertBeginEnd(i3, !=, ==);
		assertCompareIters(i1, !=, i2, ==, i3, !=, i1);

		++i1;
		/*     1  2  3  4  e
		 * i1:             *
		 * i2:             *
		 * i3:             *
		 * /
		assertBeginEnd(i1, !=, ==);
		assertBeginEnd(i2, !=, ==);
		assertBeginEnd(i3, !=, ==);
		assertCompareIters(i1, ==, i2, ==, i3, ==, i1);
#undef assertIter
#undef assertCompareIters
	}


private:
	typedef vector<MTreeTest::query::result_item> ResultsVector;
	*/

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
				assertTrue(!strippedResults.contains(data));
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
				assertTrue(!strippedResults.contains(data));
			} else {
				// distance == farthest
			}
		}
	}
}
