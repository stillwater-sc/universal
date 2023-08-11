/** **********************************************************************
 * datafile.hpp: definition of a serialization format for vector, matrix, tensor
 *               of custom arithmetic types
 *
 * @author:     Theodore Omtzigt
 * @date:       2023-08-11
 * @copyright:  Copyright (c) 2023 Stillwater Supercomputing, Inc.
 * @license:    MIT Open Source license 
 * 
 * This file is part of the universal numbers project.
 * ***********************************************************************
 */
#pragma once
#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <memory>
#include <universal/native/ieee754.hpp>
#include <universal/blas/blas.hpp>
 
namespace sw { namespace universal { namespace blas {  

/*
        Theo base class `ICollection` that defines the interface for adding items
        and displaying items in a collection.

        The `datafile` template class inherits from `ICollection` and implements the required
        functions for the specialized collection type. We then create instances of different specialized
        collections and collection holders.Finally, we use a `std::vector` of `std::unique_ptr` to store
        references to the different collections, and we can interact with them through the base class interface.

        This approach allows you to aggregate references to different template specialized collections
        using a common base class. The base class reference enables you to treat these different collections
        uniformly in terms of their interface, even though they have different underlying types.
    */

    // Base class for the collection
    class ICollection {
    public:
        virtual void save(std::ostream&, bool) const = 0;
        virtual ~ICollection() {}
    };

    // CollectionHolder class template
    template <typename CollectionType>
    class CollectionHolder : public ICollection {
    public:
        CollectionHolder(CollectionType& dataStructure) : collection(dataStructure) {}

        void addItem(typename CollectionType::value_type item) {
            collection.push_back(item);
        }

        void save(std::ostream& ostr, bool hex) const override {
            using Scalar = CollectionType::value_type;
            ostr << type_tag(Scalar()) << '\n';
            if (hex) {
                for (const auto& item : collection) {
                    ostr << to_hex(item, false, false) << ' ';
                }
                std::cout << std::endl;
            }
            else {
                for (const auto& item : collection) {
                    ostr << item << ' ';
                    //ostr << to_binary(item) << " : " << to_hex(item) << " : " << item << '\n';
                }
                std::cout << std::endl;
            }
        }

    private:
        CollectionType& collection;
    };

    constexpr bool BinaryFormat = true;
    constexpr bool TextFormat = !BinaryFormat;

	template<bool SerializationFormat = TextFormat>
	class datafile {
	public:
        template<typename Aggregate>
        void add(Aggregate& ds) {
            dataStructures.push_back(std::make_unique<CollectionHolder<Aggregate>>(ds));
        }

		bool save(std::ostream& ostr, bool hex = false) const {
            for (const auto& ds : dataStructures) {
                ostr << "NEXT ";
                ds->save(ostr, hex);
            }
            return true;
		}

		bool restore(std::istream& istr) {

		}

	protected:

	private:
        std::vector<std::unique_ptr<ICollection>> dataStructures;
	};

} } }  // namespace sw::universal::blas
