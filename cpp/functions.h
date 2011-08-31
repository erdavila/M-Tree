#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_


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


} /* namespace mtree::functions */
} /* namespace mtree */


#endif /* FUNCTIONS_H_ */
