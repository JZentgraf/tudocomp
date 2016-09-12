#ifndef _INCLUDED_DS_PHI_ARRAY_HPP
#define _INCLUDED_DS_PHI_ARRAY_HPP

#include <tudocomp/util.h>
#include "forward.hh"


namespace tudocomp {

using io::InputView;

template<class T>
class PhiArray {

public:
    typedef sdsl::int_vector<> iv_t;

private:
    iv_t m_phi;

public:
    inline PhiArray() {
    }

    inline iv_t& data() {
        return m_phi;
    }

    inline const iv_t& data() const {
        return m_phi;
    }

    inline iv_t::value_type operator[](iv_t::size_type i) const {
        return m_phi[i];
    }

    inline iv_t::size_type size() const {
        return m_phi.size();
    }

    inline void construct(T& t);
};

}//ns
#include <tudocomp/ds/TextDS.hpp>
#include <tudocomp/ds/SuffixArray.hpp>
namespace tudocomp {

template<class T>
inline void PhiArray<T>::construct(T& t) {
	auto& sa = t.require_sa();
	auto n = sa.size();

	m_phi = iv_t(n, 0, bits_for(n));
	for(size_t i = 0, prev = 0; i < n; i++) {
		auto s = sa[i];
		m_phi[s] = prev;
		prev = s;
	}
}

}

#endif
