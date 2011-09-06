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
	std::vector<T> queue1(firstPartition.begin(), firstPartition.end());
	// Sort by distance to the first promoted data
	std::sort(queue1.begin(), queue1.end(),
		[&](const T& data1, const T& data2) {
			double distance1 = distanceFunction(data1, promoted.first);
			double distance2 = distanceFunction(data2, promoted.first);
			return distance1 < distance2;
		}
	);

	std::vector<T> queue2(firstPartition.begin(), firstPartition.end());
	// Sort by distance to the second promoted data
	std::sort(queue2.begin(), queue2.end(),
		[&](const T& data1, const T& data2) {
			double distance1 = distanceFunction(data1, promoted.second);
			double distance2 = distanceFunction(data2, promoted.second);
			return distance1 < distance2;
		}
	);

	firstPartition.clear();

	typename std::vector<T>::iterator i1 = queue1.begin();
	typename std::vector<T>::iterator i2 = queue2.begin();

	while(i1 != queue1.end()  ||  i2 != queue2.end()) {
		while(i1 != queue1.end()) {
			T& data = *i1;
			++i1;
			if(secondPartition.find(data) == secondPartition.end()) {
				firstPartition.insert(data);
				break;
			}
		}

		while(i2 != queue2.end()) {
			T& data = *i2;
			++i2;
			if(firstPartition.find(data) == firstPartition.end()) {
				secondPartition.insert(data);
				break;
			}
		}
	}
}


} /* namespace mtree::functions */
} /* namespace mtree */


#endif /* FUNCTIONS_H_ */
