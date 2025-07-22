#pragma once
// datafile.hpp: definition of a serialization format for vector, matrix, tensor
//               of custom arithmetic types
//
// Copyright (c) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT 
// 
// This file is part of the universal numbers project.
#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <memory>
// the arithmetic types datafile is supporting
#include <universal/number_systems.hpp>
// the aggregation types that datafile is supporting
#include <numeric/containers.hpp>
 
namespace sw { namespace blas {
    using namespace sw::numeric::containers;
	using namespace sw::universal;

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
    constexpr uint32_t UNIVERSAL_PAL_TYPE          = 0x0701;
    constexpr uint32_t UNIVERSAL_UNKNOWN_ARITHMETIC_TYPE = 0xFFFF;

    template<typename Scalar>
    bool generateScalarTypeId(uint32_t& typeId, uint32_t& nrParameters, uint32_t parameter[16], const Scalar& t = {}) {
        constexpr size_t nrBytes = sizeof(t);
        bool bSuccess{ true };
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
        else if constexpr (sw::universal::is_integer<Scalar>) {
            typeId = UNIVERSAL_INTEGER_TYPE;
            nrParameters = 3;
            parameter[0] = Scalar::nbits;
            parameter[1] = Scalar::bitsInBlock;
            switch (Scalar::NumberType) {
            case sw::universal::IntegerNumberType::IntegerNumber:
                parameter[2] = 0;
                break;
            case sw::universal::IntegerNumberType::WholeNumber:
                parameter[2] = 1;
                break;
            case sw::universal::IntegerNumberType::NaturalNumber:
                parameter[2] = 2;
                break;
            default:
                parameter[2] = -1; // error
            }
        }
        else if constexpr (sw::universal::is_fixpnt<Scalar>) {
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
        else if constexpr (sw::universal::is_cfloat<Scalar>) {
            typeId = UNIVERSAL_CFLOAT_TYPE;
            nrParameters = 6;
            parameter[0] = Scalar::nbits;
            parameter[1] = Scalar::es;
            parameter[2] = Scalar::bitsInBlock;
            parameter[3] = (Scalar::hasSubnormals ? 1 : 0);
            parameter[4] = (Scalar::hasSupernormals ? 1 : 0);
            parameter[5] = (Scalar::isSaturating ? 1 : 0);
        }
        else if constexpr (sw::universal::is_posit<Scalar>) {
            typeId = UNIVERSAL_POSIT_TYPE;
            nrParameters = 2;
            parameter[0] = Scalar::nbits;
            parameter[1] = Scalar::es;
        }
        else if constexpr (sw::universal::is_lns<Scalar>) {
            typeId = UNIVERSAL_LNS_TYPE;
            nrParameters = 3;
            parameter[0] = Scalar::nbits;
            parameter[1] = Scalar::rbits;
            parameter[2] = Scalar::bitsInBlock;
            //parameter[3] = xtra;
        }
        else if constexpr (sw::universal::is_dbns<Scalar>) {
            typeId = UNIVERSAL_DBNS_TYPE;
            nrParameters = 3;
            parameter[0] = Scalar::nbits;
            parameter[1] = Scalar::fbbits;
            parameter[2] = Scalar::bitsInBlock;
            //parameter[3] = xtra;
        }
        else {
            typeId = UNIVERSAL_UNKNOWN_ARITHMETIC_TYPE;
            bSuccess = false;
        }
        return bSuccess;
    }
 
    // save the Universal type id given an arithmetic type
    template<typename Scalar>
    void saveTypeId(std::ostream& ostr, const Scalar& t = {}) {
        uint32_t typeId{ UNIVERSAL_UNKNOWN_ARITHMETIC_TYPE };
        uint32_t nrParameters{ 0 };
        uint32_t parameter[16]{ 0 };
        if (generateScalarTypeId<Scalar>(typeId, nrParameters, parameter)) {
            ostr << typeId << '\n';
            ostr << nrParameters;
            for (unsigned i = 0; i < nrParameters; ++i) {
                ostr << ' ' << parameter[i];
            }
            ostr << std::endl;
        }
        else {
            std::cerr << "failed to generate type id\n";
        }
    }

    std::string collectionType(uint32_t aggregationType) {
        std::string t{""};
        switch (aggregationType) {
        case UNIVERSAL_AGGREGATE_SCALAR:
            t = "scalar";
            break;
        case UNIVERSAL_AGGREGATE_VECTOR:
            t = "vector";
            break;
        case UNIVERSAL_AGGREGATE_MATRIX:
            t = "matrix";
            break;
        case UNIVERSAL_AGGREGATE_TENSOR:
            t = "tensor";
            break;
        }
        return t;
    }

    std::string scalarType(uint32_t scalarType) {
        std::string t{""};
        switch (scalarType) {
        case UNIVERSAL_NATIVE_INT8_TYPE:
            t = "char";
            break;
        case UNIVERSAL_NATIVE_INT16_TYPE:
            t = "short";
            break;
        case UNIVERSAL_NATIVE_INT32_TYPE:
            t = "int";
            break;
        case UNIVERSAL_NATIVE_INT64_TYPE:
            t = "long long";
            break;
        case UNIVERSAL_NATIVE_FP8_TYPE:
            t = "FP8";
            break;
        case UNIVERSAL_NATIVE_FP16_TYPE:
            t = "FP16";
            break;
        case UNIVERSAL_NATIVE_FP32_TYPE:
            t = "float";
            break; 
        case UNIVERSAL_NATIVE_FP64_TYPE:
            t = "double";
            break;
        case UNIVERSAL_INTEGER_TYPE:
            t = "integer<>";
            break;
        case UNIVERSAL_FIXPNT_TYPE:
            t = "fixpnt<>";
            break;
        case UNIVERSAL_AREAL_TYPE:
            t = "areal<>";
            break;
        case UNIVERSAL_BFLOAT_TYPE:
            t = "bfloat16";
            break;
        case UNIVERSAL_CFLOAT_TYPE:
            t = "cfloat<>";
            break;
        case UNIVERSAL_POSIT_TYPE:
            t = "posit<>";
            break;
        case UNIVERSAL_LNS_TYPE:
            t = "lns<>";
            break;
        case UNIVERSAL_DBNS_TYPE:
            t = "dbns<>";
            break;
        case UNIVERSAL_PAL_TYPE:
            t = "pal<>";
            break;
        default:
            t = "unknown type";
        }
        return t;
    }
    
    /*
        The base class `ICollection` defines the interface for adding items
        to a collection, and serializing the collection to and from a stream.

        The `CollectionContainer` template class inherits from `ICollection` and implements the required
        functions for the specialized collection type. We then create instances of different specialized
        collections and collection holders. Finally, we use a `std::vector` of `std::unique_ptr` to store
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

    // CollectionContainer class template
    template <typename CollectionType>
    class CollectionContainer : public ICollection {
    public:
        CollectionContainer(CollectionType& dataStructure) : collection(dataStructure) {}

        void saveAggregationInfo(std::ostream& ostr) const {
            using Scalar = typename CollectionType::value_type;
            uint32_t typeId{ UNIVERSAL_UNKNOWN_ARITHMETIC_TYPE };
            uint32_t nrParameters{ 0 };
            uint32_t parameter[16]{ 0 };
            if (generateScalarTypeId<Scalar>(typeId, nrParameters, parameter)) {
                //std::cout << typeid(CollectionType).name() << '\n';
                ostr << "# sw::universal::blas::" << collectionType(CollectionType::AggregationType) << "<" << scalarType(typeId) << ">\n";

                ostr << CollectionType::AggregationType << ' ' << collection.size() << '\n';
            }
        }

        void save(std::ostream& ostr, bool hex) const override {
            using Scalar = typename CollectionType::value_type;
            saveTypeId<Scalar>(ostr);
            saveAggregationInfo(ostr);
            int i{ 0 };
            if (hex) {
                for (const auto& item : collection) {
                    ostr << to_hex(item, false, false);
                    ++i;
                    if ((i % 10) == 0) ostr << '\n'; else ostr << ' ';
                }
                ostr << '\n';
            }
            else {
                for (const auto& item : collection) {
                    ostr << item;
                    ++i;
                    if ((i % 10) == 0) ostr << '\n'; else ostr << ' ';
                }
                ostr << '\n';
            }
        }

        void restore(std::istream& istr) override {
            int v;
            istr >> v;
        }
    private:
        CollectionType& collection;
    };

    constexpr bool BinaryFormat = true;
    constexpr bool TextFormat = !BinaryFormat;

	template<bool SerializationFormat = TextFormat>
	class datafile {
	public:
        void clear() {
            dataStructures.clear();
            dsName.clear();
        }

        template<typename Scalar>
        void create(uint32_t aggregate) {          
            switch (aggregate) {
            case UNIVERSAL_AGGREGATE_SCALAR:
                std::cout << "Creating a scalar";
                break;
            case UNIVERSAL_AGGREGATE_VECTOR:
                std::cout << "Creating a vector";
                break;
            case UNIVERSAL_AGGREGATE_MATRIX:
                std::cout << "Creating a matrix";
                break;
            case UNIVERSAL_AGGREGATE_TENSOR:
                std::cout << "Creating a tensor";
                break;
            default:
                std::cout << "unknown aggregate\n";
            }
        }

        template<typename Aggregate>
        void add(Aggregate& ds, const std::string& name = "undefined") {
            dataStructures.push_back(std::make_unique<CollectionContainer<Aggregate>>(ds));
            dsName.push_back(name);
        }
        template<typename Aggregate>
        bool get(const std::string& name, Aggregate& ds) {
            for (int i = 0; i < dsName.size(); ++i) {
                if (dsName[i] == name) {
                    ds = dataStructures[i];
                    return true;
                }
            }
            return false;
        }

		bool save(std::ostream& ostr, bool hex = false) const {
            ostr << UNIVERSAL_DATA_FILE_MAGIC_NUMBER << '\n';
            unsigned i = 0;
            for (const auto& ds : dataStructures) {
                ds->save(ostr, hex);
                ostr << dsName[i++] << '\n';
            }
            uint32_t terminationToken{ 0 };
            ostr << terminationToken << std::endl;
            return true;
		}

        template<typename Scalar>
        void restoreVector(std::istream& istr, uint32_t nrElements) {
            vector<Scalar>* v = new vector<Scalar>;
            add<vector<Scalar>>(*v, "placeholder");
            std::string blob;
            std::getline(istr, blob); // consume newline
            Scalar item{ 0 };
            for (unsigned i = 0; i < nrElements; ++i) {
                istr >> item;
                //std::cout << "vector data item : " << item << ' ';
                v->push_back(item);
            }
//            std::cout << "restored vector is : " << *v << '\n';
        }
        template<typename Scalar>
        void restoreMatrix(std::istream& istr, uint32_t nrElements) { // we know that blas::matrix uses a vector for storage
            matrix<Scalar>* v = new matrix<Scalar>;
            add<matrix<Scalar>>(*v, "placeholder");
            Scalar item{ 0 };
            for (unsigned i = 0; i < nrElements; ++i) {
                istr >> item;
                v->push_back(item);
            }
            //            std::cout << "restored matrix is : " << *v << '\n';
        }

        template<typename Scalar>
        void restoreData(std::istream& istr, uint32_t aggregationType, uint32_t nrElements) {
            switch (aggregationType) {
            case UNIVERSAL_AGGREGATE_SCALAR:
                std::cout << "Creating a scalar\n";
                break;
            case UNIVERSAL_AGGREGATE_VECTOR:
                std::cout << "Creating a vector\n";
                restoreVector<Scalar>(istr, nrElements);
                break;
            case UNIVERSAL_AGGREGATE_MATRIX:
                std::cout << "Creating a matrix\n";
                restoreMatrix<Scalar>(istr, nrElements);
                break;
            case UNIVERSAL_AGGREGATE_TENSOR:
                std::cout << "Creating a tensor\n";
                break;
            default:
                std::cout << "unknown aggregate\n";
            }
        }
        void restoreCollection(std::istream& istr, uint32_t typeId, uint32_t nrParameters, uint32_t* parameter, uint32_t aggregationType, uint32_t nrElements) {
            switch (typeId) {
            case UNIVERSAL_NATIVE_INT32_TYPE:
                restoreData<int32_t>(istr, aggregationType, nrElements);
                break;
            case UNIVERSAL_NATIVE_FP32_TYPE:
                restoreData<float>(istr, aggregationType, nrElements);
                break;
            case UNIVERSAL_NATIVE_FP64_TYPE:
                restoreData<double>(istr, aggregationType, nrElements);
                break;
            case UNIVERSAL_CFLOAT_TYPE:
                // TBD: this is ugly: we will need to enumerate all possible
                // configurations in explicit template invocations, which will
                // be hundreds of templates, and that will be a major compilation
                // task. For the datafile restore() this would not have any positive ROI
                // but it maybe an interesting interface for a CLI, so I am 
                // keeping it in here as a discovery note.

                // one idea is to serialize just a limited set of cfloats
                // like just the small fp8 and fp16 configurations
                using onecfloat = cfloat<16, 5, uint16_t, true, false, false>;
                restoreData<onecfloat>(istr, aggregationType, nrElements);
                break;
            case UNIVERSAL_LNS_TYPE:
                using onelns = lns<8, 2, uint8_t>;
                restoreData<onelns>(istr, aggregationType, nrElements);
                break;
            case UNIVERSAL_DBNS_TYPE:
                using onedbns = dbns<8, 3, uint8_t>;
                restoreData<onedbns>(istr, aggregationType, nrElements);
                break;
            default:
                std::cout << "unknown typeId : " << typeId << '\n';
            }
        }

		bool restore(std::istream& istr) {
            constexpr bool TraceParse = true;
            uint32_t magic_number;
            istr >> magic_number;
            if (magic_number != UNIVERSAL_DATA_FILE_MAGIC_NUMBER) {
                std::cerr << "not a Universal datafile\n";
                return false;
            }
            if constexpr (TraceParse) std::cout << "magic number is correct : " << magic_number << '\n';
            clear();
            uint32_t parameter[16]{ 0 };
            uint32_t typeId;
            istr >> typeId;
            while (typeId > 0) {
                if constexpr (TraceParse) std::cout << "typeid          : " << typeId << '\n';
                uint32_t nrParameters{ 0 };
                istr >> nrParameters;
                if constexpr (TraceParse) std::cout << "nrParameters    : " << nrParameters << '\n';
                for (uint32_t i = 0; i < nrParameters; ++i) {
                    istr >> parameter[i];
                    if constexpr (TraceParse) std::cout << "parameter[" << i << "]    : " << parameter[i] << '\n';
                }
                // read the mandatory comment line
                std::string aggregationTypeComment;
                std::string token;
                istr >> token; // pick up the comment token
                if constexpr (TraceParse) std::cout << "comment token   : " << token << '\n';
                std::getline(istr, aggregationTypeComment);
                if constexpr (TraceParse) std::cout << "comment line    : " << aggregationTypeComment << std::endl;
                uint32_t aggregationType, nrElements;
                istr >> aggregationType >> nrElements;
                if constexpr (TraceParse) {
                    std::cout << "aggregationType : " << aggregationType << '\n';
                    std::cout << "nrElements      : " << nrElements << '\n';
                }

                restoreCollection(istr, typeId, nrParameters, parameter, aggregationType, nrElements);
               
                // gather the data structure name and overwrite the "undefined" placeholder created by add() in restoreCollection
                std::string name;
                istr >> name;
                if constexpr (TraceParse) std::cout << "just read ds    : " << name << '\n';
                size_t lastItem = dsName.size() - 1;
                dsName[lastItem] = name;

                // read the typeId of the next record, or the termination token
                istr >> typeId;
            }
            return true;
		}

	protected:

	private:
        std::vector<std::unique_ptr<ICollection>> dataStructures;
        std::vector<std::string> dsName;
	};

} }  // namespace sw::blas
