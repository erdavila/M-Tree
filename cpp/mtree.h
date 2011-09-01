#ifndef MTREE_H_
#define MTREE_H_


#include <limits>
#include <map>
#include <queue>


namespace mtree {



template <typename T>
class MTreeBase {
private:
	class Node;
	class Entry;

public:

	class ResultItem {
	public:
		T data;
		double distance;

		ResultItem() = default;
		ResultItem(const ResultItem&) = default;
		~ResultItem() = default;
		ResultItem& operator=(const ResultItem&) = default;
	};

	class ResultsIterator {
	public:
		ResultsIterator() : isEnd(true) { }

		ResultsIterator(const ResultsIterator&) = default;

		ResultsIterator(ResultsIterator&&); // TODO: implement

		ResultsIterator(const MTreeBase* mtree, const T& queryData, double range, size_t limit)
			: mtree(mtree),
			  queryData(queryData),
			  range(range),
			  limit(limit),
			  isEnd(false),
			  yieldedCount(0)
		{
			if(mtree->root == NULL) {
				isEnd = true;
				return;
			}

			double distance = mtree->distanceFunction(queryData, mtree->root->data);
			double minDistance = std::max(distance - mtree->root->radius, 0.0);

			pendingQueue.push(ItemWithDistances<Node>(mtree->root, distance, minDistance));
			nextPendingMinDistance = minDistance;

			fetchNext();
		}

		// TODO: default?
		~ResultsIterator() {
			// TODO: review
		}

		ResultsIterator& operator=(const ResultsIterator&) = delete; // TODO: confirm if really delete
		ResultsIterator& operator=(ResultsIterator&&) = delete; // TODO: confirm if really delete

		bool operator==(const ResultsIterator& ri) const {
			if(this->isEnd  &&  ri.isEnd) {
				return true;
			}

			if(this->isEnd  ||  ri.isEnd) {
				return false;
			}

			assert(!"IMPLEMENTED");
		}

		bool operator!=(const ResultsIterator& ri) const {
			return ! operator==(ri);
		}

		// prefix
		ResultsIterator& operator++() {
			fetchNext();
			return *this;
		}

		// postfix
		ResultsIterator operator++(int) {
			ResultsIterator aCopy = *this;
			operator++();
			return aCopy;
		}

		ResultItem operator*() const {
			return currentResultItem;
		}

	private:
		template <typename U>
		struct ItemWithDistances {
			const U* item;
			double distance;
			double minDistance;

			ItemWithDistances(const U* item, double distance, double minDistance)
				: item(item), distance(distance), minDistance(minDistance)
				{ }

			bool operator<(const ItemWithDistances& that) const {
				return this->minDistance > that.minDistance;
			}

		};

		void fetchNext() {
			assert(! isEnd);

			if(yieldedCount >= limit) {
				isEnd = true;
				return;
			}

			 while(!pendingQueue.empty()  ||  !nearestQueue.empty()) {
				if(prepareNextNearest()) {
					return;
				}

				assert(!pendingQueue.empty());

				ItemWithDistances<Node> pending = pendingQueue.top();
				pendingQueue.pop();

				const Node* node = pending.item;

				for(typename Node::ChildrenMap::const_iterator i = node->children.begin(); i != node->children.end(); ++i) {
					IndexItem* child = i->second;
					if(std::abs(pending.distance - child->distanceToParent) - child->radius <= range) {
						double childDistance = mtree->distanceFunction(queryData, child->data);
						double childMinDistance = std::max(childDistance - child->radius, 0.0);
						if(childMinDistance <= range) {
							Entry* entry = dynamic_cast<Entry*>(child);
							if(entry != NULL) {
								nearestQueue.push(ItemWithDistances<Entry>(entry, childDistance, childMinDistance));
							} else {
								Node* node = dynamic_cast<Node*>(child);
								assert(node != NULL);
								pendingQueue.push(ItemWithDistances<Node>(node, childDistance, childMinDistance));
							}
						}
					}
				}

				if(pendingQueue.empty()) {
					nextPendingMinDistance = std::numeric_limits<double>::infinity();
				} else {
					nextPendingMinDistance = pendingQueue.top().minDistance;
				}

			}

			isEnd = true;


			/* TODO: remove this comment

			while pending_queue:
				pending = pending_queue.pop()

				node = pending.item
				assert isinstance(node, _Node)

				for child in node.children.itervalues():
					if abs(pending.distance - child.distance_to_parent) - child.radius <= range:
						child_distance = self.distance_function(query_data, child.data)
						child_min_distance = max(child_distance - child.radius, 0)
						if child_min_distance <= range:
							iwd = _ItemWithDistances(item=child, distance=child_distance, min_distance=child_min_distance)
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
			*/
		}


		bool prepareNextNearest() {
			if(!nearestQueue.empty()) {
				const ItemWithDistances<Entry>& nextNearest = nearestQueue.top();
				if(nextNearest.distance <= nextPendingMinDistance) {
					nearestQueue.pop();
					currentResultItem.data = nextNearest.item->data;
					currentResultItem.distance = nextNearest.distance;
					return true;
				}
			}

			return false;
		}


		const MTreeBase* mtree;
		T queryData;
		double range;
		size_t limit;
		bool isEnd;
		std::priority_queue<ItemWithDistances<Node> > pendingQueue;
		double nextPendingMinDistance;
		std::priority_queue<ItemWithDistances<Entry> > nearestQueue;
		size_t yieldedCount;
		ResultItem currentResultItem;
	};


	MTreeBase(size_t minNodeCapacity=50, size_t maxNodeCapacity=-1)
		: minNodeCapacity(minNodeCapacity),
		  maxNodeCapacity(maxNodeCapacity),
		  root(NULL)
	{
		if(maxNodeCapacity == size_t(-1)) {
			this->maxNodeCapacity = 2 * minNodeCapacity - 1;
		}
	}

	MTreeBase() = delete; // TODO: confirm if really delete
	MTreeBase(const MTreeBase&) = delete; // TODO: confirm if really delete
	MTreeBase(MTreeBase&&) = delete; // TODO: confirm if really delete

	virtual ~MTreeBase() {
		delete root;
	}

	MTreeBase& operator=(const MTreeBase&) = delete; // TODO: confirm if really delete
	MTreeBase& operator=(MTreeBase&&) = delete; // TODO: confirm if really delete

	void add(const T& data) {
		if(root == NULL) {
			root = new RootLeafNode(data);
			root->addData(data, 0, this);
		} else {
			assert(!"IMPLEMENTED");
			/*
			distance = self.distance_function(data, self.root.data)
			try:
				self.root.add_data(data, distance, self)
			except _SplitNodeReplacement as e:
				assert len(e.new_nodes) == 2
				self.root = _RootNode(self.root.data)
				for new_node in e.new_nodes:
					distance = self.distance_function(self.root.data, new_node.data)
					self.root.add_child(new_node, distance, self)

			 */
		}
	}

	void remove(const T& data);

	ResultsIterator getNearestByRange(const T& queryData, double range) const {
		return getNearest(queryData, range, std::numeric_limits<unsigned int>::max());
	}

	ResultsIterator getNearestByLimit(const T& queryData, size_t limit) const {
		return getNearest(queryData, std::numeric_limits<double>::infinity(), limit);
	}

	ResultsIterator getNearest(const T& queryData, double range, size_t limit) const {
		return ResultsIterator(this, queryData, range, limit);
	}

	ResultsIterator resultsEnd() const {
		return ResultsIterator();
	}

protected:
	virtual double distanceFunction(const T&, const T&) const = 0;

	virtual double splitFunction() {
		assert(!"IMPLEMENTED");
	}

	virtual double promotionFunction() {
		assert(!"IMPLEMENTED");
	}

	virtual double partitionFunction() {
		assert(!"IMPLEMENTED");
	}

	void _check() const {
		if(root != NULL) {
			root->_check(this);
		}
	}

private:

	size_t minNodeCapacity;
	size_t maxNodeCapacity;
	Node* root;


public:
	class IndexItem {
	public:
		T data;
		double radius;
		double distanceToParent;

		virtual ~IndexItem() { };

	protected:
		IndexItem(const T& data)
			: data(data),
			  radius(0),
			  distanceToParent(-1)
			{ }

	public:
		virtual size_t _check(const MTreeBase* mtree) const {
			_checkRadius();
			_checkDistanceToParent();
			return 1;
		}

	private:
		void _checkRadius() const {
			assert(radius >= 0);
		}

	protected:
		virtual void _checkDistanceToParent() const {
			assert(dynamic_cast<const RootNodeTrait*>(this) == NULL);
			assert(distanceToParent >= 0);
		}

	private:
		IndexItem() = delete;
		IndexItem(const IndexItem&) = delete; // TODO: confirm
		IndexItem(IndexItem&&) = delete; // TODO: confirm
		IndexItem& operator=(const IndexItem&) = delete; // TODO: confirm
		IndexItem& operator=(IndexItem&&) = delete; // TODO: confirm
	};


private:
	class Node : public IndexItem {
	public:
		virtual ~Node() {
			assert(!"IMPLEMENTED");
		}

		void addData(const T& data, double distance, MTreeBase* mtree) {
			doAddData(data, distance, mtree);
			checkMaxCapacity(mtree);
		}

		size_t _check(const MTreeBase* mtree) const {
			IndexItem::_check(mtree);
			_checkMinCapacity(mtree);
			_checkMaxCapacity(mtree);

			bool   childHeightKnown = false;
			size_t childHeight;
			for(typename ChildrenMap::const_iterator i = children.begin(); i != children.end(); ++i) {
				const T& data = i->first;
				IndexItem* child = i->second;

				assert(child->data == data);
				_checkChildClass(child);
				_checkChildMetrics(child, mtree);

				size_t height = child->_check(mtree);
				if(childHeightKnown) {
					assert(childHeight == height);
				} else {
					childHeight = height;
					childHeightKnown = true;
				}
			}

			return childHeight + 1;
		}

		typedef std::map<T, IndexItem*> ChildrenMap;

		ChildrenMap children;

	protected:
		Node(const T& data) : IndexItem(data) { }

		Node() : IndexItem(*((T*)(0))) { assert(!"THIS SHOULD NEVER BE CALLED"); };

		Node(const Node&) = delete; // TODO: confirm if really delete
		Node(Node&&) = delete; // TODO: confirm if really delete

		Node& operator=(const Node&) = delete; // TODO: confirm if really delete
		Node& operator=(Node&&) = delete; // TODO: confirm if really delete

		virtual void doAddData(const T& data, double distance, MTreeBase* mtree) = 0;

		void checkMaxCapacity(MTreeBase* mtree) {
			if(children.size() > mtree->maxNodeCapacity) {
				assert(!"IMPLEMENTED");
				/*
				data_objects = frozenset(self.children.iterkeys())
				cached_distance_function = functions.make_cached_distance_function(mtree.distance_function)

				(promoted_data1, partition1,
				 promoted_data2, partition2) = mtree.split_function(data_objects, cached_distance_function)

				split_node_replacement_class = self.get_split_node_replacement_class()
				new_nodes = []
				for promoted_data, partition in [(promoted_data1, partition1),
												 (promoted_data2, partition2)]:
					new_node = split_node_replacement_class(promoted_data)
					for data in partition:
						child = self.children[data]
						distance = cached_distance_function(promoted_data, data)
						new_node.add_child(child, distance, mtree)
					new_nodes.append(new_node)

				raise _SplitNodeReplacement(new_nodes)
				*/
			}

		}

		/*
		def remove_data(self, data, distance, mtree):
			self.do_remove_data(data, distance, mtree)
			if len(self.children) < self.get_min_capacity(mtree):
				raise _NodeUnderCapacity()
		*/

		void updateMetrics(IndexItem* child, double distance) {
			child->distanceToParent = distance;
			updateRadius(child);
		}

		void updateRadius(IndexItem* child) {
			this->radius = std::max(this->radius, child->distanceToParent + child->radius);
		}


		virtual void _checkMinCapacity(const MTreeBase* mtree) const = 0;

	private:
		void _checkMaxCapacity(const MTreeBase* mtree) const {
			assert(children.size() <= mtree->maxNodeCapacity);
		}

	protected:
		virtual void _checkChildClass(IndexItem* child) const = 0;

	private:
		void _checkChildMetrics(IndexItem* child, const MTreeBase* mtree) const {
			double dist = mtree->distanceFunction(child->data, this->data);
			assert(child->distanceToParent == dist);
			assert(child->distanceToParent + child->radius <= this->radius);
		}
	};


	class RootNodeTrait : public virtual Node {
		void _checkDistanceToParent() const {
			assert(this->distanceToParent == -1);
		}
	};


	class LeafNodeTrait : public virtual Node {
		void doAddData(const T& data, double distance, MTreeBase* mtree) {
			Entry* entry = new Entry(data);
			assert(this->children.find(data) == this->children.end());
			this->children[data] = entry;
			assert(this->children.find(data) != this->children.end());
			updateMetrics(entry, distance);
		}

		/*
		def add_child(self, child, distance, mtree):
			assert child.data not in self.children
			self.children[child.data] = child
			assert child.data in self.children
			self.update_metrics(child, distance)

		@staticmethod
		def get_split_node_replacement_class():
			return _LeafNode

		def do_remove_data(self, data, distance, mtree):
			del self.children[data]

		*/

		void _checkChildClass(IndexItem* child) const {
			assert(dynamic_cast<Entry*>(child) != NULL);
		}
	};


	class RootLeafNode : public RootNodeTrait, public LeafNodeTrait {
	public:
		RootLeafNode(const T& data) : Node(data) { }
		/*

		def remove_data(self, data, distance, mtree):
			try:
				super(_RootLeafNode, self).remove_data(data, distance, mtree)
			except _NodeUnderCapacity:
				assert len(self.children) == 0
				raise _RootNodeReplacement(None)

		@staticmethod
		def get_min_capacity(mtree):
			return 1
		*/

		void _checkMinCapacity(const MTreeBase* mtree) const {
			assert(this->children.size() >= 1);
		}
	};


	class Entry : public IndexItem {
	public:
		Entry(const T& data) : IndexItem(data) { }
	};
};



} /* namespace mtree */


#endif /* MTREE_H_ */


/*
from collections import namedtuple
import functions
from heap_queue import HeapQueue



_INFINITY = float("inf")

_ItemWithDistances = namedtuple('_ItemWithDistances', 'item, distance, min_distance')



class _RootNodeReplacement(Exception):
	def __init__(self, new_root):
		super(_RootNodeReplacement, self).__init__(new_root)
		self.new_root = new_root

class _SplitNodeReplacement(Exception):
	def __init__(self, new_nodes):
		super(_SplitNodeReplacement, self).__init__(new_nodes)
		self.new_nodes = new_nodes

class _NodeUnderCapacity(Exception):
	pass




class _RootNodeTrait(_Node):


class _NonRootNodeTrait(_Node):

	def get_min_capacity(self, mtree):
		return mtree.min_node_capacity

	def _check_min_capacity(self, mtree):
		assert len(self.children) >= mtree.min_node_capacity



class _LeafNodeTrait(_Node):



class _NonLeafNodeTrait(_Node):

	CandidateChild = namedtuple('CandidateChild', 'node, distance, metric')


	def do_add_data(self, data, distance, mtree):

		min_radius_increase_needed = self.CandidateChild(None, None, _INFINITY)
		nearest_distance = self.CandidateChild(None, None, _INFINITY)

		for child in self.children.itervalues():
			distance = mtree.distance_function(child.data, data)
			if distance > child.radius:
				radius_increase = distance - child.radius
				if radius_increase < min_radius_increase_needed.metric:
					min_radius_increase_needed = self.CandidateChild(child, distance, radius_increase)
			else:
				if distance < nearest_distance.metric:
					nearest_distance = self.CandidateChild(child, distance, distance)

		if nearest_distance.node is not None:
			chosen = nearest_distance
		else:
			chosen = min_radius_increase_needed

		child = chosen.node
		try:
			child.add_data(data, chosen.distance, mtree)
		except _SplitNodeReplacement as e:
			assert len(e.new_nodes) == 2
			# Replace current child with new nodes
			del self.children[child.data]
			for new_child in e.new_nodes:
				distance = mtree.distance_function(self.data, new_child.data)
				self.add_child(new_child, distance, mtree)
		else:
			self.update_radius(child)


	def add_child(self, new_child, distance, mtree):
		new_children = [(new_child, distance)]
		while new_children:
			new_child, distance = new_children.pop()

			if new_child.data not in self.children:
				self.children[new_child.data] = new_child
				self.update_metrics(new_child, distance)
			else:
				existing_child = self.children[new_child.data]
				assert existing_child.data == new_child.data

				# Transfer the _children_ of the new_child to the existing_child
				for grandchild in new_child.children.itervalues():
					existing_child.add_child(grandchild, grandchild.distance_to_parent, mtree)

				try:
					existing_child.check_max_capacity(mtree)
				except _SplitNodeReplacement as e:
					del self.children[new_child.data]
					for new_node in e.new_nodes:
						distance = mtree.distance_function(self.data, new_node.data)
						new_children.append((new_node, distance))


	@staticmethod
	def get_split_node_replacement_class():
		return _InternalNode


	def do_remove_data(self, data, distance, mtree):
		for child in self.children.itervalues():
			if abs(distance - child.distance_to_parent) <= child.radius:   # TODO: confirm
				distance_to_child = mtree.distance_function(data, child.data)
				if distance_to_child <= child.radius:
					try:
						child.remove_data(data, distance_to_child, mtree)
					except KeyError:
						# If KeyError was raised, then the data was not found in the child
						pass
					except _NodeUnderCapacity:
						expanded_child = self.balance_children(child, mtree)
						self.update_radius(expanded_child)
						return
					else:
						self.update_radius(child)
						return
		raise KeyError()


	def balance_children(self, the_child, mtree):
		# Tries to find another_child which can donate a grandchild to the_child.

		nearest_donor = None
		distance_nearest_donor = _INFINITY

		nearest_merge_candidate = None
		distance_nearest_merge_candidate = _INFINITY

		for another_child in (child for child in self.children.itervalues() if child is not the_child):
			distance = mtree.distance_function(the_child.data, another_child.data)
			if len(another_child.children) > another_child.get_min_capacity(mtree):
				if distance < distance_nearest_donor:
					distance_nearest_donor = distance
					nearest_donor = another_child
			else:
				if distance < distance_nearest_merge_candidate:
					distance_nearest_merge_candidate = distance
					nearest_merge_candidate = another_child

		if nearest_donor is None:
			# Merge
			for grandchild in the_child.children.itervalues():
				distance = mtree.distance_function(grandchild.data, nearest_merge_candidate.data)
				nearest_merge_candidate.add_child(grandchild, distance, mtree)

			del self.children[the_child.data]
			return nearest_merge_candidate
		else:
			# Donate
			# Look for the nearest grandchild
			nearest_grandchild_distance = _INFINITY
			for grandchild in nearest_donor.children.itervalues():
				distance = mtree.distance_function(grandchild.data, the_child.data)
				if distance < nearest_grandchild_distance:
					nearest_grandchild_distance = distance
					nearest_grandchild = grandchild

			del nearest_donor.children[nearest_grandchild.data]
			the_child.add_child(nearest_grandchild, nearest_grandchild_distance, mtree)
			return the_child


	@staticmethod
	def _get_expected_child_class():
		"""Called by _check_child_class"""
		return (_InternalNode, _LeafNode)



class _RootNode(_RootNodeTrait, _NonLeafNodeTrait):

	def remove_data(self, data, distance, mtree):
		try:
			super(_RootNode, self).remove_data(data, distance, mtree)
		except _NodeUnderCapacity:
			# Promote the only child to root
			(the_child,) = self.children.itervalues()
			if isinstance(the_child, _InternalNode):
				new_root_class = _RootNode
			else:
				assert isinstance(the_child, _LeafNode)
				new_root_class = _RootLeafNode

			new_root = new_root_class(the_child.data)
			for grandchild in the_child.children.itervalues():
				distance = mtree.distance_function(new_root.data, grandchild.data)
				new_root.add_child(grandchild, distance, mtree)

			raise _RootNodeReplacement(new_root)


	@staticmethod
	def get_min_capacity(mtree):
		return 2

	def _check_min_capacity(self, mtree):
		assert len(self.children) >= 2



class _InternalNode(_NonRootNodeTrait, _NonLeafNodeTrait):
	pass



class _LeafNode(_NonRootNodeTrait, _LeafNodeTrait):
	pass



class _Entry(_IndexItem)



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


	def remove(self, data):
		"""
		Removes an object from the index.
		"""
		if self.root is None:
			raise KeyError()

		distance_to_root = self.distance_function(data, self.root.data)
		try:
			self.root.remove_data(data, distance_to_root, self)
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

			for child in node.children.itervalues():
				if abs(pending.distance - child.distance_to_parent) - child.radius <= range:
					child_distance = self.distance_function(query_data, child.data)
					child_min_distance = max(child_distance - child.radius, 0)
					if child_min_distance <= range:
						iwd = _ItemWithDistances(item=child, distance=child_distance, min_distance=child_min_distance)
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
 */
