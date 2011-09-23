// Doesn't make sense to compile this without assertions!
#undef NDEBUG

#include <algorithm>
#include <iostream>
#include <set>
#include <vector>
#include <cassert>
#include "mtree.h"
#include "functions.h"
#include "tests/fixture.h"


#define assertEqual(A, B)             assert(A == B);
#define assertLessEqual(A, B)         assert(A <= B);
#define assertIn(ELEM, CONTAINER)     assert(CONTAINER.find(ELEM) != CONTAINER.end());
#define assertNotIn(ELEM, CONTAINER)  assert(CONTAINER.find(ELEM) == CONTAINER.end());

using namespace std;


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


class MTreeTest : public MTree {
private:
	struct OnExit {
		MTreeTest* mt;
		OnExit(MTreeTest* mt) : mt(mt) {}
		~OnExit() { mt->_check(); }
	};

public:
	// Turning the member public
	using MTree::distance_function;

	MTreeTest()
		: MTree(2, -1,
				distance_function_type(),
				split_function_type(nonRandomPromotion)
			)
		{}

	void add(const Data& data) {
		OnExit onExit(this);
		return MTree::add(data);
	}

	bool remove(const Data& data) {
		OnExit onExit(this);
		return MTree::remove(data);
	}
};



class Test {
public:
	void testEmpty() {
		_checkNearestByRange({1, 2, 3}, 4);
		_checkNearestByLimit({1, 2, 3}, 4);
	}

	void test01() { _test("f01"); }
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
		 */

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
		 */
		assertIter(i1, ==, !=, 1, 1);

		// Advance the iterator
		i1++;
		/*     1  2  3  4  e
		 * i1:    *
		 */
		assertIter(i1, !=, !=, 2, 2);

		// Advance again
		++i1;
		/*     1  2  3  4  e
		 * i1:       *
		 */
		assertIter(i1, !=, !=, 3, 3);

		// Begin another iteration
		auto i2 = query.begin();
		/*     1  2  3  4  e
		 * i1:       *
		 * i2: *
		 */
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
		 */
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
		 */
		assertIter(i1, !=, !=, 3, 3);
		assertIter(i2, !=, !=, 2, 2);
		assertIter(i3, !=, !=, 3, 3);
		assertCompareIters(i1, !=, i2, !=, i3, ==, i1);

		i1++;
		/*     1  2  3  4  e
		 * i1:          *
		 * i2:    *
		 * i3:       *
		 */
		assertIter(i1, !=, !=, 4, 4);
		assertIter(i2, !=, !=, 2, 2);
		assertIter(i3, !=, !=, 3, 3);
		assertCompareIters(i1, !=, i2, !=, i3, !=, i1);

		i2++;
		/*     1  2  3  4  e
		 * i1:          *
		 * i2:       *
		 * i3:       *
		 */
		assertIter(i1, !=, !=, 4, 4);
		assertIter(i2, !=, !=, 3, 3);
		assertIter(i3, !=, !=, 3, 3);
		assertCompareIters(i1, !=, i2, ==, i3, !=, i1);

		++i3;
		/*     1  2  3  4  e
		 * i1:          *
		 * i2:       *
		 * i3:          *
		 */
		assertIter(i1, !=, !=, 4, 4);
		assertIter(i2, !=, !=, 3, 3);
		assertIter(i3, !=, !=, 4, 4);
		assertCompareIters(i1, !=, i2, !=, i3, ==, i1);

		i3++;
		/*     1  2  3  4  e
		 * i1:          *
		 * i2:       *
		 * i3:             *
		 */
		assertIter(i1, !=, !=, 4, 4);
		assertIter(i2, !=, !=, 3, 3);
		assertBeginEnd(i3, !=, ==);
		assertCompareIters(i1, !=, i2, !=, i3, !=, i1);

		++i2;
		/*     1  2  3  4  e
		 * i1:          *
		 * i2:          *
		 * i3:             *
		 */
		assertIter(i1, !=, !=, 4, 4);
		assertIter(i2, !=, !=, 4, 4);
		assertBeginEnd(i3, !=, ==);
		assertCompareIters(i1, ==, i2, !=, i3, !=, i1);

		++i2;
		/*     1  2  3  4  e
		 * i1:          *
		 * i2:             *
		 * i3:             *
		 */
		assertIter(i1, !=, !=, 4, 4);
		assertBeginEnd(i2, !=, ==);
		assertBeginEnd(i3, !=, ==);
		assertCompareIters(i1, !=, i2, ==, i3, !=, i1);

		++i1;
		/*     1  2  3  4  e
		 * i1:             *
		 * i2:             *
		 * i3:             *
		 */
		assertBeginEnd(i1, !=, ==);
		assertBeginEnd(i2, !=, ==);
		assertBeginEnd(i3, !=, ==);
		assertCompareIters(i1, ==, i2, ==, i3, ==, i1);
#undef assertIter
#undef assertCompareIters
	}


private:
	typedef vector<MTreeTest::query::result_item> ResultsVector;

	MTreeTest mtree;
	set<Data> allData;


	void _test(const char* fixtureName) {
		Fixture fixture = Fixture::load(fixtureName);
		_testFixture(fixture);
	}


	void _testFixture(const Fixture& fixture) {
		for(vector<Fixture::Action>::const_iterator i = fixture.actions.begin(); i != fixture.actions.end(); ++i) {
			switch(i->cmd) {
			case 'A':
				allData.insert(i->data);
				mtree.add(i->data);
				break;
			case 'R': {
				allData.erase(i->data);
				bool removed = mtree.remove(i->data);
				assert(removed);
				break;
			}
			default:
				cerr << i->cmd << endl;
				assert(false);
				break;
			}

			_checkNearestByRange(i->queryData, i->radius);
			_checkNearestByLimit(i->queryData, i->limit);
		}
	}


	void _checkNearestByRange(const Data& queryData, double radius) const {
		ResultsVector results;
		set<Data> strippedResults;
		MTreeTest::query query = mtree.get_nearest_by_range(queryData, radius);

		for(MTreeTest::query::iterator i = query.begin(); i != query.end(); ++i) {
			MTreeTest::query::value_type r = *i;
			results.push_back(r);
			strippedResults.insert(r.data);
		}

		double previousDistance = 0;

		for(ResultsVector::iterator i = results.begin(); i != results.end(); i++) {
			// Check if increasing distance
			assertLessEqual(previousDistance, i->distance);
			previousDistance = i->distance;

			// Check if every item in the results came from the generated queryData
			assertIn(i->data, allData);

			// Check if every item in the results is within the range
			assertLessEqual(i->distance, radius);
			assertEqual(mtree.distance_function(i->data, queryData), i->distance);
		}

		for(set<Data>::const_iterator data = allData.begin(); data != allData.end(); ++data) {
			double distance = mtree.distance_function(*data, queryData);
			if(distance <= radius) {
				assertIn(*data, strippedResults);
			} else {
				assertNotIn(*data, strippedResults);
			}
		}
	}


	void _checkNearestByLimit(const Data& queryData, unsigned int limit) const {
		ResultsVector results;
		set<Data> strippedResults;
		MTreeTest::query query = mtree.get_nearest_by_limit(queryData, limit);
		for(MTreeTest::query::iterator i = query.begin(); i != query.end(); i++) {
			MTreeTest::query::result_item r = *i;
			results.push_back(r);
			strippedResults.insert(r.data);
		}

		if(limit <= allData.size()) {
			assertEqual(limit, results.size());
		} else {
			// limit > allData.size()
			assertEqual(allData.size(), results.size());
		}

		double farthest = 0.0;
		double previousDistance = 0.0;
		for(ResultsVector::iterator i = results.begin(); i != results.end(); ++i) {
			// Check if increasing distance
			assertLessEqual(previousDistance, i->distance);
			previousDistance = i->distance;

			// Check if every item in the results came from the generated queryData
			assertIn(i->data, allData);

			// Check if items are not repeated
			assertEqual(1, count(strippedResults.begin(), strippedResults.end(), i->data));

			double distance = mtree.distance_function(i->data, queryData);
			assertEqual(distance, i->distance);
			farthest = max(farthest, distance);
		}

		for(set<Data>::iterator pData = allData.begin(); pData != allData.end(); ++pData) {
			double distance = mtree.distance_function(*pData, queryData);
			if(distance < farthest) {
				assertIn(*pData, strippedResults);
			} else if(distance > farthest) {
				assertNotIn(*pData, strippedResults);
			} else {
				// distance == farthest
			}
		}
	}
};





int main() {
#define RUN_TEST(T)   cout << "Running " #T "..." << endl; Test().T()
	RUN_TEST(testEmpty);
	RUN_TEST(test01);
	RUN_TEST(test02);
	RUN_TEST(test03);
	RUN_TEST(test04);
	RUN_TEST(test05);
	RUN_TEST(test06);
	RUN_TEST(test07);
	RUN_TEST(test08);
	RUN_TEST(test09);
	RUN_TEST(test10);
	RUN_TEST(test11);
	RUN_TEST(test12);
	RUN_TEST(test13);
	RUN_TEST(test14);
	RUN_TEST(test15);
	RUN_TEST(test16);
	RUN_TEST(test17);
	RUN_TEST(test18);
	RUN_TEST(test19);
	RUN_TEST(test20);
	RUN_TEST(testLots);
	RUN_TEST(testRemoveNonExisting);
	RUN_TEST(testGeneratedCase01);
	RUN_TEST(testGeneratedCase02);
	RUN_TEST(testNotRandom);
	RUN_TEST(testIterators);
#undef RUN_TEST

	cout << "DONE" << endl;
	return 0;
}
