import functions


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



class _IndexItem(object):
	
	def __init__(self, data):
		self.data = data
		self.radius = 0                  # Updated when a child is added to this item
		self.distance_to_parent = None   # Updated when this item is added to a parent



class _Node(_IndexItem):
	
	def __init__(self, data):
		super(_Node, self).__init__(data)
		self.children = []



class _RootLeafNode(_Node):
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
			self.root.add_data(data)
		else:
			try:
				self.root.add_data(data)
			except _SplitNodeReplacement as e:
				assert len(e.new_nodes) == 2
				self.root = _RootNode(self.root)
				for new_node in e.new_nodes:
					self.root.add_child(new_node)
