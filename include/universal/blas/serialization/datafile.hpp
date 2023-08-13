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
// the arithmetic types datafile is supporting
#include <universal/native/ieee754.hpp>
#include <universal/native/integers.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/number/einteger/einteger.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/areal/areal.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/dbns/dbns.hpp>
// the aggregation types that datafile is supporting
#include <universal/blas/blas.hpp>
 
namespace sw { namespace universal { namespace blas {  
    constexpr uint32_t UNIVERSAL_DATA_FILE_MAGIC_NUMBER = 0xAAA0;
    // arithmetic types Universal supports
    constexpr uint32_t UNIVERSAL_NATIVE_INT8_TYPE  = 0x0010;
    constexpr uint32_t UNIVERSAL_NATIVE_INT16_TYPE = 0x0011;
    constexpr uint32_t UNIVERSAL_NATIVE_INT32_TYPE = 0x0012;
    constexpr uint32_t UNIVERSAL_NATIVE_INT64_TYPE = 0x0013;
    constexpr uint32_t UNIVERSAL_NATIVE_FP8_TYPE   = 0x0020;
    constexpr uint32_t UNIVERSAL_NATIVE_FP16_TYPE  = 0x0021;
    constexpr uint32_t UNIVERSAL_NATIVE_FP32_TYPE  = 0x0022;
    constexpr uint32_t UNIVERSAL_NATIVE_FP64_TYPE  = 0x0023;
    constexpr uint32_t UNIVERSAL_INTEGER_TYPE      = 0x0101;
    constexpr uint32_t UNIVERSAL_FIXPNT_TYPE       = 0x0201;
    constexpr uint32_t UNIVERSAL_AREAL_TYPE        = 0x0301;
    constexpr uint32_t UNIVERSAL_BFLOAT_TYPE       = 0x0302;
    constexpr uint32_t UNIVERSAL_CFLOAT_TYPE       = 0x0303;
    constexpr uint32_t UNIVERSAL_POSIT_TYPE        = 0x0401;
    constexpr uint32_t UNIVERSAL_LNS_TYPE          = 0x0501;
    constexpr uint32_t UNIVERSAL_DBNS_TYPE         = 0x0601;
    constexpr uint32_t UNIVERSAL_UNKNOWN_ARITHMETIC_TYPE = 0xFFFF;

    // save the Universal type id given an arithmetic type
    template<typename Scalar>
    void saveTypeId(std::ostream& ostr, const Scalar& t = {}) {
        constexpr size_t nrBytes = sizeof(t);
        uint32_t typeId{ UNIVERSAL_UNKNOWN_ARITHMETIC_TYPE };
        uint32_t nrParameters{ 0 };
        uint32_t parameter[16];
        if constexpr (std::is_integral_v<Scalar>) {
            if constexpr (1 == nrBytes) {
                typeId = UNIVERSAL_NATIVE_INT8_TYPE;
            }
            else if constexpr (2 == nrBytes) {
                typeId = UNIVERSAL_NATIVE_INT16_TYPE;
            }
            else if constexpr (4 == nrBytes) {
                typeId = UNIVERSAL_NATIVE_INT32_TYPE;
            }
            else if constexpr (8 == nrBytes) {
                typeId = UNIVERSAL_NATIVE_INT64_TYPE;
            }
            else {
                std::cerr << "unsupported integer size of " << nrBytes << " bytes\n";
            }
        }
        else if constexpr (std::is_floating_point_v<Scalar>) {
            if constexpr (1 == nrBytes) {
                typeId = UNIVERSAL_NATIVE_FP8_TYPE;
            }
            else if constexpr (2 == nrBytes) {
                typeId = UNIVERSAL_NATIVE_FP16_TYPE;
            }
            else if constexpr (4 == nrBytes) {
                typeId = UNIVERSAL_NATIVE_FP32_TYPE;
            }
            else if constexpr (8 == nrBytes) {
                typeId = UNIVERSAL_NATIVE_FP64_TYPE;
            }
            else {
                std::cerr << "unsupported floating-point size of " << nrBytes << " bytes\n";
            }

        }
        else if constexpr (is_integer<Scalar>) {
            typeId = UNIVERSAL_INTEGER_TYPE;
            nrParameters = 3;
            parameter[0] = Scalar::nbits;
            parameter[1] = Scalar::bitsInBlock;
            switch (Scalar::NumberType) {
            case IntegerNumberType::IntegerNumber:
                parameter[2] = 0;
                break;
            case IntegerNumberType::WholeNumber:
                parameter[2] = 1;
                break;
            case IntegerNumberType::NaturalNumber:
                parameter[2] = 2;
                break;
            default:
                parameter[2] = -1; // error
            }
        }
        else if constexpr (is_fixpnt<Scalar>) {
            typeId = UNIVERSAL_FIXPNT_TYPE;
            nrParameters = 4;
            parameter[0] = Scalar::nbits;
            parameter[1] = Scalar::rbits;
            parameter[2] = (Scalar::arithmetic ? 1 : 0);
            parameter[3] = Scalar::bitsInBlock;
        }
//        else if constexpr (is_areal<Scalar>) {
//            typeId = UNIVERSAL_AREAL_TYPE;
//        }
//        else if constexpr (is_bfloat<Scalar>) {
//            typeId = UNIVERSAL_BFLOAT_TYPE;
//        }
        else if constexpr (is_cfloat<Scalar>) {
            typeId = UNIVERSAL_CFLOAT_TYPE;
            nrParameters = 6;
            parameter[0] = Scalar::nbits;
            parameter[1] = Scalar::es;
            parameter[2] = Scalar::bitsInBlock;
            parameter[3] = (Scalar::hasSubnormals ? 1 : 0);
            parameter[4] = (Scalar::hasSupernormals ? 1 : 0);
            parameter[5] = (Scalar::isSaturating ? 1 : 0);
        }
        else if constexpr (is_posit<Scalar>) {
            typeId = UNIVERSAL_POSIT_TYPE;
            nrParameters = 2;
            parameter[0] = Scalar::nbits;
            parameter[1] = Scalar::es;
        }
        else if constexpr (is_lns<Scalar>) {
            typeId = UNIVERSAL_LNS_TYPE;
            nrParameters = 3;
            parameter[0] = Scalar::nbits;
            parameter[1] = Scalar::rbits;
            parameter[2] = Scalar::bitsInBlock;
            //parameter[3] = xtra;
        }
        else if constexpr (is_dbns<Scalar>) {
            typeId = UNIVERSAL_DBNS_TYPE;
            nrParameters = 3;
            parameter[0] = Scalar::nbits;
            parameter[1] = Scalar::fbbits;
            parameter[2] = Scalar::bitsInBlock;
            //parameter[3] = xtra;
        }
        else {
            typeId = UNIVERSAL_UNKNOWN_ARITHMETIC_TYPE;
        }
        ostr << typeId << '\n';
        ostr << nrParameters;
        for (unsigned i = 0; i < nrParameters; ++i) {
            ostr << ' ' << parameter[i];
        }
        ostr << std::endl;
    }

/*
        The base class `ICollection` that defines the interface for adding items
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
        virtual void restore(std::istream&) = 0;
        virtual ~ICollection() {}
    };

    // CollectionHolder class template
    template <typename CollectionType>
    class CollectionHolder : public ICollection {
    public:
        CollectionHolder(CollectionType& dataStructure) : collection(dataStructure) {}

 //       void addItem(typename CollectionType::value_type item) {
 //          collection.push_back(item);
 //       }

        void save(std::ostream& ostr, bool hex) const override {
            using Scalar = CollectionType::value_type;
            saveTypeId<Scalar>(ostr);
            ostr << "# " << type_tag(Scalar()) << '\n'; // comment
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

        void restore(std::istream& istr) override {

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
            ostr << UNIVERSAL_DATA_FILE_MAGIC_NUMBER << '\n';
            for (const auto& ds : dataStructures) {
                ostr << "NEXT ";
                ds->save(ostr, hex);
            }
            return true;
		}

		bool restore(std::istream& istr) {
            uint32_t magic_number;
            istr >> magic_number;
            if (magic_number != UNIVERSAL_DATA_FILE_MAGIC_NUMBER) {
                std::cerr << "Not a Universal Data File\n";
                return false;
            }
            return true;
		}

	protected:

	private:
        std::vector<std::unique_ptr<ICollection>> dataStructures;
	};

} } }  // namespace sw::universal::blas
