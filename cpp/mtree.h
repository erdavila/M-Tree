#ifndef MTREE_H_
#define MTREE_H_


#include <iterator>
#include <limits>
#include <map>
#include <queue>
#include <utility>
#include "functions.h"


namespace mtree {



template <typename T>
class MTreeBase {
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


public:

	// Public exception
	class DataNotFound {
	public:
		T data;
	};


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
		typedef std::input_iterator_tag iterator_category;
		typedef ResultItem              value_type;
		typedef signed long int         difference_type;
		typedef ResultItem*             pointer;
		typedef ResultItem&             reference;


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
				return (this->minDistance > that.minDistance);
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



	class CachedDistanceFunction {
	public:
		CachedDistanceFunction(const MTreeBase* mtree) : mtree(mtree) {}

		double operator()(const T& data1, const T& data2) {
			typename CacheType::iterator i = cache.find(make_pair(data1, data2));
			if(i != cache.end()) {
				return i->second;
			}

			i = cache.find(make_pair(data2, data1));
			if(i != cache.end()) {
				return i->second;
			}

			// Not found in cache
			double distance = mtree->distanceFunction(data1, data2);

			// Store in cache
			cache.insert(make_pair(make_pair(data1, data2), distance));
			cache.insert(make_pair(make_pair(data2, data1), distance));

			return distance;
		}

	private:
		typedef std::map<std::pair<T, T>, double> CacheType;

		const MTreeBase* mtree;
		CacheType cache;
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
			double distance = distanceFunction(data, root->data);
			try {
				root->addData(data, distance, this);
			} catch(SplitNodeReplacement& e) {
				Node* newRoot = new RootNode(root->data);
				delete root;
				root = newRoot;
				for(int i = 0; i < SplitNodeReplacement::NUM_NODES; ++i) {
					Node* newNode = e.newNodes[i];
					double distance = distanceFunction(root->data, newNode->data);
					root->addChild(newNode, distance, this);
				}
			}
		}
	}


	void remove(const T& data) throw (DataNotFound) {
		if(root == NULL) {
			throw DataNotFound{data};
		}

		double distanceToRoot = distanceFunction(data, root->data);
		try {
			root->removeData(data, distanceToRoot, this);
		} catch(RootNodeReplacement e) {
			delete root;
			root = e.newRoot;
		}
	}


	ResultsIterator getNearestByRange(const T& queryData, double range) const {
		return getNearest(queryData, range, std::numeric_limits<unsigned int>::max());
	}

	ResultsIterator getNearestByLimit(const T& queryData, size_t limit) const {
		return getNearest(queryData, std::numeric_limits<double>::infinity(), limit);
	}

	ResultsIterator getNearest(const T& queryData, double range, size_t limit) const {
		return {this, queryData, range, limit};
	}

	ResultsIterator resultsEnd() const {
		return {};
	}

protected:
	virtual double distanceFunction(const T&, const T&) const = 0;

	typedef std::pair<T, T> PromotedPair;
	typedef std::set<T> DataSet;
	typedef std::set<T> Partition;


	virtual PromotedPair splitFunction(Partition& firstPartition, Partition& secondPartition, CachedDistanceFunction& cachedDistanceFunction) const {
		PromotedPair promoted = promotionFunction(firstPartition, cachedDistanceFunction);
		partitionFunction(promoted, firstPartition, secondPartition, cachedDistanceFunction);
		return promoted;
	}

	virtual PromotedPair promotionFunction(const DataSet& dataSet, CachedDistanceFunction& cachedDistanceFunction) const {
		return mtree::functions::randomPromotion(dataSet, cachedDistanceFunction);
	}

	virtual void partitionFunction(const PromotedPair& promoted, Partition& firstPartition, Partition& secondPartition, CachedDistanceFunction& cachedDistanceFunction) const {
		return mtree::functions::balancedPartition(promoted, firstPartition, secondPartition, cachedDistanceFunction);
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
			for(typename ChildrenMap::iterator i = children.begin(); i != children.end(); ++i) {
				IndexItem* child = i->second;
				delete child;
			}
		}

		void addData(const T& data, double distance, const MTreeBase* mtree) throw(SplitNodeReplacement) {
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

		virtual void doAddData(const T& data, double distance, const MTreeBase* mtree) = 0;

		virtual void doRemoveData(const T& data, double distance, const MTreeBase* mtree) throw (DataNotFound) = 0;

	public:
		void checkMaxCapacity(const MTreeBase* mtree) throw (SplitNodeReplacement) {
			if(children.size() > mtree->maxNodeCapacity) {
				Partition firstPartition;
				for(typename ChildrenMap::iterator i = children.begin(); i != children.end(); ++i) {
					firstPartition.insert(i->first);
				}

				CachedDistanceFunction cachedDistanceFunction(mtree);

				Partition secondPartition;
				PromotedPair promoted = mtree->splitFunction(firstPartition, secondPartition, cachedDistanceFunction);

				Node* newNodes[2];
				for(int i = 0; i < 2; ++i) {
					T& promotedData      = (i == 0) ? promoted.first : promoted.second;
					Partition& partition = (i == 0) ? firstPartition : secondPartition;

					Node* newNode = newSplitNodeReplacement(promotedData);
					for(typename Partition::iterator j = partition.begin(); j != partition.end(); ++j) {
						const T& data = *j;
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
		virtual Node* newSplitNodeReplacement(const T&) const = 0;

	public:
		virtual void addChild(IndexItem* child, double distance, const MTreeBase* mtree) = 0;

		virtual void removeData(const T& data, double distance, const MTreeBase* mtree) throw (RootNodeReplacement, NodeUnderCapacity, DataNotFound) {
			doRemoveData(data, distance, mtree);
			if(children.size() < getMinCapacity(mtree)) {
				throw NodeUnderCapacity();
			}
		}

		virtual size_t getMinCapacity(const MTreeBase* mtree) const = 0;

	protected:
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

			/* TODO: investigate why the following line
			 * 		assert(child->distanceToParent + child->radius <= this->radius);
			 * is not the same as the code below:
			 */
			double sum = child->distanceToParent + child->radius;
			assert(sum <= this->radius);
		}
	};


	class RootNodeTrait : public virtual Node {
		void _checkDistanceToParent() const {
			assert(this->distanceToParent == -1);
		}
	};


	class NonRootNodeTrait : public virtual Node {
		size_t getMinCapacity(const MTreeBase* mtree) const {
			return mtree->minNodeCapacity;
		}

		void _checkMinCapacity(const MTreeBase* mtree) const {
			assert(this->children.size() >= mtree->minNodeCapacity);
		}
	};


	class LeafNodeTrait : public virtual Node {
		void doAddData(const T& data, double distance, const MTreeBase* mtree) {
			Entry* entry = new Entry(data);
			assert(this->children.find(data) == this->children.end());
			this->children[data] = entry;
			assert(this->children.find(data) != this->children.end());
			updateMetrics(entry, distance);
		}

		void addChild(IndexItem* child, double distance, const MTreeBase* mtree) {
			assert(this->children.find(child->data) == this->children.end());
			this->children[child->data] = child;
			assert(this->children.find(child->data) != this->children.end());
			updateMetrics(child, distance);
		}

		Node* newSplitNodeReplacement(const T& data) const {
			return new LeafNode(data);
		}

		void doRemoveData(const T& data, double distance, const MTreeBase* mtree) throw (DataNotFound) {
			if(this->children.erase(data) == 0) {
				throw DataNotFound{data};
			}
		}


		void _checkChildClass(IndexItem* child) const {
			assert(dynamic_cast<Entry*>(child) != NULL);
		}
	};


	class NonLeafNodeTrait : public virtual Node {
		void doAddData(const T& data, double distance, const MTreeBase* mtree) {
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
				double distance = mtree->distanceFunction(child->data, data);
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
			} catch(SplitNodeReplacement e) {
				// Replace current child with new nodes
				size_t _ = this->children.erase(child->data);
				assert(_ == 1);
				delete child;

				for(int i = 0; i < e.NUM_NODES; ++i) {
					Node* newChild = e.newNodes[i];
					double distance = mtree->distanceFunction(this->data, newChild->data);
					addChild(newChild, distance, mtree);
				}
			}
		}


		void addChild(IndexItem* newChild_, double distance, const MTreeBase* mtree) {
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
					} catch(SplitNodeReplacement e) {
						size_t _ = this->children.erase(existingChild->data);
						assert(_ == 1);
						delete existingChild;

						for(int i = 0; i < e.NUM_NODES; ++i) {
							Node* newNode = e.newNodes[i];
							double distance = mtree->distanceFunction(this->data, newNode->data);
							newChildren.push_back(ChildWithDistance{newNode, distance});
						}
					}
				}
			}
		}


		Node* newSplitNodeReplacement(const T& data) const {
			return new InternalNode(data);
		}


		void doRemoveData(const T& data, double distance, const MTreeBase* mtree) throw (DataNotFound) {
			for(typename Node::ChildrenMap::iterator i = this->children.begin(); i != this->children.end(); ++i) {
				Node* child = dynamic_cast<Node*>(i->second);
				assert(child != NULL);
				if(abs(distance - child->distanceToParent) <= child->radius) {
					double distanceToChild = mtree->distanceFunction(data, child->data);
					if(distanceToChild <= child->radius) {
						try {
							child->removeData(data, distanceToChild, mtree);
							updateRadius(child);
							return;
						} catch(DataNotFound) {
							// If DataNotFound was thrown, then the data was not found in the child
						} catch(NodeUnderCapacity) {
							Node* expandedChild = balanceChildren(child, mtree);
							updateRadius(expandedChild);
							return;
						}
					}
				}
			}

			throw DataNotFound{data};
		}


		Node* balanceChildren(Node* theChild, const MTreeBase* mtree) {
			// Tries to find anotherChild which can donate a grand-child to theChild.

			Node* nearestDonor = NULL;
			double distanceNearestDonor = std::numeric_limits<double>::infinity();

			Node* nearestMergeCandidate = NULL;
			double distanceNearestMergeCandidate = std::numeric_limits<double>::infinity();

			for(typename Node::ChildrenMap::iterator i = this->children.begin(); i != this->children.end(); ++i) {
				Node* anotherChild = dynamic_cast<Node*>(i->second);
				assert(anotherChild != NULL);
				if(anotherChild == theChild) continue;

				double distance = mtree->distanceFunction(theChild->data, anotherChild->data);
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
					double distance = mtree->distanceFunction(grandchild->data, nearestMergeCandidate->data);
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
					double distance = mtree->distanceFunction(grandchild->data, theChild->data);
					if(distance < nearestGrandchildDistance) {
						nearestGrandchildDistance = distance;
						nearestGrandchild = grandchild;
					}
				}

				size_t _ = nearestDonor->children.erase(nearestGrandchild->data);
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
		RootLeafNode(const T& data) : Node(data) { }

		void removeData(const T& data, double distance, const MTreeBase* mtree) throw (RootNodeReplacement, DataNotFound) {
			try {
				Node::removeData(data, distance, mtree);
			} catch (NodeUnderCapacity) {
				assert(this->children.empty());
				throw RootNodeReplacement{NULL};
			}
		}

		size_t getMinCapacity(const MTreeBase* mtree) const {
			return 1;
		}

		void _checkMinCapacity(const MTreeBase* mtree) const {
			assert(this->children.size() >= 1);
		}
	};

	class RootNode : public RootNodeTrait, public NonLeafNodeTrait {
	public:
		RootNode(const T& data) : Node(data) {}

	private:
		void removeData(const T& data, double distance, const MTreeBase* mtree) throw (RootNodeReplacement, NodeUnderCapacity, DataNotFound) {
			try {
				Node::removeData(data, distance, mtree);
			} catch(NodeUnderCapacity) {
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
					double distance = mtree->distanceFunction(newRoot->data, grandchild->data);
					newRoot->addChild(grandchild, distance, mtree);
				}
				theChild->children.clear();

				throw RootNodeReplacement{newRoot};
			}
		}


		size_t getMinCapacity(const MTreeBase* mtree) const {
			return 2;
		}

		void _checkMinCapacity(const MTreeBase* mtree) const {
			assert(this->children.size() >= 2);
		}
	};


	class InternalNode : public NonRootNodeTrait, public NonLeafNodeTrait {
	public:
		InternalNode(const T& data) : Node(data) { }
	};


	class LeafNode : public NonRootNodeTrait, public LeafNodeTrait {
	public:
		LeafNode(const T& data) : Node(data) { }
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


class _NonRootNodeTrait(_Node)


class _LeafNodeTrait(_Node):


class _NonLeafNodeTrait(_Node)


class _RootNode(_RootNodeTrait, _NonLeafNodeTrait)


class _InternalNode(_NonRootNodeTrait, _NonLeafNodeTrait)


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


	def remove(self, data)


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
