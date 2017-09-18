
// selector_test.cpp: Test the run-time selection of posit formats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>
#include <string>

#include <boost/variant.hpp>

#include "../../utilities/es_select.hpp"
#include "../../utilities/nbits_select.hpp"

using namespace std;

struct print_es_variant 
  : public boost::static_visitor<void>
{
    template <std::size_t ES>
    void operator()(const es_tag<ES>&) const
    {
        cout << "ES = " << ES << endl;
    }
    
};

struct print_nbits_variant 
  : public boost::static_visitor<void>
{
    template <std::size_t Nbits>
    void operator()(const nbits_tag<Nbits>&) const
    {
        cout << "nbits = " << Nbits << endl;
    }
    
};


int main(int argc, char** argv)
{
    cout << "This is the posit selector test.\n";
    
    es_variant esv = es_tag<1>{};                           // init to avoid trouble without cmd line args
    if (argc > 1)
        esv = es_select(stoull(argv[1]));
    
    boost::apply_visitor(print_es_variant{}, esv);

    
    nbits_variant nbitsv = nbits_tag<4>{};                  // init to avoid trouble without cmd line args
    if (argc > 2)
        nbitsv = nbits_select(stoull(argv[2]));
    
    boost::apply_visitor(print_nbits_variant{}, nbitsv);
    
    
    
    return 0;
}
