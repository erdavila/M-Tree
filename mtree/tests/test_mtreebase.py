import unittest
import mtree.tests.fixtures.generator as generator
from mtree import MTreeBase
import mtree.functions as f



class Test(unittest.TestCase):
	
	def setUp(self):
		
		# Removing randomness
		def not_random_promotion(positions, distance_function):
			positions = sorted(positions)
			return positions[0], positions[-1]
		
		self.mtree = MTreeBase(
				min_node_capacity=2,
				max_node_capacity=3,
				split_function=f.make_split_function(not_random_promotion, f.balanced_partition)
			)
		self.all_data = set()
	
	
	'''
	'''
	def test00(self):  self._test('f00')
	def test01(self):  self._test('f01')
	def test01r(self): self._test('f01r')
	'''
	def test02(self):  self._test('f02')
	def test02r(self): self._test('f02r')
	def test03(self):  self._test('f03')
	def test03r(self): self._test('f03r')
	def test04(self):  self._test('f04')
	def test04r(self): self._test('f04r')
	def test05(self):  self._test('f05')
	def test05r(self): self._test('f05r')
	def test06(self):  self._test('f06')
	def test06r(self): self._test('f06r')
	def test07(self):  self._test('f07')
	def test07r(self): self._test('f07r')
	def test08(self):  self._test('f08')
	def test08r(self): self._test('f08r')
	def test09(self):  self._test('f09')
	def test09r(self): self._test('f09r')
	def test10(self):  self._test('f10')
	def test10r(self): self._test('f10r')
	'''
	
	
	
	def _test(self, fixture_name):
		fixtures = __import__('fixtures.' + fixture_name)
		fixture = getattr(fixtures, fixture_name)
		self._test_fixture(fixture)
	

	def _test_fixture(self, fixture):
		for action in fixture.ACTIONS:
			if isinstance(action, generator.ADD):
				assert action.data not in self.all_data
				self.all_data.add(action.data)
				self.mtree.add(action.data)
			elif isinstance(action, generator.REMOVE):
				assert action.data in self.all_data
				self.all_data.remove(action.data)
				self.mtree.remove(action.data)
			else:
				assert False
			
			self._check_nearest_by_range(action.query.data, action.query.radius)
			self._check_nearest_by_limit(action.query.data, action.query.limit)
	
	
	def _check_nearest_by_range(self, query_data, radius):
		result = list(self.mtree.get_nearest(query_data, range=radius))
		
		previous_distance = None
		for item in result:
			data, distance = item
			
			# Check if increasing distance
			if previous_distance is not None:
				self.assertTrue(distance is not None)
				self.assertLessEqual(previous_distance, distance)
			previous_distance = distance
			
			# Check if every item in the results came from the generated query_data
			self.assertIn(data, self.all_data)
			self.assertTrue(isinstance(item, MTreeBase.ResultItem), item)
			
			# Check if every item in the results is within the range
			self.assertLessEqual(distance, radius)
			self.assertEqual(self.mtree.distance_function(data, query_data), distance)
		
		stripped_result = [item.data for item in result]
		for data in self.all_data:
			dist = self.mtree.distance_function(data, query_data)
			if dist <= radius:
				self.assertIn(data, stripped_result)
			else:
				self.assertNotIn(data, stripped_result)
	
	
	def _check_nearest_by_limit(self, query_data, limit):
		nearest_result = list(self.mtree.get_nearest(query_data, limit=limit))
		
		if limit <= len(self.all_data):
			self.assertEquals(limit, len(nearest_result))
		else: # limit > len(self.all_data)
			self.assertEquals(len(self.all_data), len(nearest_result))
		
		farthest = 0.0
		previous_distance = None
		for item in nearest_result:
			data, distance = item
			
			# Check if increasing distance
			if previous_distance is not None:
				self.assertTrue(distance is not None)
				self.assertLessEqual(previous_distance, distance)
			previous_distance = distance

			# Check if every item in the results came from the generated query_data
			self.assertIn(data, self.all_data)
			self.assertTrue(isinstance(item, MTreeBase.ResultItem))
			
			# Check if items are not repeated
			self.assertEqual(1, nearest_result.count(item))
			
			d = self.mtree.distance_function(data, query_data)
			self.assertEqual(d, distance)
			farthest = max(farthest, d)
		
		stripped_nearest_result = [item.data for item in nearest_result]
		for data in self.all_data:
			d = self.mtree.distance_function(data, query_data)
			if d < farthest:
				self.assertIn(data, stripped_nearest_result)
			elif d > farthest:
				self.assertNotIn(data, stripped_nearest_result)
			else: # d == farthest:
				pass


if __name__ == "__main__":
	#import sys;sys.argv = ['', 'Test.testName']
	unittest.main()
