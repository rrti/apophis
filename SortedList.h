#ifndef _SORTEDLIST_HDR_
#define _SORTEDLIST_HDR_

#include <list>

// maintains a sorted std::list via insertion-sort
// (elements are stored in increasing order by key)
//
// note: too slow for actual use, O(N) insertion time
template<typename K, typename V> class SortedList {
public:
	typedef typename std::pair<K, V> KeyValPair;

	SortedList(unsigned int _maxSize, K _maxKey): maxSize(_maxSize), maxKey(_maxKey) {}
	~SortedList() { list.clear(); }

	typename std::list< KeyValPair >::const_iterator begin() const { return list.begin(); }
	typename std::list< KeyValPair >::const_iterator end() const { return list.end(); }
	typename std::list< KeyValPair >::iterator begin() { return list.begin(); }
	typename std::list< KeyValPair >::iterator end() { return list.end(); }
	KeyValPair& front() { return list.front(); }
	KeyValPair& back() { return list.back(); }

	unsigned int size() const { return list.size(); }
	bool empty() const { return list.empty(); }

	void push_back(K k, V v) {
		if (k > maxKey) {
			return;
		}

		typename std::list< KeyValPair >::iterator it;

		// find the right insertion position
		for (it = list.begin(); it != list.end(); it++) {
			if (k < it->first) {
				break;
			}
		}

		list.insert(it, std::pair<K, V>(k, v));

		// keep the list from exceeding its maximum size
		if (list.size() > maxSize) {
			list.pop_back();
		}
	}

	struct iterator {
	public:
		iterator& operator = (const typename std::list<KeyValPair>::iterator& _it) {
			it = _it; return *this;
		}

		iterator& operator ++ () { it++; return *this; } // prefix
		iterator operator ++ (int) { iterator _it = *this; ++(*this); return _it; } // postfix

		bool operator != (const typename std::list<KeyValPair>::iterator& _it) {
			return (it != _it);
		}

		K first() const { return it->first; }
		V second() const { return it->second; }

	private:
		typename std::list<KeyValPair>::iterator it;
	};

private:
	typename std::list< KeyValPair > list;

	unsigned int maxSize;

	K maxKey;
	V maxVal;
};

#endif
