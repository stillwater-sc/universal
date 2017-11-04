
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
#include "../../utilities/nested_apply_visitor.hpp"

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


template <std::size_t Nbits, std::size_t ES>
struct dummy_posit
{

    void whoami() const 
    { 
        cout << "I am dummy_posit<" << Nbits << ", " << ES << ">.\n";
    }
    
};

struct posit_test1
{
    template <std::size_t Nbits, std::size_t ES>
    void operator()() const
    {
        if (ES >= Nbits) {
            cerr << "Are you insane? Nbits must be larger than ES.\n";
            throw "Stupid test";
        }
            
        dummy_posit<Nbits, ES> dp;
        dp.whoami();
    }
    
};

int main(int argc, char** argv)
try {
	int nrOfFailedTestCases = 0;

    cout << "This is the posit selector test.\n";
    
    nbits_variant nbitsv = nbits_tag<4>{};                  // init to avoid trouble without cmd line args
    if (argc > 1)
        nbitsv = nbits_select(size_t(stoull(argv[1])));
    
    boost::apply_visitor(print_nbits_variant{}, nbitsv);
    
    
    es_variant esv = es_tag<1>{};                           // init to avoid trouble without cmd line args
    if (argc > 2)
        esv = es_select(size_t(stoull(argv[2])));
    
    boost::apply_visitor(print_es_variant{}, esv);

    
    // And now it all boils down to this:
    nested_apply_visitor(posit_test1{}, nbitsv, esv);
    
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
