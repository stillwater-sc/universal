// nested_apply_visitor.hpp: apply visitor with pair of variants
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#pragma once

#include <type_traits>
#include <stdexcept>

#include <boost/variant.hpp>

#include "es_select.hpp"
#include "nbits_select.hpp"

template <typename Visitor, size_t Nbits>
struct inner_applicator
  : public boost::static_visitor<void>
{
    inner_applicator(Visitor vis) : vis_(vis) {}
    
    template <std::size_t ES>
    void operator()(const es_tag<ES>&) const
    {
        vis_.template operator()<Nbits, ES>();
    }
    
    Visitor vis_;
};

template <typename Visitor, typename Variant2>
struct outer_applicator
  : public boost::static_visitor<void>
{
    outer_applicator(Visitor vis, const Variant2& v2) : vis_(vis), v2_(v2) {}
    
    template <std::size_t Nbits>
    void operator()(const nbits_tag<Nbits>&) const
    {
        boost::apply_visitor(inner_applicator<Visitor, Nbits>(vis_), v2_);
    }
    
    Visitor vis_;
    const Variant2& v2_;
};


template <typename Visitor, typename Variant1, typename Variant2>
void nested_apply_visitor(Visitor vis, const Variant1& v1, const Variant2& v2)
{
    boost::apply_visitor(outer_applicator<Visitor, Variant2>(vis, v2), v1);
}
