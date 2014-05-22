#ifndef _FINDPOWERSET_HDR_
#define _FINDPOWERSET_HDR_

#include <vector>
#include <set>

template<typename T> void FindPowerSet(
	const typename std::set<T>& S,
	const typename std::set< T>::const_iterator& SIt,
	typename std::set<T>& K,
	typename std::vector< std::vector< std::set<T> > >& PS,
	unsigned int depth,
	unsigned int maxDepth
) {
	if (depth > maxDepth) {
		return;
	}

	// make a copy of K
	PS[depth].push_back(K);

	for (typename std::set<T>::iterator it = SIt; it != S.end(); ++it) {
		typename std::set<T>::iterator nit = it;

		K.insert(*it);
		FindPowerSet(S, ++nit, K, PS, depth + 1, maxDepth);
		K.erase(*it);
	}
}

template<typename T> void FindPowerSet(const std::set<T>& S, std::vector< std::vector< std::set<T> > >& PS, unsigned int maxDepth) {
	if (S.empty()) {
		return;
	}

	std::set<T> K;

	// the power-set of S contains all
	// subsets of size 1, 2, ..., |S|
	// there are (|S| over K) subsets
	// of size K
	PS.resize(S.size() + 1);

	FindPowerSet(S, S.begin(), K, PS, 0, maxDepth);
}

#endif
