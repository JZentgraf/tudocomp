#pragma once

#include <tudocomp/ds/IntVector.hpp>

namespace tdc {

class ArrayDS : public Algorithm {
protected:
    using iv_t = DynamicIntVector;
    std::unique_ptr<iv_t> m_data;

public:
    using data_type = iv_t;
    using Algorithm::Algorithm;

    inline std::unique_ptr<iv_t> relinquish() {
        return std::move(m_data);
    }

    inline len_t operator[](size_t i) const {
        DCHECK(m_data);
        return (*m_data)[i];
    }

    inline size_t size() const {
        DCHECK(m_data);
        return m_data->size();
    }
};

} //ns
