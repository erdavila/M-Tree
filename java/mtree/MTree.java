package mtree;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.NoSuchElementException;
import java.util.PriorityQueue;


/* *
 * @brief The main class that implements the M-Tree.
 *
 * @tparam Data The type of data that will be indexed by the M-Tree. This type
 *         must be an assignable type and a strict weak ordering must be defined
 *         by @c std::less<Data>.
 * @tparam DistanceFunction The type of the function that will be used to
 *         calculate the distance between two @c Data objects. By default, it is
 *         ::mt::functions::euclidean_distance.
 * @tparam SplitFunction The type of the function that will be used to split a
 *         node when it is at its maximum capacity and a new child must be
 *         added. By default, it is a composition of
 *         ::mt::functions::random_promotion and
 *         ::mt::functions::balanced_partition.
 *
 *
 * @todo Include a @c Compare template and constructor parameters instead of
 *       implicitly using @c std::less<Data> on @c stc::set and @c std::map.
 *
 * @todo Implement an @c unordered_mtree class which uses @c std::unordered_set
 *      and @c std::unordered_map instead of @c std::set and @c std::map
 *      respectively.
 */
public class MTree<DATA> {

	/* *
	 * The type of the results for nearest-neighbor queries.
	 */
	public class ResultItem {
		public ResultItem(DATA data, double distance) {
			this.data = data;
			this.distance = distance;
		}

		/* * A nearest-neighbor. */
		public DATA data;

		/* * @brief The distance from the nearest-neighbor to the query data
		 *         object parameter.
		 */
		public double distance;
	}
	
	
	// Exception classes
	private static class SplitNodeReplacement extends Exception {
		/*
	public:
		enum { NUM_NODES = 2 };
		Node* newNodes[NUM_NODES];
		SplitNodeReplacement(Node* newNodes[NUM_NODES]) {
			for(int i = 0; i < NUM_NODES; ++i) {
				this->newNodes[i] = newNodes[i];
			}

		}
		*/
	}
	
	private static class RootNodeReplacement extends Exception {
		// A subclass of Throwable cannot be generic.  :-(
		// So, we have newRoot declared as Object instead of Node.
		private Object newRoot;
		
		private RootNodeReplacement(Object newRoot) {
			this.newRoot = newRoot;
		}
	}

	
	private static class NodeUnderCapacity extends Exception { }
	

	private static class DataNotFound extends Exception {
		// A subclass of Throwable cannot be generic.  :-(
		// So, we have data declared as Object instead of Data.
		private Object data;
		
		private DataNotFound(Object data) {
			this.data = data;
		}
	}

	/* *
	 * An Iterable class which can be iterated to fetch the results of a
	 * nearest-neighbors query.
	 * 
	 * The neighbors are presented in non-decreasing order from the @c queryData
	 * argument to the mtree::get_nearest() call.
	 *
	 *          The query on the M-Tree is executed during the iteration, as the
	 *          results are fetched. It means that, by the time when the @a n-th
	 *          result is fetched, the next result may still not be known, and
	 *          the resources allocated were only the necessary to identify the
	 *          @a n first results.
	 *
	 *          The objects in the container are mtree::query_result instances,
	 *          which contain a data object and the distance from the query
	 *          data object.
	 * @see mtree::get_nearest()
	 */
	public class Query implements Iterable<ResultItem> {

		private class ResultsIterator implements Iterator<ResultItem> {
			
			private class ItemWithDistances <U> implements Comparable<ItemWithDistances<U>> {
				private U item;
				private double distance;
				private double minDistance;

				public ItemWithDistances(U item, double distance, double minDistance) {
					this.item = item;
					this.distance = distance;
					this.minDistance = minDistance;
				}

				@Override
				public int compareTo(ItemWithDistances<U> that) {
					if(this.minDistance < that.minDistance) {
						return -1;
					} else if(this.minDistance > that.minDistance) {
						return +1;
					} else {
						return 0;
					}
				}
			}
			
			
			private ResultItem nextResultItem = null;
			private boolean finished = false;
			private PriorityQueue<ItemWithDistances<Node>> pendingQueue = new PriorityQueue<ItemWithDistances<Node>>();
			private double nextPendingMinDistance;
			private PriorityQueue<ItemWithDistances<Entry>> nearestQueue = new PriorityQueue<ItemWithDistances<Entry>>();
			private int yieldedCount;
			
			private ResultsIterator() {
				if(MTree.this.root == null) {
					finished = true;
					return;
				}
				
				double distance = MTree.this.distanceFunction.calculate(Query.this.data, MTree.this.root.data);
				double minDistance = Math.max(distance - MTree.this.root.radius, 0.0);
				
				pendingQueue.add(new ItemWithDistances<Node>(MTree.this.root, distance, minDistance));
				nextPendingMinDistance = minDistance;
			}
			
			
			@Override
			public boolean hasNext() {
				if(finished) {
					return false;
				}
				
				if(nextResultItem == null) {
					fetchNext();
				}
				
				if(nextResultItem == null) {
					finished = true;
					return false;
				} else {
					return true;
				}
			}
			
			@Override
			public ResultItem next() {
				if(hasNext()) {
					ResultItem next = nextResultItem;
					nextResultItem = null;
					return next;
				} else {
					throw new NoSuchElementException();
				}
			}
			
			@Override
			public void remove() {
				throw new UnsupportedOperationException();
			}
			
			
			private void fetchNext() {
				assert !finished;
				
				if(finished  ||  yieldedCount >= Query.this.limit) {
					finished = true;
					return;
				}
				
				while(!pendingQueue.isEmpty()  ||  !nearestQueue.isEmpty()) {
					if(prepareNextNearest()) {
						return;
					}
					
					assert !pendingQueue.isEmpty();
					
					ItemWithDistances<Node> pending = pendingQueue.poll();
					Node node = pending.item;
					
					for(IndexItem child : node.children.values()) {
						if(Math.abs(pending.distance - child.distanceToParent) - child.radius <= Query.this.range) {
							double childDistance = MTree.this.distanceFunction.calculate(Query.this.data, child.data);
							double childMinDistance = Math.max(childDistance - child.radius, 0.0);
							if(childMinDistance <= Query.this.range) {
								if(child instanceof MTree.Entry) {
									@SuppressWarnings("unchecked")
									Entry entry = (Entry)child;
									nearestQueue.add(new ItemWithDistances<Entry>(entry, childDistance, childMinDistance));
								} else {
									throw new RuntimeException("Not implemented");
									/*
									Node* node = dynamic_cast<Node*>(child);
									assert(node != NULL);
									pendingQueue.push({node, childDistance, childMinDistance});
									*/
								}
							}
						}
					}
					
					if(pendingQueue.isEmpty()) {
						nextPendingMinDistance = Double.POSITIVE_INFINITY;
					} else {
						throw new RuntimeException("Not implemented");
						/*
						nextPendingMinDistance = pendingQueue.top().minDistance;
						*/
					}
				}

				finished = true;
			}

			
			private boolean prepareNextNearest() {
				if(!nearestQueue.isEmpty()) {
					ItemWithDistances<Entry> nextNearest = nearestQueue.peek();
					if(nextNearest.distance <= nextPendingMinDistance) {
						nearestQueue.poll();
						nextResultItem = new ResultItem(nextNearest.item.data, nextNearest.distance);
						++yieldedCount;
						return true;
					}
				}
				
				return false;
			}
			
		}
		
		
		private Query(DATA data, double range, int limit) {
			this.data = data;
			this.range = range;
			this.limit = limit;
		}
		
		
		@Override
		public ResultsIterator iterator() {
			return new ResultsIterator();
		}


		/* *
		 * @brief The iterator for accessing the results of nearest-neighbor
		 *        queries.
		 * /
		class iterator {
		public:
			bool operator==(const iterator& ri) const {
				if(this->isEnd  &&  ri.isEnd) {
					return true;
				}

				if(this->isEnd  ||  ri.isEnd) {
					return false;
				}

				return  this->_query == ri._query
				    &&  this->yieldedCount == ri.yieldedCount;
			}
		};
		*/

		private DATA data;
		private double range;
		private int limit;
	}


	
	/* *
	 * @brief The default minimum capacity of nodes in an M-Tree, when not
	 * specified in the constructor call.
	 */
	public static final int DEFAULT_MIN_NODE_CAPACITY = 50;


	protected int minNodeCapacity;
	protected int maxNodeCapacity;
	protected DistanceFunction<? super DATA> distanceFunction;
	protected Node root;
	
	
	/* *
	 * @brief The main constructor of an M-Tree.
	 *
	 * @param min_node_capacity The minimum capacity of the nodes of an M-Tree.
	 *        Should be at least 2.
	 * @param max_node_capacity The maximum capacity of the nodes of an M-Tree.
	 *        Should be greater than @c min_node_capacity. If -1 is passed, then
	 *        the value <code>2*min_node_capacity - 1</code> is used.
	 * @param distance_function An instance of @c DistanceFunction.
	 * @param split_function An instance of @c SplitFunction.
	 *
	 */
	public MTree(DistanceFunction<? super DATA> distanceFunction) {
		this(DEFAULT_MIN_NODE_CAPACITY, distanceFunction);
	}
	
	public MTree(int minNodeCapacity, DistanceFunction<? super DATA> distanceFunction) {
		this(minNodeCapacity, 2 * minNodeCapacity - 1, distanceFunction);
	}
	
	public MTree(int minNodeCapacity, int maxNodeCapacity, DistanceFunction<? super DATA> distanceFunction) {
		if(minNodeCapacity < 2  ||  maxNodeCapacity <= minNodeCapacity  ||  distanceFunction == null) {
			throw new IllegalArgumentException();
		}
		
		this.minNodeCapacity = minNodeCapacity;
		this.maxNodeCapacity = maxNodeCapacity;
		this.distanceFunction = distanceFunction;
		this.root = null;
	}
	
	
	/* *
	 * @brief Adds and indexes a data object.
	 * @details An object that is already indexed should not be added. There is
	 *          no validation, and the behavior is undefined if done.
	 * @param data The data object to index.
	 */
	public void add(DATA data) {
		if(root == null) {
			root = new RootLeafNode(data);
			try {
				root.addData(data, 0);
			} catch (SplitNodeReplacement e) {
				throw new RuntimeException("Should never happen!");
			}
		} else {
			double distance = distanceFunction.calculate(data, root.data);
			try {
				root.addData(data, distance);
			} catch(SplitNodeReplacement e) {
				throw new RuntimeException("Not implemented");
				/*
				Node* newRoot = new RootNode(root->data);
				delete root;
				root = newRoot;
				for(int i = 0; i < SplitNodeReplacement::NUM_NODES; ++i) {
					Node* newNode = e.newNodes[i];
					double distance = distance_function(root->data, newNode->data);
					root->addChild(newNode, distance, this);
				}
				 */
			}
		}
	}


	/* *
	 * @brief Removes a data object from the M-Tree.
	 * @param data The data object to be removed.
	 * @return @c true if and only if the object was found.
	 */
	public boolean remove(DATA data) {
		if(root == null) {
			return false;
		}
		
		double distanceToRoot = distanceFunction.calculate(data, root.data);
		try {
			root.removeData(data, distanceToRoot);
		} catch(RootNodeReplacement e) {
			@SuppressWarnings("unchecked")
			Node newRoot = (Node) e.newRoot;
			root = newRoot;
		} catch(DataNotFound e) {
			return false;
		} catch (NodeUnderCapacity e) {
			throw new RuntimeException("Should have never happened", e);
		}
		return true;
	}

	/* *
	 * @brief Performs a nearest-neighbors query on the M-Tree, constrained by
	 *        distance.
	 * @param query_data The query data object.
	 * @param range The maximum distance from @c query_data to fetched neighbors.
	 * @return A @c query object.
	 */
	public Query getNearestByRange(DATA queryData, double range) {
		return getNearest(queryData, range, Integer.MAX_VALUE);
	}
	
	
	/* *
	 * @brief Performs a nearest-neighbors query on the M-Tree, constrained by
	 *        the number of neighbors.
	 * @param query_data The query data object.
	 * @param limit The maximum number of neighbors to fetch.
	 * @return A @c query object.
	 */
	public Query getNearestByLimit(DATA queryData, int limit) {
		return getNearest(queryData, Double.POSITIVE_INFINITY, limit);
	}

	/* *
	 * @brief Performs a nearest-neighbor query on the M-Tree, constrained by
	 *        distance and/or the number of neighbors.
	 * @param query_data The query data object.
	 * @param range The maximum distance from @c query_data to fetched neighbors.
	 * @param limit The maximum number of neighbors to fetch.
	 * @return A @c query object.
	 */
	public Query getNearest(DATA queryData, double range, int limit) {
		return new Query(queryData, range, limit);
	}

	/* *
	 * @brief Performs a nearest-neighbor query on the M-Tree, without
	 *        constraints.
	 * @param query_data The query data object.
	 * @return A @c query object.
	 * /
	query get_nearest(const Data& query_data) const {
		return {
			this,
			query_data,
			std::numeric_limits<double>::infinity(),
			std::numeric_limits<unsigned int>::max()
		};
	}
	*/
	
	protected void _check() {
		if(root != null) {
			root._check();
		}
	}
	
	/*
private:

	typedef std::pair<Data, Data> PromotedPair;
	typedef std::set<Data> Partition;

protected:
	SplitFunction split_function;
	*/

	private class IndexItem {
		DATA data;
		protected double radius;
		double distanceToParent;

		private IndexItem(DATA data) {
			this.data = data;
			this.radius = 0;
			this.distanceToParent = -1;
		}

		int _check() {
			_checkRadius();
			_checkDistanceToParent();
			return 1;
		}

		private void _checkRadius() {
			assert radius >= 0;
		}

		protected void _checkDistanceToParent() {
			assert !(this instanceof MTree.RootLeafNode);
			// assert !(this instanceof MTree.RootNode);	// TODO: pending
			assert distanceToParent >= 0;
		}
	}

	
	
	private abstract class Node extends IndexItem {

		protected Map<DATA, IndexItem> children = new HashMap<DATA, IndexItem>();
		protected Rootness rootness;
		protected Leafness<DATA> leafness;

		private
		<R extends NodeTrait & Rootness, L extends NodeTrait & Leafness<DATA>>
		Node(DATA data, R rootness, L leafness) {
			super(data);

			rootness.thisNode = this;
			this.rootness = rootness;

			leafness.thisNode = this;
			this.leafness = leafness;
		}

		private final void addData(DATA data, double distance) throws SplitNodeReplacement {
			doAddData(data, distance);
			checkMaxCapacity();
		}

		int _check() {
			super._check();
			_checkMinCapacity();
			_checkMaxCapacity();

			int childHeight = -1;
			for(Map.Entry<DATA, IndexItem> e : children.entrySet()) {
				DATA data = e.getKey();
				IndexItem child = e.getValue();
				assert child.data.equals(data);

				_checkChildClass(child);
				_checkChildMetrics(child);

				int height = child._check();
				if(childHeight < 0) {
					childHeight = height;
				} else {
					assert childHeight == height;
				}
			}

			return childHeight + 1;
		}

		protected void doAddData(DATA data, double distance) {
			leafness.doAddData(data, distance);
		}

		protected void doRemoveData(DATA data, double distance) throws DataNotFound {
			leafness.doRemoveData(data, distance);
		}

		private final void checkMaxCapacity() throws SplitNodeReplacement {
			if(children.size() > MTree.this.maxNodeCapacity) {
				throw new RuntimeException("Not implemented");
				/*
				Partition firstPartition;
				for(typename ChildrenMap::iterator i = children.begin(); i != children.end(); ++i) {
					firstPartition.insert(i->first);
				}

				cached_distance_function_type cachedDistanceFunction(mtree->distance_function);

				Partition secondPartition;
				PromotedPair promoted = mtree->split_function(firstPartition, secondPartition, cachedDistanceFunction);

				Node* newNodes[2];
				for(int i = 0; i < 2; ++i) {
					Data& promotedData    = (i == 0) ? promoted.first : promoted.second;
					Partition& partition = (i == 0) ? firstPartition : secondPartition;

					Node* newNode = newSplitNodeReplacement(promotedData);
					for(typename Partition::iterator j = partition.begin(); j != partition.end(); ++j) {
						const Data& data = *j;
						IndexItem* child = children[data];
						children.erase(data);
						double distance = cachedDistanceFunction(promotedData, data);
						newNode->addChild(child, distance, mtree);
					}

					newNodes[i] = newNode;
				}
				assert(children.empty());

				throw SplitNodeReplacement(newNodes);
				 */
			}

		}

		/*
	protected:
		virtual Node* newSplitNodeReplacement(const Data&) const = 0;

	public:
		virtual void addChild(IndexItem* child, double distance, const mtree* mtree) = 0;
		*/

		void removeData(DATA data, double distance) throws RootNodeReplacement, NodeUnderCapacity, DataNotFound {
			doRemoveData(data, distance);
			if(children.size() < getMinCapacity()) {
				throw new NodeUnderCapacity();
			}
		}

		abstract int getMinCapacity();

		private void updateMetrics(IndexItem child, double distance) {
			child.distanceToParent = distance;
			updateRadius(child);
		}

		private void updateRadius(IndexItem child) {
			this.radius = Math.max(this.radius, child.distanceToParent + child.radius);
		}

		void _checkMinCapacity() {
			rootness._checkMinCapacity();
		}

		private void _checkMaxCapacity() {
			assert children.size() <= MTree.this.maxNodeCapacity;
		}

		private void _checkChildClass(IndexItem child) {
			leafness._checkChildClass(child);
		}

		private void _checkChildMetrics(IndexItem child) {
			double dist = MTree.this.distanceFunction.calculate(child.data, this.data);
			assert child.distanceToParent == dist;

			double sum = child.distanceToParent + child.radius;
			assert sum <= this.radius;
		}

		private MTree<DATA> mtree() {
			return MTree.this;
		}
	}
	
	
	
	
	private abstract class NodeTrait {
		protected Node thisNode;
	}
	
	private interface Leafness<DATA> {
		void doAddData(DATA data, double distance);
		void doRemoveData(DATA data, double distance) throws DataNotFound;
		void _checkChildClass(MTree<DATA>.IndexItem child);
	}
	
	private interface Rootness {
		void _checkDistanceToParent();
		void _checkMinCapacity();
	}
	
	
	
	private class RootNodeTrait extends NodeTrait implements Rootness {
		
		@Override
		public void _checkDistanceToParent() {
			assert thisNode.distanceToParent == -1;
		}

		@Override
		public void _checkMinCapacity() {
			thisNode._checkMinCapacity();
		}
	};
	
	
	private class LeafNodeTrait extends NodeTrait implements Leafness<DATA> {
		
		public void doAddData(DATA data, double distance) {
			Entry entry = thisNode.mtree().new Entry(data);
			assert !thisNode.children.containsKey(data);
			thisNode.children.put(data, entry);
			assert thisNode.children.containsKey(data);
			thisNode.updateMetrics(entry, distance);
		}

		/*
		void addChild(IndexItem* child, double distance, const mtree* mtree) {
			assert(this->children.find(child->data) == this->children.end());
			this->children[child->data] = child;
			assert(this->children.find(child->data) != this->children.end());
			updateMetrics(child, distance);
		}

		Node* newSplitNodeReplacement(const Data& data) const {
			return new LeafNode(data);
		}
		*/

		@Override
		public void doRemoveData(DATA data, double distance) throws DataNotFound {
			if(thisNode.children.remove(data) == null) {
				throw new DataNotFound(data);
			}
		}

		public void _checkChildClass(IndexItem child) {
			assert child instanceof MTree.Entry;
		}
	}
	/*


	class NonLeafNodeTrait : public virtual Node {
		void doAddData(const Data& data, double distance, const mtree* mtree) {
			struct CandidateChild {
				Node* node;
				double distance;
				double metric;
			};

			CandidateChild minRadiusIncreaseNeeded = { NULL, -1.0, std::numeric_limits<double>::infinity() };
			CandidateChild nearestDistance         = { NULL, -1.0, std::numeric_limits<double>::infinity() };

			for(typename Node::ChildrenMap::iterator i = this->children.begin(); i != this->children.end(); ++i) {
				Node* child = dynamic_cast<Node*>(i->second);
				assert(child != NULL);
				double distance = mtree->distance_function(child->data, data);
				if(distance > child->radius) {
					double radiusIncrease = distance - child->radius;
					if(radiusIncrease < minRadiusIncreaseNeeded.metric) {
						minRadiusIncreaseNeeded = { child, distance, radiusIncrease };
					}
				} else {
					if(distance < nearestDistance.metric) {
						nearestDistance = { child, distance, distance };
					}
				}
			}

			CandidateChild chosen = (nearestDistance.node != NULL)
			                      ? nearestDistance
			                      : minRadiusIncreaseNeeded;

			Node* child = chosen.node;
			try {
				child->addData(data, chosen.distance, mtree);
				updateRadius(child);
			} catch(SplitNodeReplacement& e) {
				// Replace current child with new nodes
#ifndef NDEBUG
				size_t _ =
#endif
					this->children.erase(child->data);
				assert(_ == 1);
				delete child;

				for(int i = 0; i < e.NUM_NODES; ++i) {
					Node* newChild = e.newNodes[i];
					double distance = mtree->distance_function(this->data, newChild->data);
					addChild(newChild, distance, mtree);
				}
			}
		}


		void addChild(IndexItem* newChild_, double distance, const mtree* mtree) {
			Node* newChild = dynamic_cast<Node*>(newChild_);
			assert(newChild != NULL);

			struct ChildWithDistance {
				Node* child;
				double distance;
			};

			std::vector<ChildWithDistance> newChildren;
			newChildren.push_back(ChildWithDistance{newChild, distance});

			while(!newChildren.empty()) {
				ChildWithDistance cwd = newChildren.back();
				newChildren.pop_back();

				newChild = cwd.child;
				distance = cwd.distance;
				typename Node::ChildrenMap::iterator i = this->children.find(newChild->data);
				if(i == this->children.end()) {
					this->children[newChild->data] = newChild;
					updateMetrics(newChild, distance);
				} else {
					Node* existingChild = dynamic_cast<Node*>(this->children[newChild->data]);
					assert(existingChild != NULL);
					assert(existingChild->data == newChild->data);

					// Transfer the _children_ of the newChild to the existingChild
					for(typename Node::ChildrenMap::iterator i = newChild->children.begin(); i != newChild->children.end(); ++i) {
						IndexItem* grandchild = i->second;
						existingChild->addChild(grandchild, grandchild->distanceToParent, mtree);
					}
					newChild->children.clear();
					delete newChild;

					try {
						existingChild->checkMaxCapacity(mtree);
					} catch(SplitNodeReplacement& e) {
#ifndef NDEBUG
						size_t _ =
#endif
							this->children.erase(existingChild->data);
						assert(_ == 1);
						delete existingChild;

						for(int i = 0; i < e.NUM_NODES; ++i) {
							Node* newNode = e.newNodes[i];
							double distance = mtree->distance_function(this->data, newNode->data);
							newChildren.push_back(ChildWithDistance{newNode, distance});
						}
					}
				}
			}
		}


		Node* newSplitNodeReplacement(const Data& data) const {
			return new InternalNode(data);
		}


		void doRemoveData(const Data& data, double distance, const mtree* mtree) throw (DataNotFound) {
			for(typename Node::ChildrenMap::iterator i = this->children.begin(); i != this->children.end(); ++i) {
				Node* child = dynamic_cast<Node*>(i->second);
				assert(child != NULL);
				if(abs(distance - child->distanceToParent) <= child->radius) {
					double distanceToChild = mtree->distance_function(data, child->data);
					if(distanceToChild <= child->radius) {
						try {
							child->removeData(data, distanceToChild, mtree);
							updateRadius(child);
							return;
						} catch(DataNotFound&) {
							// If DataNotFound was thrown, then the data was not found in the child
						} catch(NodeUnderCapacity&) {
							Node* expandedChild = balanceChildren(child, mtree);
							updateRadius(expandedChild);
							return;
						}
					}
				}
			}

			throw DataNotFound{data};
		}


		Node* balanceChildren(Node* theChild, const mtree* mtree) {
			// Tries to find anotherChild which can donate a grand-child to theChild.

			Node* nearestDonor = NULL;
			double distanceNearestDonor = std::numeric_limits<double>::infinity();

			Node* nearestMergeCandidate = NULL;
			double distanceNearestMergeCandidate = std::numeric_limits<double>::infinity();

			for(typename Node::ChildrenMap::iterator i = this->children.begin(); i != this->children.end(); ++i) {
				Node* anotherChild = dynamic_cast<Node*>(i->second);
				assert(anotherChild != NULL);
				if(anotherChild == theChild) continue;

				double distance = mtree->distance_function(theChild->data, anotherChild->data);
				if(anotherChild->children.size() > anotherChild->getMinCapacity(mtree)) {
					if(distance < distanceNearestDonor) {
						distanceNearestDonor = distance;
						nearestDonor = anotherChild;
					}
				} else {
					if(distance < distanceNearestMergeCandidate) {
						distanceNearestMergeCandidate = distance;
						nearestMergeCandidate = anotherChild;
					}
				}
			}

			if(nearestDonor == NULL) {
				// Merge
				for(typename Node::ChildrenMap::iterator i = theChild->children.begin(); i != theChild->children.end(); ++i) {
					IndexItem* grandchild = i->second;
					double distance = mtree->distance_function(grandchild->data, nearestMergeCandidate->data);
					nearestMergeCandidate->addChild(grandchild, distance, mtree);
				}

				theChild->children.clear();
				this->children.erase(theChild->data);
				delete theChild;
				return nearestMergeCandidate;
			} else {
				// Donate
				// Look for the nearest grandchild
				IndexItem* nearestGrandchild;
				double nearestGrandchildDistance = std::numeric_limits<double>::infinity();
				for(typename Node::ChildrenMap::iterator i = nearestDonor->children.begin(); i != nearestDonor->children.end(); ++i) {
					IndexItem* grandchild = i->second;
					double distance = mtree->distance_function(grandchild->data, theChild->data);
					if(distance < nearestGrandchildDistance) {
						nearestGrandchildDistance = distance;
						nearestGrandchild = grandchild;
					}
				}

#ifndef NDEBUG
				size_t _ =
#endif
					nearestDonor->children.erase(nearestGrandchild->data);
				assert(_ == 1);
				theChild->addChild(nearestGrandchild, nearestGrandchildDistance, mtree);
				return theChild;
			}
		}


		void _checkChildClass(IndexItem* child) const {
			assert(dynamic_cast<InternalNode*>(child) != NULL
			   ||  dynamic_cast<LeafNode*>(child)     != NULL);
		}
	};
	*/


	private class RootLeafNode extends Node {
		
		private RootLeafNode(DATA data) {
			super(data, new RootNodeTrait(), new LeafNodeTrait());
		}
		
		void removeData(DATA data, double distance) throws RootNodeReplacement, DataNotFound {
			try {
				super.removeData(data, distance);
			} catch (NodeUnderCapacity e) {
				assert children.isEmpty();
				throw new RootNodeReplacement(null);
			}
		}
		
		protected void _checkDistanceToParent() {
			rootness._checkDistanceToParent();
		}

		int getMinCapacity() {
			return 1;
		}

		void _checkMinCapacity() {
			assert children.size() >= 1;
		}
	}
	/*

	class RootNode : public RootNodeTrait, public NonLeafNodeTrait {
	public:
		RootNode(const Data& data) : Node(data) {}

	private:
		void removeData(const Data& data, double distance, const mtree* mtree) throw (RootNodeReplacement, NodeUnderCapacity, DataNotFound) {
			try {
				Node::removeData(data, distance, mtree);
			} catch(NodeUnderCapacity&) {
				// Promote the only child to root
				Node* theChild = dynamic_cast<Node*>(this->children.begin()->second);
				Node* newRoot;
				if(dynamic_cast<InternalNode*>(theChild) != NULL) {
					newRoot = new RootNode(theChild->data);
				} else {
					assert(dynamic_cast<LeafNode*>(theChild) != NULL);
					newRoot = new RootLeafNode(theChild->data);
				}

				for(typename Node::ChildrenMap::iterator i = theChild->children.begin(); i != theChild->children.end(); ++i) {
					IndexItem* grandchild = i->second;
					double distance = mtree->distance_function(newRoot->data, grandchild->data);
					newRoot->addChild(grandchild, distance, mtree);
				}
				theChild->children.clear();

				throw RootNodeReplacement{newRoot};
			}
		}


		size_t getMinCapacity(const mtree* mtree) const {
			return 2;
		}

		void _checkMinCapacity(const mtree* mtree) const {
			assert(this->children.size() >= 2);
		}
	};


	class InternalNode : public NonRootNodeTrait, public NonLeafNodeTrait {
	public:
		InternalNode(const Data& data) : Node(data) { }
	};


	class LeafNode : public NonRootNodeTrait, public LeafNodeTrait {
	public:
		LeafNode(const Data& data) : Node(data) { }
	};

	 */

	private class Entry extends IndexItem {
		private Entry(DATA data) {
			super(data);
		}
	}
}
