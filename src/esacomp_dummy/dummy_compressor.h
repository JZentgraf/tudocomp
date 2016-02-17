#ifndef DUMMY_COMPRESSOR_H
#define DUMMY_COMPRESSOR_H

#include "tudocomp.h"
#include "esacomp/esacomp_rule_compressor.h"
#include "esacomp/rule.h"
#include "esacomp/rules.h"

// Put every C++ code in this project into a common namespace
// in order to not pollute the global one
namespace esacomp {

using namespace tudocomp;

class DummyCompressor: public EsacompCompressStrategy {
public:
    using EsacompCompressStrategy::EsacompCompressStrategy;

    virtual Rules compress(Input& input, size_t threshold) final override;
};

}

#endif