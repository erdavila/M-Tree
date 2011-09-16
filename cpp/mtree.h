#ifndef MTREE_H_
#define MTREE_H_


#include <iterator>
#include <limits>
#include <map>
#include <queue>
#include <utility>
#include "functions.h"


#ifdef NDEBUG
# define IF_DEBUG(X)
#else
# define IF_DEBUG(X) X
#endif



namespace mtree {



template <typename Data>
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
		Data data;
	};


	class ResultItem {
	public:
		Data data;
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

		ResultsIterator(ResultsIterator&&) = default;

		ResultsIterator(const MTreeBase* mtree, const Data& queryData, double range, size_t limit)
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

		~ResultsIterator() = default;

		ResultsIterator& operator=(const ResultsIterator&) = default;

		ResultsIterator& operator=(ResultsIterator&& ri) {
			std::swap(mtree                 , ri.mtree);
			std::swap(queryData             , ri.queryData);
			std::swap(range                 , ri.range);
			std::swap(limit                 , ri.limit);
			std::swap(isEnd                 , ri.isEnd);
			std::swap(pendingQueue          , ri.pendingQueue);
			std::swap(nextPendingMinDistance, ri.nextPendingMinDistance);
			std::swap(nearestQueue          , ri.nearestQueue);
			std::swap(yieldedCount          , ri.yieldedCount);
			std::swap(currentResultItem     , ri.currentResultItem);
		}

		bool operator==(const ResultsIterator& ri) const {
			if(this->isEnd  &&  ri.isEnd) {
				return true;
			}

			if(this->isEnd  ||  ri.isEnd) {
				return false;
			}

			return  this->mtree == ri.mtree
			    &&  this->range == ri.range
			    &&  this->limit == ri.limit
			    &&  this->yieldedCount == ri.yieldedCount;
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

		ResultItem& operator*() {
			return currentResultItem;
		}

		const ResultItem& operator*() const {
			return currentResultItem;
		}

		ResultItem* operator->() {
			return &currentResultItem;
		}

		const ResultItem* operator->() const {
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
		Data queryData;
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

		double operator()(const Data& data1, const Data& data2) {
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
		typedef std::map<std::pair<Data, Data>, double> CacheType;

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

	// Cannot copy!
	MTreeBase(const MTreeBase&) = delete;

	// ... but moving is ok.
	MTreeBase(MTreeBase&& that)
		: root(that.root),
		  maxNodeCapacity(that.maxNodeCapacity),
		  minNodeCapacity(that.minNodeCapacity)
	{
		that.root = 0;
	}

	virtual ~MTreeBase() {
		delete root;
	}

	// Cannot copy!
	MTreeBase& operator=(const MTreeBase&) = delete;

	// ... but moving is ok.
	MTreeBase& operator=(MTreeBase&& that) {
		std::swap(this->root, that.root);
		this->minNodeCapacity = that.minNodeCapacity;
		this->maxNodeCapacity = that.maxNodeCapacity;
		return *this;
	}

	void add(const Data& data) {
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


	void remove(const Data& data) throw (DataNotFound) {
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


	ResultsIterator getNearestByRange(const Data& queryData, double range) const {
		return getNearest(queryData, range, std::numeric_limits<unsigned int>::max());
	}

	ResultsIterator getNearestByLimit(const Data& queryData, size_t limit) const {
		return getNearest(queryData, std::numeric_limits<double>::infinity(), limit);
	}

	ResultsIterator getNearest(const Data& queryData, double range, size_t limit) const {
		return {this, queryData, range, limit};
	}

	ResultsIterator resultsEnd() const {
		return {};
	}

protected:
	virtual double distanceFunction(const Data&, const Data&) const = 0;

	typedef std::pair<Data, Data> PromotedPair;
	typedef std::set<Data> DataSet;
	typedef std::set<Data> Partition;


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
		IF_DEBUG(
			if(root != NULL) {
					root->_check(this);
			}
		)
	}

private:

	size_t minNodeCapacity;
	size_t maxNodeCapacity;
	Node* root;


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

		void addData(const Data& data, double distance, const MTreeBase* mtree) throw(SplitNodeReplacement) {
			doAddData(data, distance, mtree);
			checkMaxCapacity(mtree);
		}

		IF_DEBUG(size_t _check(const MTreeBase* mtree) const {
			IndexItem::_check(mtree);
			_checkMinCapacity(mtree);
			_checkMaxCapacity(mtree);

			bool   childHeightKnown = false;
			size_t childHeight;
			for(typename ChildrenMap::const_iterator i = children.begin(); i != children.end(); ++i) {
				IF_DEBUG(const Data& data = i->first);
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
		})

		typedef std::map<Data, IndexItem*> ChildrenMap;

		ChildrenMap children;

	protected:
		Node(const Data& data) : IndexItem(data) { }

		Node() : IndexItem(*((Data*)(0))) { assert(!"THIS SHOULD NEVER BE CALLED"); };

		Node(const Node&) = delete;
		Node(Node&&) = delete;
		Node& operator=(const Node&) = delete;
		Node& operator=(Node&&) = delete;

		virtual void doAddData(const Data& data, double distance, const MTreeBase* mtree) = 0;

		virtual void doRemoveData(const Data& data, double distance, const MTreeBase* mtree) throw (DataNotFound) = 0;

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
		virtual void addChild(IndexItem* child, double distance, const MTreeBase* mtree) = 0;

		virtual void removeData(const Data& data, double distance, const MTreeBase* mtree) throw (RootNodeReplacement, NodeUnderCapacity, DataNotFound) {
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
		IF_DEBUG(void _checkChildMetrics(IndexItem* child, const MTreeBase* mtree) const {
			double dist = mtree->distanceFunction(child->data, this->data);
			assert(child->distanceToParent == dist);

			/* TODO: investigate why the following line
			 * 		assert(child->distanceToParent + child->radius <= this->radius);
			 * is not the same as the code below:
			 */
			double sum = child->distanceToParent + child->radius;
			assert(sum <= this->radius);
		})
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
		void doAddData(const Data& data, double distance, const MTreeBase* mtree) {
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

		Node* newSplitNodeReplacement(const Data& data) const {
			return new LeafNode(data);
		}

		void doRemoveData(const Data& data, double distance, const MTreeBase* mtree) throw (DataNotFound) {
			if(this->children.erase(data) == 0) {
				throw DataNotFound{data};
			}
		}


		void _checkChildClass(IndexItem* child) const {
			assert(dynamic_cast<Entry*>(child) != NULL);
		}
	};


	class NonLeafNodeTrait : public virtual Node {
		void doAddData(const Data& data, double distance, const MTreeBase* mtree) {
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
				IF_DEBUG(size_t _ =)
					this->children.erase(child->data);
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
						IF_DEBUG(size_t _ =)
							this->children.erase(existingChild->data);
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


		Node* newSplitNodeReplacement(const Data& data) const {
			return new InternalNode(data);
		}


		void doRemoveData(const Data& data, double distance, const MTreeBase* mtree) throw (DataNotFound) {
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

				IF_DEBUG(size_t _ =)
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

		void removeData(const Data& data, double distance, const MTreeBase* mtree) throw (RootNodeReplacement, DataNotFound) {
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
		RootNode(const Data& data) : Node(data) {}

	private:
		void removeData(const Data& data, double distance, const MTreeBase* mtree) throw (RootNodeReplacement, NodeUnderCapacity, DataNotFound) {
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


#undef IF_DEBUG


#endif /* MTREE_H_ */
