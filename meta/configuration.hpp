//  Copyright (c) Deighton Systems Limited. 2025. All Rights Reserved.
//  Author: Edward Deighton
//  License: Please see license.txt in the project root folder.

//  Use and copying of this software and preparation of derivative works
//  based upon this software are permitted. Any copy of this software or
//  of any derivative work must include the above copyright notice, this
//  paragraph and the one after it.  Any distribution of this software or
//  derivative works must comply with all applicable laws.

//  This software is made available AS IS, and COPYRIGHT OWNERS DISCLAIMS
//  ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE, AND NOTWITHSTANDING ANY OTHER PROVISION CONTAINED HEREIN, ANY
//  LIABILITY FOR DAMAGES RESULTING FROM THE SOFTWARE OR ITS USE IS
//  EXPRESSLY DISCLAIMED, WHETHER ARISING IN CONTRACT, TORT (INCLUDING
//  NEGLIGENCE) OR STRICT LIABILITY, EVEN IF COPYRIGHT OWNERS ARE ADVISED
//  OF THE POSSIBILITY OF SUCH DAMAGES.

#pragma once

#include "common/serialisation.hpp"

#include "vocab/meta/configuration.hpp"

#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

#include <sstream>

namespace mega::meta
{

inline pipeline::Configuration makePipelineConfiguration( const Configuration& configuration )
{
    std::ostringstream os;
    {
        boost::archive::xml_oarchive oa( os );

        pipeline::ConfigurationHeader header = configuration.header;
        oa&                           boost::serialization::make_nvp( "pipeline_header", header );
        Configuration                 temp = configuration;
        oa&                           boost::serialization::make_nvp( "pipeline_config", temp );
    }
    return pipeline::Configuration( os.str() );
}

inline Configuration fromPipelineConfiguration( const pipeline::Configuration& pipelineConfig )
{
    Configuration configuration;
    {
        std::istringstream           is( pipelineConfig.get() );
        boost::archive::xml_iarchive ia( is );

        pipeline::ConfigurationHeader header;
        ia&                           boost::serialization::make_nvp( "pipeline_header", header );
        ia&                           boost::serialization::make_nvp( "pipeline_config", configuration );
        configuration.header = header;
    }
    return configuration;
}

} // namespace mega::meta


