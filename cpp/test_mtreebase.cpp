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


class MTreeBaseTest : public mtree::MTreeBase<Data> {
public:
	MTreeBaseTest() : MTreeBase<Data>(2) { }

	void add(const Data& data) {
		MTreeBase<Data>::add(data);
		_check();
	}

	void remove(const Data& data) {
		MTreeBase<Data>::remove(data);
		_check();
	}

	double distanceFunction(const Data& data1, const Data& data2) const {
		return mtree::functions::euclideanDistance(data1, data2);
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

	void test01() { _test("f01"); }
	void test02() { _test("f02"); }

private:
	typedef vector<MTreeBaseTest::ResultItem> ResultsVector;

	MTreeBaseTest mtree;
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
			case 'R':
				allData.erase(i->data);
				mtree.remove(i->data);
				break;
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
		MTreeBaseTest::ResultsIterator i = mtree.getNearestByRange(queryData, radius);
		for(; i != mtree.resultsEnd(); i++) {
			MTreeBaseTest::ResultItem r = *i;
			results.push_back(r);
			strippedResults.insert(r.data);
		}

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
	Test().test01();
	Test().test02();

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
 */

