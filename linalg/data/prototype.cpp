// scaling.cpp: test suite for scaling functions for data preprocessing
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <memory>

/*
In this example, we create a base class `ICollection` that defines the interface for 
adding items and displaying items in a collection. 
The `CollectionHolder` template class inherits from `ICollection` and implements the 
required functions for the specialized collection type. We then create instances of 
different specialized collections and collection holders. Finally, we use a
`std::vector` of `std::unique_ptr` to store references to the different collections, 
and we can interact with them through the base class interface.

This approach allows you to aggregate references to different template specialized 
collections using a common base class. The base class reference enables you to treat 
these different collections uniformly in terms of their interface, even though they 
have different underlying types.
*/

// Base class for the collection
class ICollection {
public:
//    virtual void addItem() = 0;
    virtual void displayItems() = 0;
    virtual ~ICollection() {}
};

// CollectionHolder class template
template <typename CollectionType>
class CollectionHolder : public ICollection {
public:
    CollectionHolder(CollectionType& collection) : c(collection) {}
    ~CollectionHolder() {}

    void addItem(typename CollectionType::value_type item) {
        c.push_back(item);
    }

    void displayItems() override {
        for (const auto& item : c) {
            std::cout << item << " ";
        }
        std::cout << std::endl;
    }

private:
    CollectionType& c;
};

int main() {
    // Create instances of different specialized collections
    std::vector<int> intVector;
    std::list<std::string> stringList;
//    std::map<std::string, int> stringIntMap;

    intVector.push_back(1);
    intVector.push_back(2);
    intVector.push_back(3);
    // Create collection holders with references to the specialized collections
    CollectionHolder<std::vector<int>> intCollection(intVector);
    CollectionHolder<std::list<std::string>> stringCollection(stringList);
//    CollectionHolder<std::map<std::string, int>> mapCollection(stringIntMap);

    // Use the base class reference to aggregate the collections
    std::vector<std::unique_ptr<ICollection>> collections;
    collections.push_back(std::make_unique<CollectionHolder<std::vector<int>>>(intVector));
    collections.push_back(std::make_unique<CollectionHolder<std::list<std::string>>>(stringList));
//    collections.push_back(std::make_unique<CollectionHolder<std::map<std::string, int>>>(stringIntMap));

    // Add items to and display items from each collection through the base class reference
    for (const auto& collection : collections) {
 //       collection->addItem(/* add item appropriate to the collection */); // incorrect as base class cannot have a specialized addItem method
        collection->displayItems();
    }

    return 0;
}

