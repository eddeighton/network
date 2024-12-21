
//  Copyright (c) Deighton Systems Limited. 2022. All Rights Reserved.
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

#ifndef GUARD_2023_January_21_build_hash_code
#define GUARD_2023_January_21_build_hash_code

#include "common/stash.hpp"

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_member.hpp>

#include <string>
#include <map>

namespace mega::pipeline
{
class BuildHashCode
{
public:
    BuildHashCode() {}

    BuildHashCode( const boost::filesystem::path& filePath, const task::FileHash& fileHashCode )
        : m_filePath( filePath )
        , m_fileHashCode( fileHashCode )
    {
    }

    template < typename Archive >
    void save( Archive& archive, const unsigned int ) const
    {
        archive&          boost::serialization::make_nvp( "FilePath", m_filePath );
        const std::string strHex = m_fileHashCode.toHexString();
        archive&          boost::serialization::make_nvp( "FileHash", strHex );
    }

    template < typename Archive >
    void load( Archive& archive, const unsigned int )
    {
        archive&    boost::serialization::make_nvp( "FilePath", m_filePath );
        std::string strHex;
        archive&    boost::serialization::make_nvp( "FileHash", strHex );
        m_fileHashCode.set( common::Hash::fromHexString( strHex ) );
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    boost::filesystem::path m_filePath;
    task::FileHash          m_fileHashCode;
};
}

#endif //GUARD_2023_January_21_build_hash_code
