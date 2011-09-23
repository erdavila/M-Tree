#ifndef MTREE_H_
#define MTREE_H_


#include <iterator>
#include <limits>
#include <map>
#include <queue>
#include <utility>
#include "functions.h"



namespace mt {



/**
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
template <
	typename Data,
	typename DistanceFunction = ::mt::functions::euclidean_distance,
	typename SplitFunction = ::mt::functions::split_function<
	        ::mt::functions::random_promotion,
	        ::mt::functions::balanced_partition
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

	/**
	 * @brief A container-like class which can be iterated to fetch the results
	 *        of a nearest-neighbors query.
	 * @details The neighbors are presented in non-decreasing order from the
	 *          @c query_data argument to the mtree::get_nearest() call.
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
	class query {
	public:

		/**
		 * @brief The type of the results for nearest-neighbor queries.
		 */
		class result_item {
		public:
			/** @brief A nearest-neighbor */
			Data data;

			/** @brief The distance from the nearest-neighbor to the query data
			 *         object parameter.
			 */
			double distance;

			/** @brief Default constructor */
			result_item() = default;

			/** @brief Copy constructor */
			result_item(const result_item&) = default;

			/** @brief Move constructor */
			result_item(result_item&&) = default;

			~result_item() = default;

			/** @brief Copy assignment */
			result_item& operator=(const result_item&) = default;

			/** @brief Move assignment */
			result_item& operator=(result_item&& ri) {
				if(this != &ri) {
					data = std::move(ri.data);
					distance = ri.distance;
				}
				return *this;
			}
		};


		typedef result_item value_type;


		query() = delete;

		/**
		 * @brief Copy constructor.
		 */
		query(const query&) = default;

		/**
		 * @brief Move constructor.
		 */
		query(query&&) = default;

		query(const mtree* _mtree, const Data& data, double range, size_t limit)
			: _mtree(_mtree), data(data), range(range), limit(limit)
			{}


		/** @brief Copy assignment. */
		query& operator=(const query&) = default;

		/** @brief Move assignment. */
		query& operator=(query&& q) {
			if(this != &q) {
				this->_mtree = q._mtree;
				this->range = q.range;
				this->limit = q.limit;
				this->data = std::move(q.data);
			}
			return *this;
		}



		/**
		 * @brief The iterator for accessing the results of nearest-neighbor
		 *        queries.
		 */
		class iterator {
		public:
			typedef std::input_iterator_tag iterator_category;
			typedef result_item             value_type;
			typedef signed long int         difference_type;
			typedef result_item*            pointer;
			typedef result_item&            reference;


			iterator() : isEnd(true) {}


			explicit iterator(const query* _query)
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


			/** @brief Copy constructor. */
			iterator(const iterator&) = default;

			/** @brief Move constructor. */
			iterator(iterator&&) = default;

			~iterator() = default;

			/** @brief Copy assignment. */
			iterator& operator=(const iterator&) = default;

			/** @brief Move assignment. */
			iterator& operator=(iterator&& i) {
				if(this != &i) {
					this->_query = i._query;
					this->currentResultItem = std::move(i.currentResultItem);
					this->isEnd = i.isEnd;
					this->pendingQueue = std::move(i.pendingQueue);
					this->nextPendingMinDistance = i.nextPendingMinDistance;
					this->nearestQueue = std::move(i.nearestQueue);
					this->yieldedCount = i.yieldedCount;
				}
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


			/**
			 * @brief Advance the iterator to the next result.
			 */
			//@{
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
			//@}


			/**
			 * @brief Gives access to the current query result.
			 * @details An iterator instance always refers to the same
			 *          result_item instance; only the fields of the result_item
			 *          are updated when the iterator changes.
			 */
			//@{
			const result_item& operator*() const {
				return currentResultItem;
			}

			const result_item* operator->() const {
				return &currentResultItem;
			}
			//@}

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


			const query* _query;
			result_item currentResultItem;
			bool isEnd;
			std::priority_queue<ItemWithDistances<Node>> pendingQueue;
			double nextPendingMinDistance;
			std::priority_queue<ItemWithDistances<Entry>> nearestQueue;
			size_t yieldedCount;
		};


		/**
		 * @brief Begins the execution of the query and returns an interator
		 *        which refers to the first result.
		 */
		iterator begin() const {
			return iterator(this);
		}


		/**
		 * @brief Returns an iterator which informs that there are no more
		 *        results.
		 */
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
	mtree(const mtree&) = delete;

	// ... but moving is ok.
	/** @brief Move constructor. */
	mtree(mtree&& that)
		: root(that.root),
		  maxNodeCapacity(that.maxNodeCapacity),
		  minNodeCapacity(that.minNodeCapacity),
		  distance_function(that.distance_function),
		  split_function(that.split_function)
	{
		that.root = NULL;
	}


	~mtree() {
		delete root;
	}

	// Cannot copy!
	mtree& operator=(const mtree&) = delete;

	// ... but moving is ok.
	/** @brief Move assignment. */
	mtree& operator=(mtree&& that) {
		if(&that != this) {
			std::swap(this->root, that.root);
			this->minNodeCapacity = that.minNodeCapacity;
			this->maxNodeCapacity = that.maxNodeCapacity;
			this->distance_function = std::move(that.distance_function);
			this->split_function = std::move(that.split_function);
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
	 * @param query_data The query data object.
	 * @param range The maximum distance from @c query_data to fetched neighbors.
	 * @return A @c query object.
	 */
	query get_nearest_by_range(const Data& query_data, double range) const {
		return get_nearest(query_data, range, std::numeric_limits<unsigned int>::max());
	}

	/**
	 * @brief Performs a nearest-neighbors query on the M-Tree, constrained by
	 *        the number of neighbors.
	 * @param query_data The query data object.
	 * @param limit The maximum number of neighbors to fetch.
	 * @return A @c query object.
	 */
	query get_nearest_by_limit(const Data& query_data, size_t limit) const {
		return get_nearest(query_data, std::numeric_limits<double>::infinity(), limit);
	}

	/**
	 * @brief Performs a nearest-neighbor query on the M-Tree, constrained by
	 *        distance and/or the number of neighbors.
	 * @param query_data The query data object.
	 * @param range The maximum distance from @c query_data to fetched neighbors.
	 * @param limit The maximum number of neighbors to fetch.
	 * @return A @c query object.
	 */
	query get_nearest(const Data& query_data, double range, size_t limit) const {
		return {this, query_data, range, limit};
	}

	/**
	 * @brief Performs a nearest-neighbor query on the M-Tree, without
	 *        constraints.
	 * @param query_data The query data object.
	 * @return A @c query object.
	 */
	query get_nearest(const Data& query_data) const {
		return {
			this,
			query_data,
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
