#ifndef MTREE_H_
#define MTREE_H_


#include <iterator>
#include <limits>
#include <map>
#include <queue>
#include <utility>
#include "functions.h"



namespace mtree {



/**
 * @brief The main class that implements the M-Tree.
 *
 * @tparam Data The type of data that will be indexed by the M-Tree. This type
 *         must be an assignable type and a strict weak ordering must be defined
 *         by @c std::less<Data>.
 * @tparam DistanceFunction The type of the function that will be used to
 *         calculate the distance between two @c Data objects. By default, it is
 *         ::mtree::functions::euclidean_distance.
 * @tparam SplitFunction The type of the function that will be used to split a
 *         node when it is at its maximum capacity and a new child must be
 *         added. By default, it is a composition of
 *         ::mtree::functions::random_promotion and
 *         ::mtree::functions::balanced_partition.
 *
 *
 * @bug There should be a way of specifying a Compare type and value for @c Data
 *      instead of implicitly using @c std::less<Data>.
 *
 * @bug There should be a way of using unordered instead of ordered @c Data (by
 *      using @c std::unordered_set instead of @c std::set and @c
 *      std::unordered_map instead of @c std::map.
 */
template <
	typename Data,
	typename DistanceFunction = ::mtree::functions::euclidean_distance,
	typename SplitFunction = ::mtree::functions::split_function<
	        ::mtree::functions::random_promotion,
	        ::mtree::functions::balanced_partition
		>
>
class mtree {
public:
	typedef DistanceFunction distance_function_type;
	typedef SplitFunction    split_function_type;
	typedef functions::cached_distance_function<Data, DistanceFunction> cached_distance_function_type;

private:
	class Node;
	class Entry;


	// Exception classes
	class SplitNodeReplacement {
	public:
		enum { NUM_NODES = 2 };
		Node* newNodes[NUM_NODES];
		SplitNodeReplacement(Node* newNodes[NUM_NODES]) {
			for(int i = 0; i < NUM_NODES; ++i) {
				this->newNodes[i] = newNodes[i];
			}

		}
	};

	class RootNodeReplacement {
	public:
		Node* newRoot;
	};

	class NodeUnderCapacity { };

	class DataNotFound {
	public:
		Data data;
	};


public:

	class result_item {
	public:
		Data data;
		double distance;

		result_item() = default;
		result_item(const result_item&) = default;
		~result_item() = default;
		result_item& operator=(const result_item&) = default;
	};


	/**
	 * @brief A container-like class which can be iterated to fetch the results
	 *        of a nearest-neighbors query.
	 * @details The neighbors are presented in non-decreasing order from the
	 *          @c queryData argument to the mtree::get_nearest() call.
	 *
	 *          The query on the M-Tree is executed during the iteration, as the
	 *          results are fetched. It means that, by the time when the @a n-th
	 *          result is fetched, not all results are known, and the resources
	 *          allocated were only the necessary to identify the @a n first
	 *          results.
	 *
	 *          The objects in the container are mtree::query_result instances,
	 *          which contain a data object and the distance from the query
	 *          data object.
	 * @see mtree::get_nearest()
	 */
	class query {
	public:
		typedef result_item value_type;


		query() = delete;

		query(const query&) = default;

		query(query&&) = default;

		query(const mtree* _mtree, const Data& data, double range, size_t limit)
			: _mtree(_mtree), data(data), range(range), limit(limit)
			{}


		query& operator=(const query&) = default;

		query& operator=(query&& that) {
			// Trivial data is copied
			this->_mtree = that._mtree;
			this->range = that.range;
			this->limit = that.limit;
			// Potential complex data is swapped
			std::swap(this->data, that.data);

			return *this;
		}


		class iterator {
		public:
			typedef std::input_iterator_tag iterator_category;
			typedef result_item             value_type;
			typedef signed long int         difference_type;
			typedef result_item*            pointer;
			typedef result_item&            reference;


			iterator() : isEnd(true) {}


			iterator(const query* _query)
				: _query(_query),
				  isEnd(false),
				  yieldedCount(0)
			{
				if(_query->_mtree->root == NULL) {
					isEnd = true;
					return;
				}

				double distance = _query->_mtree->distance_function(_query->data, _query->_mtree->root->data);
				double minDistance = std::max(distance - _query->_mtree->root->radius, 0.0);

				pendingQueue.push({_query->_mtree->root, distance, minDistance});
				nextPendingMinDistance = minDistance;

				fetchNext();
			}


			iterator(const iterator&) = default;

			iterator(iterator&&) = default;

			~iterator() = default;

			iterator& operator=(const iterator&) = default;

			iterator& operator=(iterator&& that) {
				// Simple data is copied
				// Potential complex data is swapped

				this->_query = that._query;
				std::swap(this->currentResultItem.data, that.currentResultItem.data);
				this->currentResultItem.distance = that.currentResultItem.distance;
				this->isEnd = that.isEnd;
				std::swap(this->pendingQueue, that.pendingQueue);
				this->nextPendingMinDistance = that.nextPendingMinDistance;
				std::swap(this->nearestQueue, that.nearestQueue);
				this->yieldedCount = that.yieldedCount;

				return *this;
			}


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

			bool operator!=(const iterator& ri) const {
				return ! this->operator==(ri);
			}

			// prefix
			iterator& operator++() {
				fetchNext();
				return *this;
			}

			// postfix
			iterator operator++(int) {
				iterator aCopy = *this;
				operator++();
				return aCopy;
			}

			const result_item& operator*() const {
				return currentResultItem;
			}

			const result_item* operator->() const {
				return &currentResultItem;
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
					return (this->minDistance > that.minDistance);
				}

			};

			void fetchNext() {
				assert(! isEnd);

				if(isEnd  ||  yieldedCount >= _query->limit) {
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
						if(std::abs(pending.distance - child->distanceToParent) - child->radius <= _query->range) {
							double childDistance = _query->_mtree->distance_function(_query->data, child->data);
							double childMinDistance = std::max(childDistance - child->radius, 0.0);
							if(childMinDistance <= _query->range) {
								Entry* entry = dynamic_cast<Entry*>(child);
								if(entry != NULL) {
									nearestQueue.push({entry, childDistance, childMinDistance});
								} else {
									Node* node = dynamic_cast<Node*>(child);
									assert(node != NULL);
									pendingQueue.push({node, childDistance, childMinDistance});
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
			}


			bool prepareNextNearest() {
				if(!nearestQueue.empty()) {
					ItemWithDistances<Entry> nextNearest = nearestQueue.top();
					if(nextNearest.distance <= nextPendingMinDistance) {
						nearestQueue.pop();
						currentResultItem.data = nextNearest.item->data;
						currentResultItem.distance = nextNearest.distance;
						++yieldedCount;
						return true;
					}
				}

				return false;
			}

		private:
			const query* _query;
			result_item currentResultItem;
			bool isEnd;
			std::priority_queue<ItemWithDistances<Node>> pendingQueue;
			double nextPendingMinDistance;
			std::priority_queue<ItemWithDistances<Entry>> nearestQueue;
			size_t yieldedCount;
		};


		iterator begin() const {
			return iterator(this);
		}


		iterator end() const {
			return {};
		}

	private:
		const mtree* _mtree;
		Data data;
		double range;
		size_t limit;
	};



	enum {
		/**
		 * @brief The default minimum capacity of nodes in an M-Tree, when not
		 * specified in the constructor call.
		 */
		DEFAULT_MIN_NODE_CAPACITY = 50
	};


	/**
	 * The main constructor of an M-Tree.
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
	explicit mtree(
			size_t min_node_capacity = DEFAULT_MIN_NODE_CAPACITY,
			size_t max_node_capacity = -1,
			const DistanceFunction& distance_function = DistanceFunction(),
			const SplitFunction& split_function = SplitFunction()
		)
		: minNodeCapacity(min_node_capacity),
		  maxNodeCapacity(max_node_capacity),
		  root(NULL),
		  distance_function(distance_function),
		  split_function(split_function)
	{
		if(max_node_capacity == size_t(-1)) {
			this->maxNodeCapacity = 2 * min_node_capacity - 1;
		}
	}

	// Cannot copy!
	/**
	 * @brief <span style="color:red">Deleted</span> copy constructor.
	 * @details It cannot be used!
	 */
	mtree(const mtree&) = delete;

	// ... but moving is ok.
	/**
	 * @brief Move constructor.
	 * @param that An M-Tree rvalue reference.
	 */
	mtree(mtree&& that)
		: root(that.root),
		  maxNodeCapacity(that.maxNodeCapacity),
		  minNodeCapacity(that.minNodeCapacity),
		  distance_function(that.distance_function),
		  split_function(that.split_function)
	{
		that.root = 0;
	}


	/**
	 * @brief The destructor.
	 */
	~mtree() {
		delete root;
	}

	// Cannot copy!
	/**
	 * @brief <span style="color:red">Deleted</span> copy assignment.
	 * @details It cannot be used!
	 */
	mtree& operator=(const mtree&) = delete;

	// ... but moving is ok.
	/**
	 * @brief Move assignment.
	 * @param that An M-Tree rvalue reference.
	 */
	mtree& operator=(mtree&& that) {
		if(&that != this) {
			std::swap(this->root, that.root);
			this->minNodeCapacity = that.minNodeCapacity;
			this->maxNodeCapacity = that.maxNodeCapacity;
			this->distance_function = that.distance_function;
			this->split_function = that.split_function;
		}
		return *this;
	}


	/**
	 * @brief Adds and indexes a data object.
	 * @details An object that is already indexed should not be added. There is
	 *          no validation, and the behavior is undefined if done.
	 * @param data The data object to index.
	 */
	void add(const Data& data) {
		if(root == NULL) {
			root = new RootLeafNode(data);
			root->addData(data, 0, this);
		} else {
			double distance = distance_function(data, root->data);
			try {
				root->addData(data, distance, this);
			} catch(SplitNodeReplacement& e) {
				Node* newRoot = new RootNode(root->data);
				delete root;
				root = newRoot;
				for(int i = 0; i < SplitNodeReplacement::NUM_NODES; ++i) {
					Node* newNode = e.newNodes[i];
					double distance = distance_function(root->data, newNode->data);
					root->addChild(newNode, distance, this);
				}
			}
		}
	}


	/**
	 * @brief Removes a data object from the M-Tree.
	 * @param data The data object to be removed.
	 * @return @c true if and only if the object was found.
	 */
	bool remove(const Data& data) {
		if(root == NULL) {
			return false;
		}

		double distanceToRoot = distance_function(data, root->data);
		try {
			root->removeData(data, distanceToRoot, this);
		} catch(RootNodeReplacement& e) {
			delete root;
			root = e.newRoot;
		} catch(DataNotFound) {
			return false;
		}
		return true;
	}


	/**
	 * @brief Performs a nearest-neighbors query on the M-Tree, constrained by
	 *        distance.
	 * @param queryData The query data object.
	 * @param range The maximum distance from @c queryData to fetched neighbors.
	 * @return A @c query object.
	 */
	query get_nearest_by_range(const Data& queryData, double range) const {
		return get_nearest(queryData, range, std::numeric_limits<unsigned int>::max());
	}

	/**
	 * @brief Performs a nearest-neighbors query on the M-Tree, constrained by
	 *        the number of neighbors.
	 * @param queryData The query data object.
	 * @param limit The maximum number of neighbors to fetch.
	 * @return A @c query object.
	 */
	query get_nearest_by_limit(const Data& queryData, size_t limit) const {
		return get_nearest(queryData, std::numeric_limits<double>::infinity(), limit);
	}

	/**
	 * @brief Performs a nearest-neighbor query on the M-Tree, constrained by
	 *        distance and/or the number of neighbors.
	 * @param queryData The query data object.
	 * @param range The maximum distance from @c queryData to fetched neighbors.
	 * @param limit The maximum number of neighbors to fetch.
	 * @return A @c query object.
	 */
	query get_nearest(const Data& queryData, double range, size_t limit) const {
		return {this, queryData, range, limit};
	}

	/**
	 * @brief Performs a nearest-neighbor query on the M-Tree, without
	 *        constraints.
	 * @param queryData The query data object.
	 * @return A @c query object.
	 */
	query get_nearest(const Data& queryData) const {
		return {
			this,
			queryData,
			std::numeric_limits<double>::infinity(),
			std::numeric_limits<unsigned int>::max()
		};
	}

protected:

	void _check() const {
#ifndef NDEBUG
			if(root != NULL) {
				root->_check(this);
			}
#endif
	}

private:

	typedef std::pair<Data, Data> PromotedPair;
	typedef std::set<Data> Partition;


	size_t minNodeCapacity;
	size_t maxNodeCapacity;
	Node* root;

protected:
	DistanceFunction distance_function;
	SplitFunction split_function;

public:
	class IndexItem {
	public:
		Data data;
		double radius;
		double distanceToParent;

		virtual ~IndexItem() { };

		IndexItem() = delete;
		IndexItem(const IndexItem&) = delete;
		IndexItem(IndexItem&&) = delete;
		IndexItem& operator=(const IndexItem&) = delete;
		IndexItem& operator=(IndexItem&&) = delete;

	protected:
		IndexItem(const Data& data)
			: data(data),
			  radius(0),
			  distanceToParent(-1)
			{ }

	public:
		virtual size_t _check(const mtree* mtree) const {
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
	};


private:
	class Node : public IndexItem {
	public:
		virtual ~Node() {
			for(typename ChildrenMap::iterator i = children.begin(); i != children.end(); ++i) {
				IndexItem* child = i->second;
				delete child;
			}
		}

		void addData(const Data& data, double distance, const mtree* mtree) throw(SplitNodeReplacement) {
			doAddData(data, distance, mtree);
			checkMaxCapacity(mtree);
		}

#ifndef NDEBUG
		size_t _check(const mtree* mtree) const {
			IndexItem::_check(mtree);
			_checkMinCapacity(mtree);
			_checkMaxCapacity(mtree);

			bool   childHeightKnown = false;
			size_t childHeight;
			for(typename ChildrenMap::const_iterator i = children.begin(); i != children.end(); ++i) {
#ifndef NDEBUG
				const Data& data = i->first;
#endif
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
#endif

		typedef std::map<Data, IndexItem*> ChildrenMap;

		ChildrenMap children;

	protected:
		Node(const Data& data) : IndexItem(data) { }

		Node() : IndexItem(*((Data*)(0))) { assert(!"THIS SHOULD NEVER BE CALLED"); };

		Node(const Node&) = delete;
		Node(Node&&) = delete;
		Node& operator=(const Node&) = delete;
		Node& operator=(Node&&) = delete;

		virtual void doAddData(const Data& data, double distance, const mtree* mtree) = 0;

		virtual void doRemoveData(const Data& data, double distance, const mtree* mtree) throw (DataNotFound) = 0;

	public:
		void checkMaxCapacity(const mtree* mtree) throw (SplitNodeReplacement) {
			if(children.size() > mtree->maxNodeCapacity) {
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
			}

		}

	protected:
		virtual Node* newSplitNodeReplacement(const Data&) const = 0;

	public:
		virtual void addChild(IndexItem* child, double distance, const mtree* mtree) = 0;

		virtual void removeData(const Data& data, double distance, const mtree* mtree) throw (RootNodeReplacement, NodeUnderCapacity, DataNotFound) {
			doRemoveData(data, distance, mtree);
			if(children.size() < getMinCapacity(mtree)) {
				throw NodeUnderCapacity();
			}
		}

		virtual size_t getMinCapacity(const mtree* mtree) const = 0;

	protected:
		void updateMetrics(IndexItem* child, double distance) {
			child->distanceToParent = distance;
			updateRadius(child);
		}

		void updateRadius(IndexItem* child) {
			this->radius = std::max(this->radius, child->distanceToParent + child->radius);
		}


		virtual void _checkMinCapacity(const mtree* mtree) const = 0;

	private:
		void _checkMaxCapacity(const mtree* mtree) const {
			assert(children.size() <= mtree->maxNodeCapacity);
		}

	protected:
		virtual void _checkChildClass(IndexItem* child) const = 0;

	private:
#ifndef NDEBUG
		void _checkChildMetrics(IndexItem* child, const mtree* mtree) const {
			double dist = mtree->distance_function(child->data, this->data);
			assert(child->distanceToParent == dist);

			/* TODO: investigate why the following line
			 * 		assert(child->distanceToParent + child->radius <= this->radius);
			 * is not the same as the code below:
			 */
			double sum = child->distanceToParent + child->radius;
			assert(sum <= this->radius);
		}
#endif
	};


	class RootNodeTrait : public virtual Node {
		void _checkDistanceToParent() const {
			assert(this->distanceToParent == -1);
		}
	};


	class NonRootNodeTrait : public virtual Node {
		size_t getMinCapacity(const mtree* mtree) const {
			return mtree->minNodeCapacity;
		}

		void _checkMinCapacity(const mtree* mtree) const {
			assert(this->children.size() >= mtree->minNodeCapacity);
		}
	};


	class LeafNodeTrait : public virtual Node {
		void doAddData(const Data& data, double distance, const mtree* mtree) {
			Entry* entry = new Entry(data);
			assert(this->children.find(data) == this->children.end());
			this->children[data] = entry;
			assert(this->children.find(data) != this->children.end());
			updateMetrics(entry, distance);
		}

		void addChild(IndexItem* child, double distance, const mtree* mtree) {
			assert(this->children.find(child->data) == this->children.end());
			this->children[child->data] = child;
			assert(this->children.find(child->data) != this->children.end());
			updateMetrics(child, distance);
		}

		Node* newSplitNodeReplacement(const Data& data) const {
			return new LeafNode(data);
		}

		void doRemoveData(const Data& data, double distance, const mtree* mtree) throw (DataNotFound) {
			if(this->children.erase(data) == 0) {
				throw DataNotFound{data};
			}
		}


		void _checkChildClass(IndexItem* child) const {
			assert(dynamic_cast<Entry*>(child) != NULL);
		}
	};


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


	class RootLeafNode : public RootNodeTrait, public LeafNodeTrait {
	public:
		RootLeafNode(const Data& data) : Node(data) { }

		void removeData(const Data& data, double distance, const mtree* mtree) throw (RootNodeReplacement, DataNotFound) {
			try {
				Node::removeData(data, distance, mtree);
			} catch (NodeUnderCapacity&) {
				assert(this->children.empty());
				throw RootNodeReplacement{NULL};
			}
		}

		size_t getMinCapacity(const mtree* mtree) const {
			return 1;
		}

		void _checkMinCapacity(const mtree* mtree) const {
			assert(this->children.size() >= 1);
		}
	};

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


	class Entry : public IndexItem {
	public:
		Entry(const Data& data) : IndexItem(data) { }
	};
};



} /* namespace mtree */



#endif /* MTREE_H_ */
