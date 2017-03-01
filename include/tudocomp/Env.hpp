#pragma once

#include <tudocomp/pre_header/Env.hpp>

namespace tdc {

inline AlgorithmValue& EnvRoot::algo_value() {
    return *m_algo_value;
}

inline Stat& EnvRoot::stat_current() {
    return m_stat_stack.empty() ? m_stat_root : m_stat_stack.top();
}

inline void EnvRoot::begin_stat_phase(const std::string& name) {
    IF_STATS({
        DVLOG(1) << "begin phase \"" << name << "\"";

        m_stat_stack.push(Stat(name));
        Stat& stat = m_stat_stack.top();
        stat.begin();
    })
}

inline void EnvRoot::end_stat_phase() {
    IF_STATS({
        DCHECK(!m_stat_stack.empty());

        Stat& stat_ref = m_stat_stack.top();
        stat_ref.end();

        DVLOG(1) << "end phase \"" << stat_ref.title() << "\"";

        Stat stat = stat_ref; //copy
        m_stat_stack.pop();

        if(!m_stat_stack.empty()) {
            Stat& parent = m_stat_stack.top();
            parent.add_sub(stat);
        } else {
            m_stat_root = stat;
        }
    })
}

inline StatGuard EnvRoot::stat_phase(const std::string& name) {
    begin_stat_phase(name);
    return StatGuard(*this);
}

inline Stat& EnvRoot::finish_stats() {
    IF_STATS({
        while(!m_stat_stack.empty()) {
            end_stat_phase();
        }
    })

    return m_stat_root;
}

inline void EnvRoot::restart_stats(const std::string& root_name) {
    finish_stats();
    begin_stat_phase(root_name);
}

template<class T>
inline void EnvRoot::log_stat(const std::string& name, const T& value) {
    IF_STATS({
        DVLOG(1) << "stat: " << name << " = " << value;
        stat_current().add_stat(name, value);
    })
}

inline Env::Env(Env&& other):
    m_root(std::move(other.m_root)),
    m_node(other.m_node) {}

inline Env::Env(std::shared_ptr<EnvRoot> root,
                const AlgorithmValue& node):
    m_root(root),
    m_node(node) {}

inline Env::~Env() = default;

inline const AlgorithmValue& Env::algo() const {
    return m_node;
}

inline std::shared_ptr<EnvRoot>& Env::root() {
    return m_root;
}

inline void Env::error(const std::string& msg) {
    throw std::runtime_error(msg);
}

inline Env Env::env_for_option(const std::string& option) {
    CHECK(algo().arguments().count(option) > 0)
        << "env_for_option(): There is no option '"
        << option
        << "'";
    auto& a = algo().arguments().at(option).as_algorithm();

    return Env(m_root, a);
}

inline const OptionValue& Env::option(const std::string& option) const {
    return algo().arguments().at(option);
}

inline void Env::begin_stat_phase(const std::string& name) {
    IF_STATS(m_root->begin_stat_phase(name)); //delegate
}

inline void Env::end_stat_phase() {
    IF_STATS(m_root->end_stat_phase()); //delegate
}

inline StatGuard Env::stat_phase(const std::string& name) {
    begin_stat_phase(name);
    return StatGuard(*m_root);
}

inline Stat& Env::finish_stats() {
    return m_root->finish_stats(); //delegate
}

inline void Env::restart_stats(const std::string& root_name) {
    m_root->restart_stats(root_name); //delegate
}

template<class T>
inline void Env::log_stat(const std::string& name, const T& value) {
    IF_STATS(m_root->log_stat(name, value));
}

}

