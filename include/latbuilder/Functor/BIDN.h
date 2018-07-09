// This file is part of LatNet Builder.
//
// Copyright (C) 2012-2018  Pierre L'Ecuyer and Universite de Montreal
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

#ifndef LATBUILDER__FUNCTOR__BIDN_H
#define LATBUILDER__FUNCTOR__BIDN_H

#include "latbuilder/Types.h"

#include <cmath>
#include <sstream>

namespace LatBuilder { namespace Functor {

/**
 * One-dimensional merit function for the \f$\mathcal P_\alpha\f$ discrepancy. TODO
 *
 * This merit function is defined as
 * \f[
 *    \omega(x) =
 *    -\frac{(-4\pi^2)^{\alpha/2}}{\alpha!} \, B_\alpha(x),
 * \f]
 * for even integers \f$\alpha\f$, where \f$B_\alpha(x)\f$ is the Bernoulli
 * polynomial of degree \f$\alpha\f$.
 */
class BIDN {
public:
   typedef Real value_type;
   typedef Real result_type;

   /**
    * Constructor.
    *
    * \param alpha     Value of \f$\alpha\f$.
    */
   BIDN(unsigned int interlacingFactor):
      m_interlacingFactor(interlacingFactor),
      m_factor( (double) (1 << (m_interlacingFactor - 1)) / ((1 << (m_interlacingFactor - 1)) - 1))
   {}

   unsigned int alpha() const
   { return m_alpha; }

    unsigned int interlacingFactor() const
   { return m_interlacingFactor; }


   bool symmetric() const
   { return false; }

   static constexpr Compress suggestedCompression()
   { return Compress::NONE; }

   /**
    * Returns the one-dimensional function evaluated at \c x.
    */
   result_type operator()(const value_type& x, uInteger n = 0) const
   { 
      if (x < std::numeric_limits<double>::epsilon()){
         return m_factor; 
      }
      else{
         return m_factor * (1 -  (double) ( (1 << m_interlacingFactor) - 1) / (1 << ( - (m_interlacingFactor - 1 ) * (int) std::floor(std::log2(x)) )));
      }
   }

   std::string name() const
   { std::ostringstream os; os << "B-IDN" << "-d" << interlacingFactor() ; return os.str(); }

private:
   unsigned int m_alpha;
   unsigned int m_interlacingFactor;
   unsigned int m_min;
   result_type m_factor;

};

/**
 * Formats \c functor and outputs it on \c os.
 */
inline
std::ostream& operator<<(std::ostream& os, const BIDN& functor)
{ return os << functor.name(); }

}}

#endif
