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

#ifndef NETBUILDER__PARSER__LEVEL_COMBINER_PARSER_H
#define NETBUILDER__PARSER__LEVEL_COMBINER_PARSER_H

#include "latbuilder/Parser/Common.h"
#include "netbuilder/Types.h"
#include "netbuilder/LevelCombiner.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

namespace NetBuilder { namespace Parser {
namespace lbp = LatBuilder::Parser;

using namespace NetBuilder::LevelCombiner;

/**
 * Exception thrown when trying to parse an invalid size parameter.
 */
class BadLevelCombiner : public lbp::ParserError {
public:
   BadLevelCombiner(const std::string& message):
      lbp::ParserError("cannot parse combiner string: " + message)
   {}
};

/**
 * Parser for construction parameters.
 */
template <NetConstruction NC, NetBuilder::PointSetType PST>
struct LevelCombinerParser {};

template <NetConstruction NC>
struct LevelCombinerParser<NC, NetBuilder::PointSetType::UNILEVEL> {
   typedef Combiner result_type;

   static result_type parse(const CommandLine<NC,NetBuilder::PointSetType::UNILEVEL>& commandLine)
   {
       return result_type();
   }
};

template <NetConstruction NC>
struct LevelCombinerParser<NC, NetBuilder::PointSetType::MULTILEVEL> {
   typedef Combiner result_type;

   static result_type parse(const CommandLine<NC,NetBuilder::PointSetType::MULTILEVEL>& commandLine)
   {
       std::string str = commandLine.s_combiner;
       if (str=="sum")
       {
           return result_type(SumCombiner());
       }
       else if (str=="max")
       {
           return result_type(MaxCombiner());
       }
       else
       {
            std::vector<std::string> combinerStrings;
            boost::split(combinerStrings, str, boost::is_any_of(":"));
            if (combinerStrings.size()!=2 || combinerStrings[0] != "level")
            {
                throw BadLevelCombiner(str);
            }
            else
            {
                unsigned int level;
                if (combinerStrings[1] == "max")
                {
                    level = commandLine.m_sizeParam.log2NumPoints();
                }
                else
                {
                    level = boost::lexical_cast<unsigned int>(combinerStrings[1]);
                    if (level > commandLine.m_sizeParam.log2NumPoints() || level == 0)
                    {
                       throw BadLevelCombiner("incompatible combiner level and size.");
                    }  
                }
                return LevelSelectorCombiner(level);
            }
       }
       throw BadLevelCombiner(str);
       return result_type();
   }
};

}}

#endif
