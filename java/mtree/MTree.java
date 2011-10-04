package mtree;

import java.util.ArrayDeque;
import java.util.Deque;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Map;
import java.util.NoSuchElementException;
import java.util.PriorityQueue;
import java.util.Set;


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
		// A subclass of Throwable cannot be generic.  :-(
		// So, we have newNodes declared as Object[] instead of Node[].
		private Object newNodes[];
		
		private SplitNodeReplacement(Object... newNodes) {
			this.newNodes = newNodes;
		}
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
									@SuppressWarnings("unchecked")
									Node childNode = (Node)child;
									pendingQueue.add(new ItemWithDistances<Node>(childNode, childDistance, childMinDistance));
								}
							}
						}
					}
					
					if(pendingQueue.isEmpty()) {
						nextPendingMinDistance = Double.POSITIVE_INFINITY;
					} else {
						nextPendingMinDistance = pendingQueue.peek().minDistance;
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
	protected SplitFunction<DATA> splitFunction;
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
	public MTree(DistanceFunction<? super DATA> distanceFunction,
			SplitFunction<DATA> splitFunction) {
		this(DEFAULT_MIN_NODE_CAPACITY, distanceFunction, splitFunction);
	}
	
	public MTree(int minNodeCapacity,
			DistanceFunction<? super DATA> distanceFunction,
			SplitFunction<DATA> splitFunction) {
		this(minNodeCapacity, 2 * minNodeCapacity - 1, distanceFunction, splitFunction);
	}
	
	public MTree(int minNodeCapacity, int maxNodeCapacity,
			DistanceFunction<? super DATA> distanceFunction,
			SplitFunction<DATA> splitFunction)
	{
		if(minNodeCapacity < 2  ||  maxNodeCapacity <= minNodeCapacity  ||
		   distanceFunction == null  ||  splitFunction == null) {
			throw new IllegalArgumentException();
		}
		
		this.minNodeCapacity = minNodeCapacity;
		this.maxNodeCapacity = maxNodeCapacity;
		this.distanceFunction = distanceFunction;
		this.splitFunction = splitFunction;
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
				Node newRoot = new RootNode(data);
				root = newRoot;
				for(int i = 0; i < e.newNodes.length; i++) {
					@SuppressWarnings("unchecked")
					Node newNode = (Node) e.newNodes[i];
					distance = distanceFunction.calculate(root.data, newNode.data);
					root.addChild(newNode, distance);
				}
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
			assert !(this instanceof MTree.RootNode);
			assert distanceToParent >= 0;
		}
	}

	
	
	private abstract class Node extends IndexItem {

		protected Map<DATA, IndexItem> children = new HashMap<DATA, IndexItem>();
		protected Rootness       rootness;
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
				Set<DATA> firstPartition = new HashSet<DATA>();
				for(DATA data : children.keySet()) {
					firstPartition.add(data);
				}
				
				DistanceFunction<? super DATA> cachedDistanceFunction = DistanceFunctions.cached(MTree.this.distanceFunction);
				
				Set<DATA> secondPartition = new HashSet<DATA>();
				DATA[] promoted = MTree.this.splitFunction.process(firstPartition, secondPartition, cachedDistanceFunction);
				
				Node newNode0 = null;
				Node newNode1 = null;
				for(int i = 0; i < 2; ++i) {
					DATA promotedData   = promoted[i];
					Set<DATA> partition = (i == 0) ? firstPartition : secondPartition;
					
					Node newNode = newSplitNodeReplacement(promotedData);
					for(DATA data : partition) {
						IndexItem child = children.get(data);
						children.remove(data);
						double distance = cachedDistanceFunction.calculate(promotedData, data);
						newNode.addChild(child, distance);
					}

					if(i == 0) {
						newNode0 = newNode;
					} else {
						newNode1 = newNode;
					}
				}
				assert children.isEmpty();

				throw new SplitNodeReplacement(newNode0, newNode1);
			}

		}

		protected Node newSplitNodeReplacement(DATA data) {
			return leafness.newSplitNodeReplacement(data);
		}

		protected void addChild(IndexItem child, double distance) {
			leafness.addChild(child, distance);
		}

		void removeData(DATA data, double distance) throws RootNodeReplacement, NodeUnderCapacity, DataNotFound {
			doRemoveData(data, distance);
			if(children.size() < getMinCapacity()) {
				throw new NodeUnderCapacity();
			}
		}

		protected int getMinCapacity() {
			return rootness.getMinCapacity();
		}

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
		
		protected void _checkDistanceToParent() {
			rootness._checkDistanceToParent();
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
		void addChild(MTree<DATA>.IndexItem child, double distance);
		void doRemoveData(DATA data, double distance) throws DataNotFound;
		MTree<DATA>.Node newSplitNodeReplacement(DATA data);
		void _checkChildClass(MTree<DATA>.IndexItem child);
	}
	
	private interface Rootness {
		int getMinCapacity();
		void _checkDistanceToParent();
		void _checkMinCapacity();
	}
	
	
	
	private class RootNodeTrait extends NodeTrait implements Rootness {
		
		@Override
		public int getMinCapacity() {
			throw new RuntimeException("Should not be called!");
		}
		
		@Override
		public void _checkDistanceToParent() {
			assert thisNode.distanceToParent == -1;
		}

		@Override
		public void _checkMinCapacity() {
			thisNode._checkMinCapacity();
		}
		
	};


	private class NonRootNodeTrait extends NodeTrait implements Rootness {
		
		@Override
		public int getMinCapacity() {
			return MTree.this.minNodeCapacity;
		}
		
		@Override
		public void _checkMinCapacity() {
			assert thisNode.children.size() >= thisNode.mtree().minNodeCapacity;
		}
		
		@Override
		public void _checkDistanceToParent() {
			assert thisNode.distanceToParent >= 0;
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

		public void addChild(IndexItem child, double distance) {
			assert !thisNode.children.containsKey(child.data);
			thisNode.children.put(child.data, child);
			assert thisNode.children.containsKey(child.data);
			thisNode.updateMetrics(child, distance);
		}
		
		public Node newSplitNodeReplacement(DATA data) {
			return thisNode.mtree().new LeafNode(data);
		}
		

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


	class NonLeafNodeTrait extends NodeTrait implements Leafness<DATA> {
		
		public void doAddData(DATA data, double distance) {
			throw new RuntimeException("Not implemented");
			/*
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
			*/
		}
		

		public void addChild(IndexItem newChild_, double distance) {
			@SuppressWarnings("unchecked")
			Node newChild = (Node) newChild_;
			
			class ChildWithDistance {
				Node child;
				double distance;
				private ChildWithDistance(Node child, double distance) {
					this.child = child;
					this.distance = distance;
				}
			}
			
			Deque<ChildWithDistance> newChildren = new ArrayDeque<ChildWithDistance>();
			newChildren.addFirst(new ChildWithDistance(newChild, distance));
			
			while(!newChildren.isEmpty()) {
				ChildWithDistance cwd = newChildren.removeFirst();
				
				newChild = cwd.child;
				distance = cwd.distance;
				if(thisNode.children.containsKey(newChild.data)) {
					throw new RuntimeException("Not implemented");
					/*
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
					*/
				} else {
					thisNode.children.put(newChild.data, newChild);
					thisNode.updateMetrics(newChild, distance);
				}
			}
		}


		public Node newSplitNodeReplacement(DATA data) {
			throw new RuntimeException("Not implemented");
			/*
			return new InternalNode(data);
			*/
		}


		public void doRemoveData(DATA data, double distance) throws DataNotFound {
			for(IndexItem childItem : thisNode.children.values()) {
				@SuppressWarnings("unchecked")
				Node child = (Node)childItem;
				if(Math.abs(distance - child.distanceToParent) <= child.radius) {
					double distanceToChild = thisNode.mtree().distanceFunction.calculate(data, child.data);
					if(distanceToChild <= child.radius) {
						try {
							child.removeData(data, distanceToChild);
							throw new RuntimeException("Not implemented");
							/*
							updateRadius(child);
							return;
							*/
						} catch(DataNotFound e) {
							// If DataNotFound was thrown, then the data was not found in the child
						} catch(NodeUnderCapacity e) {
							Node expandedChild = balanceChildren(child);
							thisNode.updateRadius(expandedChild);
							return;
						} catch (RootNodeReplacement e) {
							throw new RuntimeException("Should never happen!");
						}
					}
				}
			}
			throw new RuntimeException("Not implemented");
			/*
			
			throw DataNotFound{data};
			*/
		}

		
		private Node balanceChildren(Node theChild) {
			// Tries to find anotherChild which can donate a grand-child to theChild.
			
			Node nearestDonor = null;
			double distanceNearestDonor = Double.POSITIVE_INFINITY;
			
			Node nearestMergeCandidate = null;
			double distanceNearestMergeCandidate = Double.POSITIVE_INFINITY;

			for(IndexItem child : thisNode.children.values()) {
				@SuppressWarnings("unchecked")
				Node anotherChild = (Node)child;
				if(anotherChild == theChild) continue;

				double distance = thisNode.mtree().distanceFunction.calculate(theChild.data, anotherChild.data);
				if(anotherChild.children.size() > anotherChild.getMinCapacity()) {
					throw new RuntimeException("Not implemented");
					/*
					if(distance < distanceNearestDonor) {
						distanceNearestDonor = distance;
						nearestDonor = anotherChild;
					}
					*/
				} else {
					if(distance < distanceNearestMergeCandidate) {
						distanceNearestMergeCandidate = distance;
						nearestMergeCandidate = anotherChild;
					}
				}
			}

			if(nearestDonor == null) {
				// Merge
				for(IndexItem grandchild : theChild.children.values()) {
					double distance = thisNode.mtree().distanceFunction.calculate(grandchild.data, nearestMergeCandidate.data);
					nearestMergeCandidate.addChild(grandchild, distance);
				}

				IndexItem removed = thisNode.children.remove(theChild.data);
				assert removed != null;
				return nearestMergeCandidate;
			} else {
				throw new RuntimeException("Not implemented");
				/*
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
				 */
			}
		}


		public void _checkChildClass(IndexItem child) {
			assert child instanceof MTree.InternalNode
			    || child instanceof MTree.LeafNode;
		}
	}


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
		
		protected int getMinCapacity() {
			return 1;
		}

		void _checkMinCapacity() {
			assert children.size() >= 1;
		}
	}

	private class RootNode extends Node {

		private RootNode(DATA data) {
			super(data, new RootNodeTrait(), new NonLeafNodeTrait());
		}
		
		void removeData(DATA data, double distance) throws RootNodeReplacement, NodeUnderCapacity, DataNotFound {
			try {
				super.removeData(data, distance);
			} catch(NodeUnderCapacity e) {
				// Promote the only child to root
				@SuppressWarnings("unchecked")
				Node theChild = (Node)(children.values().iterator().next());
				Node newRoot;
				if(theChild instanceof MTree.InternalNode) {
					newRoot = new RootNode(theChild.data);
				} else {
					assert theChild instanceof MTree.LeafNode;
					newRoot = new RootLeafNode(theChild.data);
				}

				for(IndexItem grandchild : theChild.children.values()) {
					distance = MTree.this.distanceFunction.calculate(newRoot.data, grandchild.data);
					newRoot.addChild(grandchild, distance);
				}
				theChild.children.clear();

				throw new RootNodeReplacement(newRoot);
			}
		}
		
		
		@Override
		protected int getMinCapacity() {
			return 2;
		}
		
		@Override
		void _checkMinCapacity() {
			assert children.size() >= 2;
		}
	}


	private class InternalNode extends Node {
		private InternalNode(DATA data) {
			super(data, new NonRootNodeTrait(), new NonLeafNodeTrait());
		}
	};


	private class LeafNode extends Node {

		public LeafNode(DATA data) {
			super(data, new NonRootNodeTrait(), new LeafNodeTrait());
		}
	}


	private class Entry extends IndexItem {
		private Entry(DATA data) {
			super(data);
		}
	}
}
