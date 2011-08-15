from collections import namedtuple
import functions
from heap_queue import HeapQueue



_CHECKED = True
if _CHECKED:
	def _checked(unchecked_method):
		def checked_method(mtree, *args, **kwargs):
			result = unchecked_method(mtree, *args, **kwargs)
			mtree._check()
			return result
		return checked_method
else:
	def _checked(unchecked_method):
		return unchecked_method



_INFINITY = float("inf")

_ItemWithDistances = namedtuple('_ItemWithDistances', 'item, distance, min_distance')



class _RootNodeReplacement(Exception):
	def __init__(self, new_root):
		super(_RootNodeReplacement, self).__init__(new_root)
		self.new_root = new_root

class _NodeUnderCapacity(Exception):
	pass


class _IndexItem(object):
	
	def __init__(self, data):
		self.data = data
		self.radius = 0                  # Updated when a child is added to this item
		self.distance_to_parent = None   # Updated when this item is added to a parent
	
	def _check(self):
		self._check_data()
		self._check_radius()
		self._check_distance_to_parent()
	
	def _check_data(self):
		assert self.data is not None
	
	def _check_radius(self):
		assert self.radius is not None
		assert self.radius >= 0
	
	def _check_distance_to_parent(self):
		assert not isinstance(self, (_RootLeafNode, _RootNode)), self
		assert self.distance_to_parent is not None
	
	def _check_distance_to_parent__root(self):
		assert isinstance(self, (_RootLeafNode, _RootNode)), self
		assert self.distance_to_parent is None



class _Node(_IndexItem):
	
	def __init__(self, data):
		super(_Node, self).__init__(data)
		self.children = []
	
	def add_data(self, data, distance, mtree):
		child = self._add_data(data)
		self.update_metrics(child, distance)
		if len(self.children) > mtree.max_node_capacity:
			raise NotImplementedError()
	
	def _add_data__leaf(self, data):
		entry = _Entry(data)
		self.children.append(entry)
		return entry
	
	def remove_data(self, data):
		self._remove_data(data)
		if len(self.children) < self._min_capacity():
			raise _NodeUnderCapacity()
	
	def _remove_data__leaf(self, data):
		index = self.get_child_index_by_data(data)
		del self.children[index]
	
	def update_metrics(self, child, distance):
		child.distance_to_parent = distance
		self.radius = max(self.radius, distance + child.radius)
	
	def get_child_index_by_data(self, data):
		for index, child in enumerate(self.children):
			if child.data == data:
				return index
	
	def _check(self, mtree):
		super(_Node, self)._check()
		for child in self.children:
			self._check_child_class(child)
			self._check_child_metrics(child, mtree)
			child._check()
	
	def _check_child_class(self, child):
		expected_class = self._get_expected_child_class()
		assert isinstance(child, expected_class)
	
	@staticmethod
	def _get_expected_child_class__leaf(self):
		return _Entry
	
	def _check_child_metrics(self, child, mtree):
		assert child.distance_to_parent == mtree.distance_function(child.data, self.data)
		assert child.distance_to_parent + child.radius <= self.radius



class _RootLeafNode(_Node):
	
	_add_data = _Node._add_data__leaf
	
	def remove_data(self, data):
		try:
			super(_RootLeafNode, self).remove_data(data)
		except _NodeUnderCapacity:
			raise _RootNodeReplacement(None)
	
	_remove_data = _Node._remove_data__leaf
	
	@staticmethod
	def _min_capacity():
		return 1
	
	_check_distance_to_parent = _Node._check_distance_to_parent__root
	
	_get_expected_child_class = _Node._get_expected_child_class__leaf



class _RootNode(_Node):
	pass


class _InternalNode(_Node):
	pass


class _LeafNode(_Node):
	pass


class _Entry(_IndexItem):
	pass



class MTreeBase(object):
	"""
	A data structure for indexing objects based on their proximity.
	
	The data objects must be any hashable object and the support functions
	(distance and split functions) must understand them.
	
	See http://en.wikipedia.org/wiki/M-tree
	"""
	
	
	ResultItem = namedtuple('ResultItem', 'data, distance')
	
	
	def __init__(self,
		         min_node_capacity=50, max_node_capacity=None,
		         distance_function=functions.euclidean_distance,
		         split_function=functions.make_split_function(functions.random_promotion, functions.balanced_partition)
		        ):
		"""
		Creates an M-Tree.
		
		The argument min_node_capacity must be at least 2.
		The argument max_node_capacity should be at least 2*min_node_capacity-1.
		The optional argument distance_function must be a function which calculates
		the distance between two data objects.
		The optional argument split_function must be a function which chooses two
		data objects and then partitions the set of data into two subsets
		according to the chosen objects. Its arguments are the set of data objects
		and the distance_function. Must return a sequence with the following four values:
			- First chosen data object.
			- Subset with at least [min_node_capacity] objects based on the first
				chosen data object. Must contain the first chosen data object.
			- Second chosen data object.
			- Subset with at least [min_node_capacity] objects based on the second
				chosen data object. Must contain the second chosen data object.
		"""
		if min_node_capacity < 2:
			raise ValueError("min_node_capacity must be at least 2")
		if max_node_capacity is None:
			max_node_capacity = 2 * min_node_capacity - 1
		if max_node_capacity <= min_node_capacity:
			raise ValueError("max_node_capacity must be greater than min_node_capacity")
		
		self.min_node_capacity = min_node_capacity
		self.max_node_capacity = max_node_capacity
		self.distance_function = distance_function
		self.split_function = split_function
		self.root = None
	
	
	@_checked
	def add(self, data):
		"""
		Adds and indexes an object.
		
		The object must not currently already be indexed!
		"""
		if self.root is None:
			self.root = _RootLeafNode(data)
			self.root.add_data(data, 0, self)
		else:
			try:
				self.root.add_data(data)
			except _SplitNodeReplacement as e:
				assert len(e.new_nodes) == 2
				self.root = _RootNode(self.root)
				for new_node in e.new_nodes:
					self.root.add_child(new_node)
	
	
	@_checked
	def remove(self, data):
		"""
		Removes an object from the index.
		"""
		try:
			self.root.remove_data(data)
		except _RootNodeReplacement as e:
			self.root = e.new_root
	
	
	def get_nearest(self, query_data, range=_INFINITY, limit=_INFINITY):
		"""
		Returns an iterator on the indexed data nearest to the query_data. The
		returned items are tuples containing the data and its distance to the
		query_data, in increasing distance order. The results can be limited by
		the range (maximum distance from the query_data) and limit arguments.
		"""
		if self.root is None:
			# No indexed data!
			return
		
		distance = self.distance_function(query_data, self.root.data)
		min_distance = max(distance - self.root.radius, 0)
		
		pending_queue = HeapQueue(
				content=[_ItemWithDistances(item=self.root, distance=distance, min_distance=min_distance)],
				key=lambda iwd: iwd.min_distance,
			)
		
		nearest_queue = HeapQueue(key=lambda iwd: iwd.distance)
		
		yielded_count = 0
		
		while pending_queue:
			pending = pending_queue.pop()
			
			node = pending.item
			assert isinstance(node, _Node)
			
			for child in node.children:
				if abs(pending.distance - child.distance_to_parent) - child.radius <= range:
					child_distance = self.distance_function(query_data, child.data)
					if child_distance - child.radius <= range:
						iwd = _ItemWithDistances(item=child, distance=child_distance, min_distance=child_distance)
						if isinstance(child, _Entry):
							nearest_queue.push(iwd)
						else:
							pending_queue.push(iwd)
			
			# Tries to yield known results so far
			if pending_queue:
				next_pending = pending_queue.head()
				next_pending_min_distance = next_pending.min_distance
			else:
				next_pending_min_distance = _INFINITY
			
			while nearest_queue:
				next_nearest = nearest_queue.head()
				assert isinstance(next_nearest, _ItemWithDistances)
				if next_nearest.distance <= next_pending_min_distance:
					_ = nearest_queue.pop()
					assert _ is next_nearest
					
					yield self.ResultItem(data=next_nearest.item.data, distance=next_nearest.distance)
					yielded_count += 1
					if yielded_count >= limit:
						# Limit reached
						return
				else:
					break

	
	
	
	
	def _check(self):
		if self.root is not None:
			self.root._check(self)
