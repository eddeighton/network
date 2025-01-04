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

#include "database/directories.hpp"
#include "database/manifest_data.hpp"
#include "database/component_info.hpp"

#include "pipeline/configuration.hpp"

#include "common/hash.hpp"

#include <boost/filesystem/path.hpp>

#include <vector>
#include <string>

namespace mega::meta
{

struct Configuration
{
    pipeline::ConfigurationHeader          header;
    common::Hash                           pipelineHash;
    std::vector< io::ComponentInfo >       componentInfos;
    io::Directories                        directories;
    io::ManifestData                       manifestData;
    std::vector< boost::filesystem::path > interfacePaths;

    template < class Archive >
    inline void serialize( Archive& archive, const unsigned int )
    {
        // NOTE: header serialization handled seperately so can access in pipeline abstraction
        // archive& boost::serialization::make_nvp( "header", header );
        archive& boost::serialization::make_nvp( "pipeline", pipelineHash );
        archive& boost::serialization::make_nvp( "componentInfos", componentInfos );
        archive& boost::serialization::make_nvp( "directories", directories );
        archive& boost::serialization::make_nvp( "manifestData", manifestData );
        archive& boost::serialization::make_nvp( "interfacePaths", interfacePaths );
    }
};

} // namespace mega::compiler

