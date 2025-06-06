/*++
Copyright (c) 2006 Microsoft Corporation

Module Name:

    uint_set.h

Abstract:

    Sets of unsigned integers.
    
Author:

    Leonardo de Moura (leonardo) 2006-12-07.

Revision History:

--*/
#pragma once

#include "util/util.h"
#include "util/vector.h"


class uint_set : unsigned_vector {
    
public:
    
    typedef unsigned data;

    void swap(uint_set & other) noexcept {
        unsigned_vector::swap(other);
    }

    // return the maximum value that can be stored in the set. 
    unsigned get_max_elem() const {
        return 32 * size();
    }

    void reset() {
        unsigned_vector::reset();
    }

    bool empty() const {
        for (unsigned i = 0; i < size(); i++) {
            if ((*this)[i] != 0) {
                return false;
            }
        }
        return true;
    }

    void insert(unsigned val) {
        unsigned idx = val >> 5;
        if (idx >= size()) {
            resize(idx+1);
        }
        (*this)[idx] |= 1 << (val & 31);
    }

    void remove(unsigned val) {
        unsigned idx = val >> 5;
        if (idx < size()) {
            (*this)[val >> 5] &= ~(1 << (val & 31));
        }
    }

    bool contains(unsigned val) const {
        unsigned idx = val >> 5;        
        return idx < size() && ((*this)[idx] & (1 << (val & 31))) != 0;
    }

    unsigned num_elems() const {
        unsigned r = 0;
        for (unsigned i = 0; i < size(); i++) {
            r += get_num_1bits((*this)[i]);
        }
        return r;
    }

    // Insert in the this object the elements in the set source.
    uint_set & operator |=(const uint_set & source) {
        auto source_size = source.size();
        if (source_size > size()) {
            resize(source_size + 1);
        }
        for (unsigned i = 0; i < source_size; i++) {
            (*this)[i] |= source[i];
        }
        return *this;
    }

    uint_set& operator &=(const uint_set& source) {
        auto source_size = source.size();
        if (source_size < size()) {
            resize(source_size);
        }
        for (unsigned i = 0; i < size(); i++) {
            (*this)[i] &= source[i];
        }
        return *this;
    }

    bool operator==(const uint_set & source) const {
        auto min_size = size();
        if (source.size() < min_size) {
            min_size = source.size();
        }
        for (unsigned i = 0; i < min_size; i++) {
            if ((*this)[i] != source[i]) {
                return false;
            }
        }
        for (auto i = min_size; i < size(); ++i) {
            if ((*this)[i]) {
                return false;
            }
        }
        for (auto i = min_size; i < source.size(); ++i) {
            if (source[i]) {
                return false;
            }
        }

        return true;
    }

    bool operator!=(const uint_set & source) const {
        return !operator==(source);
    }

    // Return true if the this set is a subset of source.
    bool subset_of(const uint_set & source) const {
        auto min_size = size();
        if (source.size() < min_size) {
            min_size = source.size();
        }
        for (unsigned i = 0; i < min_size; i++) {
            if (((*this)[i] & ~source[i]) != 0) {
                return false;
            }
        }
        for (auto i = min_size; i < size(); ++i) {
            if ((*this)[i]) {
                return false;
            }
        }
        return true;
    }

    class iterator {
        uint_set const* m_set;
        unsigned  m_index;
        unsigned  m_last;

        bool invariant() const { return m_index <= m_last; }

        bool at_end() const { return m_index == m_last; }

        void scan_idx() {
            SASSERT(invariant());
            while (!at_end() && !m_set->contains(m_index) && 0 != (m_index & 31)) {
                ++m_index;
            }
            SASSERT(invariant());
        }
        void scan_word() {
            SASSERT((m_index & 31) == 0);
            SASSERT(invariant());
            unsigned idx = m_index >> 5;
            while (!at_end() && !(*m_set)[idx]) {
                ++idx;
                m_index += 32;
            }
            SASSERT(invariant());
        }
        bool contains() const { return m_set->contains(m_index); }
        void scan() {     
            scan_idx();
            if (contains() || at_end()) {
                return;
            }
            scan_word();
            if (!at_end() && !contains()) {
                ++m_index;
            }
            scan_idx();
            SASSERT(invariant());
        }
    public:
        iterator(uint_set const& s, bool at_end): 
            m_set(&s), m_index(at_end?s.get_max_elem():0), m_last(s.get_max_elem()) {
            scan();
            SASSERT(invariant());
        }
        unsigned operator*() const { return m_index; }
        bool operator!=(iterator const& it) const { return m_index != it.m_index; }
        iterator & operator++() { ++m_index; scan(); return *this; }
        iterator operator++(int) { iterator tmp = *this; ++*this; return tmp; }
    };

    iterator const begin() const { return iterator(*this, false); }
    iterator const end() const { return iterator(*this, true); }

    unsigned get_hash() const {
        unsigned h = 0;
        for (unsigned i = 0; i < size(); ++i) {
            h += (i+1)*((*this)[i]);
        }
        return h;
    }

    struct eq { bool operator()(uint_set const& s1, uint_set const& s2) const { return s1 == s2; } };
    struct hash { unsigned operator()(uint_set const& s) const { return s.get_hash(); } };
        
};

inline std::ostream & operator<<(std::ostream & target, const uint_set & s) {
    unsigned n = s.get_max_elem() + 1;
    target << "{";
    bool first = true;
    for (unsigned i = 0; i < n; i++) {
        if (s.contains(i)) {
            if (first) {
                first = false;
            }
            else {
                target << ", ";
            }
            target << i;
        }
    }
    target << "}";
    return target;
}


class tracked_uint_set {
    svector<char>        m_in_set;
    svector<unsigned>    m_set;
public:
    typedef svector<unsigned>::const_iterator iterator;
    void insert(unsigned v) {
        m_in_set.reserve(v+1, false);
        if (m_in_set[v])
            return;
        m_in_set[v] = true;
        m_set.push_back(v);
    }
    
    void remove(unsigned v) {
        if (contains(v)) {
            m_in_set[v] = false;
            auto i = m_set.size();
            for (; i > 0 && m_set[--i] != v; ) 
                ;
            SASSERT(m_set[i] == v);
            m_set[i] = m_set.back();
            m_set.pop_back();
        }
    }
    
    bool contains(unsigned v) const {
        return v < m_in_set.size() && m_in_set[v] != 0;
    }
    
    bool empty() const {
        return m_set.empty();
    }
    
    // erase some variable from the set
    unsigned erase() {
        SASSERT(!empty());
        unsigned v = m_set.back();
        m_set.pop_back();
        m_in_set[v] = false;
        return v;
    }
    unsigned size() const { return m_set.size(); }
    iterator begin() const { return m_set.begin(); }
    iterator end() const { return m_set.end(); }
    // void reset() { m_set.reset(); m_in_set.reset(); }
    void reset() { 
        auto sz = m_set.size();
        for (unsigned i = 0; i < sz; ++i) m_in_set[m_set[i]] = false;
        m_set.reset(); 
    }
    void finalize() { m_set.finalize(); m_in_set.finalize(); }
    tracked_uint_set& operator&=(tracked_uint_set const& other) {
        unsigned j = 0;
        for (unsigned i = 0; i < m_set.size(); ++i) {
            if (other.contains(m_set[i])) {
                m_set[j] = m_set[i];
                ++j;
            }
            else {
                m_in_set[m_set[i]] = false;
            }
        }
        m_set.resize(j);
        return *this;
    }
    tracked_uint_set& operator|=(tracked_uint_set const& other) {
        for (unsigned i = 0; i < other.m_set.size(); ++i) {
            insert(other.m_set[i]);
        }
        return *this;
    }
};

class indexed_uint_set {
    unsigned        m_size;
    unsigned_vector m_elems;
    unsigned_vector m_index;
public:
    indexed_uint_set():
        m_size(0)
    {}

    void insert_fresh(unsigned x) {
        SASSERT(!contains(x));
        m_index.reserve(x + 1, UINT_MAX);
        m_elems.reserve(m_size + 1);
        m_index[x] = m_size;
        m_elems[m_size] = x;
        m_size++;
        SASSERT(contains(x));
    }

    void insert(unsigned x) {
        if (!contains(x))
            insert_fresh(x);
    }
    
    void remove(unsigned x) {
        SASSERT(contains(x));
        unsigned y = m_elems[--m_size];
        if (x != y) {
            unsigned idx = m_index[x];
            m_index[y] = idx;
            m_elems[idx] = y;
            m_index[x] = m_size;
            m_elems[m_size] = x; 
        }
        SASSERT(!contains(x));
    }

    unsigned elem_at(unsigned index) {
        SASSERT(index < m_size);
        return m_elems[index];
    }
    unsigned operator[](unsigned index) const {
        SASSERT(index < m_size);
        return m_elems[index];
    }

    void swap_elems(unsigned i, unsigned j) {
        if (i == j)
            return;
        SASSERT(i < m_size && j < m_size);
        unsigned x = m_elems[i], y = m_elems[j];
        m_elems[i] = y; m_elems[j] = x;
        m_index[x] = j; m_index[y] = i;
    }

    bool contains(unsigned x) const { return x < m_index.size() && m_index[x] < m_size && m_elems[m_index[x]] == x; }
    void reset() { m_size = 0; }
    bool empty() const { return m_size == 0; }    
    unsigned size() const { return m_size; }
    unsigned max_var() const { return m_index.size(); }
    typedef  unsigned_vector::const_iterator iterator;
    iterator begin() const { return m_elems.begin(); }
    iterator end() const { return m_elems.begin() + m_size; }
    
};

inline std::ostream& operator<<(std::ostream& out, indexed_uint_set const& s) {
    for (unsigned i : s) out << i << " ";
    return out;
}

inline std::ostream& operator<<(std::ostream& out, tracked_uint_set const& s) {
    for (unsigned i : s) out << i << " ";
    return out;
}
