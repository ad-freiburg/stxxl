/***************************************************************************
 *  tests/algo/test_sort.cpp
 *
 *  Part of the STXXL. See http://stxxl.org
 *
 *  Copyright (C) 2002 Roman Dementiev <dementiev@mpi-sb.mpg.de>
 *  Copyright (C) 2010 Andreas Beckmann <beckmann@cs.uni-frankfurt.de>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See accompanying file LICENSE_1_0.txt or copy at
 *  http://www.boost.org/LICENSE_1_0.txt)
 **************************************************************************/

#define STXXL_DEFAULT_BLOCK_SIZE(T) 4096

//! \example algo/test_sort.cpp
//! This is an example of how to use \c stxxl::sort() algorithm

#include <foxxll/mng.hpp>
#include <stxxl/sort>
#include <stxxl/vector>
#include <stxxl/bits/common/padding.h>

#define RECORD_SIZE 8

using KEY_TYPE = unsigned;

struct my_type : stxxl::padding<RECORD_SIZE - sizeof(KEY_TYPE)>
{
    using key_type = KEY_TYPE;

    key_type m_key;

    key_type key() const
    {
        return m_key;
    }

    my_type() { }
    explicit my_type(key_type k) : m_key(k)
    {}

    static my_type min_value()
    {
        return my_type(std::numeric_limits<key_type>::min());
    }

    static my_type max_value()
    {
        return my_type(std::numeric_limits<key_type>::max());
    }
};

std::ostream& operator << (std::ostream& o, const my_type& obj)
{
    o << obj.m_key;
    return o;
}

bool operator < (const my_type& a, const my_type& b)
{
    return a.key() < b.key();
}

bool operator == (const my_type& a, const my_type& b)
{
    return a.key() == b.key();
}

bool operator != (const my_type& a, const my_type& b)
{
    return a.key() != b.key();
}

struct cmp : public std::less<my_type>
{
    my_type min_value() const
    {
        return my_type::min_value();
    }
    my_type max_value() const
    {
        return my_type::max_value();
    }
};

int main()
{
#if STXXL_PARALLEL_MULTIWAY_MERGE
    STXXL_MSG("STXXL_PARALLEL_MULTIWAY_MERGE");
#endif
    unsigned memory_to_use = 64 * STXXL_DEFAULT_BLOCK_SIZE(my_type);
    using vector_type = stxxl::vector<my_type>;

    foxxll::stats_data stats_begin(*foxxll::stats::get_instance());

    {
        // test small vector that can be sorted internally
        vector_type v(3);
        v[0] = my_type(42);
        v[1] = my_type(0);
        v[2] = my_type(23);
        STXXL_MSG("small vector unsorted " << v[0] << " " << v[1] << " " << v[2]);
        //stxxl::sort(v.begin(), v.end(), cmp(), memory_to_use);
        stxxl::stl_in_memory_sort(v.begin(), v.end(), cmp());
        STXXL_MSG("small vector sorted   " << v[0] << " " << v[1] << " " << v[2]);
        STXXL_CHECK(stxxl::is_sorted(v.cbegin(), v.cend(), cmp()));
    }

    const uint64_t n_records = uint64_t(192) * uint64_t(STXXL_DEFAULT_BLOCK_SIZE(my_type)) / sizeof(my_type);
    vector_type v(n_records);

    stxxl::random_number32 rnd;
    STXXL_MSG("Filling vector..., input size = " << v.size() << " elements (" << ((v.size() * sizeof(my_type)) >> 20) << " MiB)");
    for (vector_type::size_type i = 0; i < v.size(); i++)
        v[i].m_key = 1 + (rnd() % 0xfffffff);

    STXXL_MSG("Checking order...");
    STXXL_CHECK(!stxxl::is_sorted(v.cbegin(), v.cend(), cmp()));

    STXXL_MSG("Sorting (using " << (memory_to_use >> 20) << " MiB of memory)...");
    stxxl::sort(v.begin(), v.end(), cmp(), memory_to_use);

    STXXL_MSG("Checking order...");
    STXXL_CHECK(stxxl::is_sorted(v.cbegin(), v.cend(), cmp()));

    STXXL_MSG("Done, output size=" << v.size());

    std::cout << foxxll::stats_data(*foxxll::stats::get_instance()) - stats_begin;

    return 0;
}

// vim: et:ts=4:sw=4
