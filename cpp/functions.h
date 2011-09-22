#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_


#include <ext/algorithm>
#include <iterator>
#include <set>
#include <utility>
#include <vector>


namespace mt {
namespace functions {


/*
 * Calculates the euclidean distance between two data objects representing
 * coordinates.
 * Assumes that the data objects are same-sized sequences of numbers.
 * See http://en.wikipedia.org/wiki/Euclidean_distance
 */
struct euclidean_distance {
	template <typename Container>
	double operator()(const Container& data1, const Container& data2) const {
		double distance = 0;
		for(auto i1 = data1.begin(), i2 = data2.begin(); i1 != data1.end()  &&  i2 != data2.end(); ++i1, ++i2) {
			double diff = *i1 - *i2;
			distance += diff * diff;
		}
		distance = sqrt(distance);
		return distance;
	}
};



struct random_promotion {
	template <typename Data, typename CachedDistanceFunction>
	std::pair<Data, Data> operator()(const std::set<Data>& dataObjects, CachedDistanceFunction& distanceFunction) const {
		std::vector<Data> promoted;
		random_sample_n(dataObjects.begin(), dataObjects.end(), inserter(promoted, promoted.begin()), 2);
		assert(promoted.size() == 2);
		return {promoted[0], promoted[1]};
	}
};



struct balanced_partition {
	template <typename Data, typename DistanceFunction>
	void operator()(const std::pair<Data, Data>& promoted,
	                std::set<Data>& firstPartition,
	                std::set<Data>& secondPartition,
	                DistanceFunction& distanceFunction
	            ) const
	{
		std::vector<Data> queue1(firstPartition.begin(), firstPartition.end());
		// Sort by distance to the first promoted data
		std::sort(queue1.begin(), queue1.end(),
			[&](const Data& data1, const Data& data2) {
				double distance1 = distanceFunction(data1, promoted.first);
				double distance2 = distanceFunction(data2, promoted.first);
				return distance1 < distance2;
			}
		);

		std::vector<Data> queue2(firstPartition.begin(), firstPartition.end());
		// Sort by distance to the second promoted data
		std::sort(queue2.begin(), queue2.end(),
			[&](const Data& data1, const Data& data2) {
				double distance1 = distanceFunction(data1, promoted.second);
				double distance2 = distanceFunction(data2, promoted.second);
				return distance1 < distance2;
			}
		);

		firstPartition.clear();

		typename std::vector<Data>::iterator i1 = queue1.begin();
		typename std::vector<Data>::iterator i2 = queue2.begin();

		while(i1 != queue1.end()  ||  i2 != queue2.end()) {
			while(i1 != queue1.end()) {
				Data& data = *i1;
				++i1;
				if(secondPartition.find(data) == secondPartition.end()) {
					firstPartition.insert(data);
					break;
				}
			}

			while(i2 != queue2.end()) {
				Data& data = *i2;
				++i2;
				if(firstPartition.find(data) == firstPartition.end()) {
					secondPartition.insert(data);
					break;
				}
			}
		}
	}
};



template <typename PromotionFunction, typename PartitionFunction>
struct split_function {
	typedef PromotionFunction promotion_function_type;
	typedef PartitionFunction partition_function_type;

	PromotionFunction promotion_function;
	PartitionFunction partition_function;

	explicit split_function(
			PromotionFunction promotion_function = PromotionFunction(),
			PartitionFunction partition_function = PartitionFunction()
		)
	: promotion_function(promotion_function),
	  partition_function(partition_function)
	{}


	template <typename Data, typename DistanceFunction>
	std::pair<Data, Data> operator()(
				std::set<Data>& firstPartition,
				std::set<Data>& secondPartition,
				DistanceFunction& distanceFunction
			) const
	{
		std::pair<Data, Data> promoted = promotion_function(firstPartition, distanceFunction);
		partition_function(promoted, firstPartition, secondPartition, distanceFunction);
		return promoted;
	}
};



template <typename Data, typename DistanceFunction>
class cached_distance_function {
public:
	explicit cached_distance_function(const DistanceFunction& distance_function)
		: distance_function(distance_function)
		{}

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
		double distance = distance_function(data1, data2);

		// Store in cache
		cache.insert(make_pair(make_pair(data1, data2), distance));
		cache.insert(make_pair(make_pair(data2, data1), distance));

		return distance;
	}

private:
	typedef std::map<std::pair<Data, Data>, double> CacheType;

	const DistanceFunction& distance_function;
	CacheType cache;
};



} /* namespace functions */
} /* namespace mtree */


#endif /* FUNCTIONS_H_ */
