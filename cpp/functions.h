#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_


#include <ext/algorithm>
#include <iterator>
#include <set>
#include <utility>
#include <vector>


namespace mtree {
namespace functions {


template <typename T>
double euclideanDistance(const std::vector<T>& data1, const std::vector<T>& data2) {
	typedef typename std::vector<T>::const_iterator Iter;

	double distance = 0;
	for(Iter i1 = data1.begin(), i2 = data2.begin(); i1 != data1.end()  &&  i2 != data2.end(); ++i1, ++i2) {
		double diff = *i1 - *i2;
		distance += diff * diff;
	}
	distance = sqrt(distance);
	return distance;
}



template <typename T, typename CachedDistanceFunction>
std::pair<T, T> randomPromotion(const std::set<T>& dataObjects, CachedDistanceFunction& distanceFunction) {
	std::vector<T> promoted;
	random_sample_n(dataObjects.begin(), dataObjects.end(), inserter(promoted, promoted.begin()), 2);
	assert(promoted.size() == 2);
	return std::pair<T, T>(promoted[0], promoted[1]);
}



template <typename T, typename DistanceFunction>
void balancedPartition(const std::pair<T, T>& promoted, std::set<T>& firstPartition, std::set<T>& secondPartition, DistanceFunction& distanceFunction) {
	struct DataWithDistance {
		T data;
		double distance;

		DataWithDistance(const T& data, double distance) : data(data), distance(distance) {}
		bool operator<(const DataWithDistance& dwi) const {
			return this->distance > dwi.distance;
		}
	};

	// TODO: these need not to be priority-queues!
	std::priority_queue<DataWithDistance> queue1;
	std::priority_queue<DataWithDistance> queue2;

	for(typename std::set<T>::iterator i = firstPartition.begin(); i != firstPartition.end(); ++i) {
		const T& data = *i;

		double distance1 = distanceFunction(data, promoted.first);
		queue1.push(DataWithDistance(data, distance1));

		double distance2 = distanceFunction(data, promoted.first);
		queue2.push(DataWithDistance(data, distance2));
	}
	firstPartition.clear();

	while(!queue1.empty()  ||  !queue2.empty()) {
		while(!queue1.empty()) {
			T data = queue1.top().data;
			queue1.pop();
			if(secondPartition.find(data) == secondPartition.end()) {
				firstPartition.insert(data);
				break;
			}
		}

		while(!queue2.empty()) {
			T data = queue2.top().data;
			queue2.pop();
			if(firstPartition.find(data) == firstPartition.end()) {
				secondPartition.insert(data);
				break;
			}
		}
	}
}


/*

def make_split_function(promotion_function, partition_function):
	"""
	Creates a splitting function.
	The parameters must be callable objects:
		- promotion_function(data_objects, distance_function)
			Must return two objects chosen from the data_objects argument.
		- partition_function(promoted_data1, promoted_data2, data_objects, distance_function)
			Must return a sequence with two iterable objects containing a partition
			of the data_objects. The promoted_data1 and promoted_data2 arguments
			should be used as partitioning criteria and must be contained on the
			corresponding iterable subsets.
	"""
	def split_function(data_objects, distance_function):
		promoted_data1, promoted_data2 = promotion_function(data_objects, distance_function)
		partition1, partition2 = partition_function(promoted_data1, promoted_data2, data_objects, distance_function)

		return promoted_data1, partition1, promoted_data2, partition2

	return split_function




def make_cached_distance_function(distance_function):
	cache = {}

	def cached_distance_function(data1, data2):
		try:
			distance = cache[data1][data2]
		except KeyError:
			distance = distance_function(data1, data2)

			if data1 in cache:
				cache[data1][data2] = distance
			else:
				cache[data1] = { data2 : distance }

			if data2 in cache:
				cache[data2][data1] = distance
			else:
				cache[data2] = { data1 : distance }

		return distance

	cached_distance_function.cache = cache

	return cached_distance_function


*/




} /* namespace mtree::functions */
} /* namespace mtree */


#endif /* FUNCTIONS_H_ */
