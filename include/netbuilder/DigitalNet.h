// This file is part of Lattice Builder.
//
// Copyright (C) 2012-2016  Pierre L'Ecuyer and Universite de Montreal
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef NETBUILDER__DIGITAL_NET_H
#define NETBUILDER__DIGITAL_NET_H

#include "netbuilder/Types.h"
#include "netbuilder/Util.h"
#include "netbuilder/GeneratingMatrix.h"
#include "netbuilder/NetConstructionTraits.h"

#include <memory>
#include <sstream>
#include <stdexcept>

namespace NetBuilder {

typedef size_t size_type;

/** Definition of a digital net in base 2. A digital net is uniquely defined by its set
 * of generating matrices.
*/
class DigitalNet
{
    public:

        /** 
         * Default constructor 
         * @param dimension Number of coordinates in the net.
         * @param nRows Number of rows of the generating matrices.
         * @param nCols Number of columns of the generating matrices.
        */
        DigitalNet(unsigned int dimension=0, unsigned int nRows=0, unsigned int nCols=0):
            m_dimension(dimension),
            m_nRows(nRows),
            m_nCols(nCols)
        {};

        /** 
         * Default destructor
        */
        virtual ~DigitalNet() = default;

        /** 
         * Return the number of columns of the generating matrices. 
        */
        unsigned int numColumns() const { return m_nCols; }

        /** 
         * Returns the number of rows of the generating matrices. 
        */
        unsigned int numRows() const { return m_nRows; };

        /** 
         * Returns the number of points of the net 
        */
        uInteger numPoints() const { return intPow(2, m_nCols) ; }

        /** 
         * Returns the number of points of the net 
        */
        uInteger size() const { return intPow(2, m_nCols) ; }

        /** 
         * Returns the dimension (number of coordinates) of the net.
        */
        unsigned int dimension() const { return m_dimension ; }

        /** 
         * Returns the generating matrix corresponding to coordinate \c coord.
         * @param coord An integer refering to the coordinate (ranges in 1 -> dimension).
        */
        GeneratingMatrix generatingMatrix(unsigned int coord) const 
        {
            return m_generatingMatrices[coord-1]->subMatrix(m_nRows, m_nCols);
        }

        /** 
         * Returns a raw pointer to the generating matrix corresponding to coordinate \c coord.
         * @param coord An integer refering to the coordinate (ranges in 1 -> dimension).
        */
        GeneratingMatrix* pointerToGeneratingMatrix(unsigned int coord) const 
        {
            return m_generatingMatrices[coord-1].get();
        }

        // /** 
        //  *  Returns the upper-left submatrix of the generating matrix corresponding coordinate \c coord.
        //  *  @param coord Coordinate (ranges in 1 -> dimension).
        //  *  @param nRows Number of rows of the submatrix (ranges in 1 -> numRows() ).
        //  *  @param nCols Number of columns of the submatrix (ranges in 1 -> numCols() ).
        //  */ 
        // GeneratingMatrix generatingMatrix(unsigned int coord, unsigned int nRows, unsigned int nCols)
        // {
        //     return m_generatingMatrices[coord-1]->subMatrix(nRows, nCols);
        // }

        /**
         * Returns a std::string representing the net. The verbosity of the representation
         * depends on \c outputFormat.
         * @param outputFormat Format to use to represent the net.
         */ 
        virtual std::string format(OutputFormat outputFormat) const = 0;

        /** 
         * Returns a bool indicating whether the net can be viewed as a digital sequence.
         */ 
        virtual bool isSequenceViewabe() const = 0;

        /** 
         * Extends the size of the underlying generating matrices without changing the external
         * state of the object. This method will throw a std::domain_error if isSequenceViewable() returns
         * false.
         * @param nRows New number of rows.
         * @param nCols New number of columns.
         */ 
        virtual void extendSize(unsigned int nRows, unsigned int nCols) const= 0;

    protected:

        unsigned int m_dimension; // dimension of the net
        unsigned int m_nRows; // number of rows in generating matrices
        unsigned int m_nCols; // number of columns in generating matrices
        mutable std::vector<std::shared_ptr<GeneratingMatrix>> m_generatingMatrices; // vector of shared pointers to the generating matrices

        /** 
         * Most general constructor. 
         * Designed to be used by derived classes. */
        DigitalNet(unsigned int dimension, unsigned int nRows, unsigned int nCols, std::vector<std::shared_ptr<GeneratingMatrix>> genMatrices):
            m_dimension(dimension),
            m_nRows(nRows),
            m_nCols(nCols),
            m_generatingMatrices(genMatrices)
        {};

};

/** Derived class of DigitalNet designed to implement specific construction methods. The available construction methods
 * are described by the NetConstruction enumeration which is a non-type template parameter of the DigitalNetConstruction 
 * class. @see NetBuilder::NetConstructionTraits.
 * */ 
template <NetConstruction NC>
class DigitalNetConstruction : public DigitalNet
{
    public:

        typedef NetConstructionTraits<NC> ConstructionMethod;

        typedef typename ConstructionMethod::GenValue GenValue;

        typedef typename ConstructionMethod::DesignParameter DesignParameter;

        typedef typename ConstructionMethod::GeneratingMatrixComputationData GeneratingMatrixComputationData;

        /** 
         * Basic constructor with default parameters
         * @param dimension Dimension of the net.
         * @param designParameter Design parameter of the net.
         */
        DigitalNetConstruction(
            unsigned int dimension = 0, DesignParameter designParameter = ConstructionMethod::defaultDesignParameter):
            DigitalNet(dimension, ConstructionMethod::nRows(designParameter), ConstructionMethod::nCols(designParameter)),
            m_designParameter(designParameter)
        {
            if(dimension>0)
            {
                std::vector<GenValue> genValues = ConstructionMethod::defaultGenValues(dimension, m_designParameter); // get the default generating values
                m_genValues.reserve(m_dimension);
                m_genMatsComputationData.reserve(m_dimension);
                for(const auto& genValue : genValues)
                {
                    // construct the generating matrix and store them and the generating values
                    m_genMatsComputationData.push_back(std::shared_ptr<GeneratingMatrixComputationData>());
                    m_generatingMatrices.push_back(std::shared_ptr<GeneratingMatrix>(ConstructionMethod::createGeneratingMatrix(genValue,m_designParameter,m_genMatsComputationData.back())));
                    m_genValues.push_back(std::shared_ptr<GenValue>(new GenValue(std::move(genValue))));
                }
            }
        };


        /** 
         * Basic constructor with default parameters
         * @param dimension Dimension of the net.
         * @param designParameter Design parameter of the net.
         */
        DigitalNetConstruction(
            unsigned int dimension,
            DesignParameter designParameter, 
            std::vector<GenValue> genValues):
                DigitalNet(dimension, ConstructionMethod::nRows(designParameter), ConstructionMethod::nCols(designParameter)),
                m_designParameter(std::move(designParameter))
        {
            m_genValues.reserve(m_dimension);
            m_genMatsComputationData.reserve(m_dimension);
            for(const auto& genValue : genValues)
            {
                // construct the generating matrix and store them and the generating values
                m_genMatsComputationData.push_back(std::shared_ptr<GeneratingMatrixComputationData>{});
                m_generatingMatrices.push_back(std::shared_ptr<GeneratingMatrix>(ConstructionMethod::createGeneratingMatrix(genValue,m_designParameter,m_genMatsComputationData.back())));
                m_genValues.push_back(std::shared_ptr<GenValue>(new GenValue(std::move(genValue))));
            }
        }

        /**
         * Default destructor
        */
        ~DigitalNetConstruction() = default;

        /** Instantiates a digital net with a dimension increased by one using the generating value \c newGenValue. 
         * Note that the generating matrices for the lower dimensions are not copied. The net on 
         * which this method is called and the new net share these ressources.
         * @param newGenValue  Generating value used to extend the net.
         * @return A std::unique_ptr to the instantiated net.
         */ 
        std::unique_ptr<DigitalNetConstruction<NC>> extendDimension(const GenValue& newGenValue){

            // create the new generating matrix
            std::shared_ptr<GeneratingMatrixComputationData> newComputationData{};

            std::shared_ptr<GeneratingMatrix> newMat(ConstructionMethod::createGeneratingMatrix(newGenValue,m_designParameter, newComputationData));

            // copy the vector of pointers to matrices and add the new matrix
            auto genMats = m_generatingMatrices; 
            genMats.push_back(std::move(newMat));

            // copy the vector of pointers to generating values and add the new generating value
            auto genVals = m_genValues;
            genVals.push_back(std::shared_ptr<GenValue>(new GenValue(newGenValue)));

            auto computationData = m_genMatsComputationData;
            computationData.push_back(std::move(newComputationData));

            // instantiate the new net and return the unique pointer to it
            return std::unique_ptr<DigitalNetConstruction<NC>>(new DigitalNetConstruction<NC>>(m_dimension+1, m_designParameter, std::move(genVals), std::move(genMats), std::move(computationData)));
        }


        /**
         * {@inheritDoc}
         */ 
        virtual std::string format(OutputFormat outputFormat) const
        {
            std::string res = ConstructionMethod::format(m_genValues,m_designParameter,outputFormat);
            if (outputFormat==OutputFormat::GUI)
            {
                std::ostringstream stream;
                for(unsigned int dim = 1; dim <= m_dimension; ++dim)
                {
                    stream << "//dim = " << dim << std::endl;
                    stream << generatingMatrix(dim) << std::endl;
                }
                res+="\n\n";
                res+=stream.str();
            }
            return res;
        }

        /**
         * {@inheritDoc}
         */ 
        virtual bool isSequenceViewabe() const 
        {
            return ConstructionMethod::isSequenceViewable;
        }

        /**
         * {@inheritDoc}
         */ 
        virtual void extendSize(unsigned int nRows, unsigned int nCols) const 
        {
            if (isSequenceViewable())
            {
                throw std::logic_error("extendSize should be overriden but is not.");
            }
            else
            {
                throw std::domain_error("This net cannot be viewed as a digital sequence.")
            }
        }
    
    private:

        DesignParameter m_designParameter; // design parameter of the net
        mutable std::vector<std::shared_ptr<GenValue>> m_genValues; // vector of shared pointers to the generating values of the net
        mutable std::vector<std::shared_ptr<GeneratingMatrixComputationData>> m_genMatsComputationData; // vector of shared pointers to the computation data of the generating matrices of the net

        /** Most general constructor
         * for the use of std::make_unique in member fuctions.
         * @param dimension Dimension of the net.
         * @param designParameter Design parameter of the net.
         * @param genValues Vector of shared pointers to the generating values of each coordinate.
         * @param genMatrices Vector of shared pointers to the generating matrices of each coordinate.
         * @param computationData Vector of shared pointers to the computation data of the matrices of each coordinates.
        */ 
        DigitalNetConstruction(
            unsigned int dimension,
            DesignParameter designParameter,
            std::vector<std::shared_ptr<GenValue>> genValues,
            std::vector<std::shared_ptr<GeneratingMatrix>> genMatrices,
            std::vector<std::shared_ptr<GeneratingMatrixComputationData>> computationData):
                DigitalNet(dimension, ConstructionMethod::nRows(designParameter), ConstructionMethod::nCols(designParameter), std::move(genMatrices)),
                m_designParameter(std::move(designParameter)),
                m_genValues(std::move(genValues)),
                m_genMatsComputationData(std::move(computationData))
        {};
};


// Override of extendSize method in the case of Sobol construction.
template<>
void DigitalNetConstruction<NetConstruction::SOBOL>::extendSize(unsigned int nRows, unsigned int nCols) const
{
    assert(nRows == nCols);
    ConstructionMethod::extendGeneratingMatrices(nRows, nCols, m_generatingMatrices, m_genMatsComputationData);
}

}

#endif
