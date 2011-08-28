#include <algorithm>
#include <iostream>
#include <set>
#include <vector>
#include <cassert>
#include "mtree.h"


#define assertEqual(A, B)             assert(A == B);
#define assertLessEqual(A, B)         assert(A <= B);
#define assertIn(ELEM, CONTAINER)     assert(CONTAINER.find(ELEM) != CONTAINER.end());
#define assertNotIn(ELEM, CONTAINER)  assert(CONTAINER.find(ELEM) == CONTAINER.end());

using namespace std;


typedef vector<int> Data;


class MTreeBaseTest : public mtree::MTreeBase<Data> {
public:
	MTreeBaseTest() : MTreeBase<Data>(2, 3) { }

	void add(const Data& data) {
		MTreeBase<Data>::add(data);
		_check();
	}

	void remove(const Data& data) {
		MTreeBase<Data>::remove(data);
		_check();
	}

	double distanceFunction(const Data&, const Data&) const {
		assert(!"IMPLEMENTED");
	}

protected:
	void promotionFunction() const;

private:

};



class Test {
public:
	void testEmpty() {
		_checkNearestByRange({1, 2, 3}, 4);
		_checkNearestByLimit({1, 2, 3}, 4);
	}

private:
	MTreeBaseTest mtree;
	set<Data> allData;

	void _checkNearestByRange(const Data& queryData, double radius) const {
		typedef vector<MTreeBaseTest::ResultItem> ResultsVector;

		ResultsVector results;
		MTreeBaseTest::ResultsIterator i = mtree.getNearestByRange(queryData, radius);
		for(; i != mtree.resultsEnd(); i++) {
			MTreeBaseTest::ResultItem r = *i;
			results.push_back(r);
		}

		// TODO: TRY: vector<MTreeBaseTest::ResultItem> result(results.begin(), results.end());

		double previousDistance = 0;

		for(ResultsVector::iterator i = results.begin(); i != results.end(); i++) {
			// Check if increasing distance
			assertLessEqual(previousDistance, i->distance);
			previousDistance = i->distance;

			// Check if every item in the results came from the generated query_data
			assertIn(i->data, allData);

			// Check if every item in the results is within the range
			assertLessEqual(i->distance, radius);
			assertEqual(mtree.distanceFunction(i->data, queryData), i->distance);
		}

		set<Data> strippedResults;
		for(ResultsVector::iterator i = results.begin(); i != results.end(); ++i) {
			strippedResults.insert(i->data);
		}
		for(set<Data>::const_iterator data = allData.begin(); data != allData.end(); ++data) {
			double distance = mtree.distanceFunction(*data, queryData);
			if(distance <= radius) {
				assertIn(*data, strippedResults);
			} else {
				assertNotIn(*data, strippedResults);
			}
		}
	}


	void _checkNearestByLimit(const Data& queryData, unsigned int limit) const {
		typedef vector<MTreeBaseTest::ResultItem> ResultsVector;

		ResultsVector results;
		set<Data> strippedResults;
		MTreeBaseTest::ResultsIterator i = mtree.getNearestByLimit(queryData, limit);
		for(; i != mtree.resultsEnd(); i++) {
			MTreeBaseTest::ResultItem r = *i;
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

			// Check if every item in the results came from the generated query_data
			assertIn(i->data, allData);

			// Check if items are not repeated
			assertEqual(1, count(strippedResults.begin(), strippedResults.end(), i->data));

			double distance = mtree.distanceFunction(i->data, queryData);
			assertEqual(distance, i->distance);
			farthest = max(farthest, distance);
		}

		for(set<Data>::iterator pData = allData.begin(); pData != allData.end(); ++pData) {
			double distance = mtree.distanceFunction(*pData, queryData);
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
	Test().testEmpty();
	//test01();

	cout << "OK" << endl;
	return 0;
}


/*

class Test(unittest.TestCase):

	def setUp(self):

		# Removing randomness
		def not_random_promotion(data_objects, distance_function):
			data_objects = sorted(data_objects)
			return data_objects[0], data_objects[-1]


		self.mtree = MTreeBase(
				min_node_capacity=2,
				max_node_capacity=3,
				split_function=f.make_split_function(not_random_promotion, f.balanced_partition)
			)

		def checked(unchecked_method):
			def checked_method(*args, **kwargs):
				result = unchecked_method(*args, **kwargs)
				self.mtree._check()
				return result
			return checked_method

		self.mtree.add = checked(self.mtree.add)
		self.mtree.remove = checked(self.mtree.remove)

		self.all_data = set()



	def testEmpty(self):
		self._check_nearest_by_range((1, 2, 3), 4)
		self._check_nearest_by_limit((1, 2, 3), 4)

	def test01(self):  self._test('f01')
	def test02(self):  self._test('f02')
	def test03(self):  self._test('f03')
	def test04(self):  self._test('f04')
	def test05(self):  self._test('f05')
	def test06(self):  self._test('f06')
	def test07(self):  self._test('f07')
	def test08(self):  self._test('f08')
	def test09(self):  self._test('f09')
	def test10(self):  self._test('f10')
	def test11(self):  self._test('f11')
	def test12(self):  self._test('f12')
	def test13(self):  self._test('f13')
	def test14(self):  self._test('f14')
	def test15(self):  self._test('f15')
	def test16(self):  self._test('f16')
	def test17(self):  self._test('f17')
	def test18(self):  self._test('f18')
	def test19(self):  self._test('f19')
	def test20(self):  self._test('f20')

	def testLots(self):  self._test('fLots')


	def testRemoveNonExisting(self):
		# Empty
		self.assertRaises(KeyError, lambda: self.mtree.remove((99, 77)))

		# With some items
		self.mtree.add((4, 44))
		self.assertRaises(KeyError, lambda: self.mtree.remove((99, 77)))

		self.mtree.add((95, 43))
		self.assertRaises(KeyError, lambda: self.mtree.remove((99, 77)))

		self.mtree.add((76, 21))
		self.assertRaises(KeyError, lambda: self.mtree.remove((99, 77)))

		self.mtree.add((64, 53))
		self.assertRaises(KeyError, lambda: self.mtree.remove((99, 77)))

		self.mtree.add((47, 3))
		self.assertRaises(KeyError, lambda: self.mtree.remove((99, 77)))

		self.mtree.add((26, 11))
		self.assertRaises(KeyError, lambda: self.mtree.remove((99, 77)))


	def testGeneratedCase01(self): self._test('fG01')
	def testGeneratedCase02(self): self._test('fG02')


	def testRandom(self):
		fixtures_path, _ = os.path.split(fixtures.__file__)
		random_test_path = os.path.join(fixtures_path, 'fRandom.py')

		if os.path.isfile(random_test_path):
			print >>sys.stderr, "WARNING: Using previously generated random test (fRandom)."
			generated = False
		else:
			# Random test doesn't exist. Generate it
			options = generator.Options(actions=500, dimensions=3, remove_chance=0.2)
			fixture = generator.generate_test_data(options)
			f = file(random_test_path, 'w')
			stdout_bkp = sys.stdout
			sys.stdout = f
			try:
				print "# Test case generated by testRandom()."
				generator.print_test_data(fixture, options)
			finally:
				sys.stdout = stdout_bkp
			f.close()
			generated = True


		try:
			self._test('fRandom')
		except:
			print >>sys.stderr, "WARNING: The random test (fRandom) failed."
			print >>sys.stderr, "Investigate it, fix MTreeBase and then convert"
			print >>sys.stderr, "the random test to a permanent test case."
			raise
		else:
			if generated:
				os.remove(random_test_path)
				os.remove(random_test_path + 'c')
			else:
				print >>sys.stderr, "ATTENTION: The previously existing random test"
				print >>sys.stderr, "has passed. Do want to delete it or convert to"
				print >>sys.stderr, "a permanent test case?"



	def _test(self, fixture_name):
		fixtures = __import__('fixtures.' + fixture_name)
		fixture = getattr(fixtures, fixture_name)
		self._test_fixture(fixture)


	def _test_fixture(self, fixture):
		def callback(action):
			if isinstance(action, generator.ADD):
				assert action.data not in self.all_data
				self.all_data.add(action.data)
				self.mtree.add(action.data)
			elif isinstance(action, generator.REMOVE):
				assert action.data in self.all_data
				self.all_data.remove(action.data)
				self.mtree.remove(action.data)
			else:
				assert False, action.__class__

			self._check_nearest_by_range(action.query.data, action.query.radius)
			self._check_nearest_by_limit(action.query.data, action.query.limit)

		fixture.PERFORM(callback)


if __name__ == "__main__":
	#import sys;sys.argv = ['', 'Test.testName']
	unittest.main()

 */

