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

#include <gtest/gtest.h>

#include "database/compiler/lib/api/component_type.hpp"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/interprocess/streams/vectorstream.hpp>

using namespace mega;

TEST( ComponentType, Basic )
{
    ComponentType c;

    ASSERT_EQ( c.get(), ComponentType::TOTAL_COMPONENT_TYPES );
    c.set( ComponentType::eInterface );
    ASSERT_EQ( c.get(), ComponentType::eInterface );
    ASSERT_EQ( c, ComponentType{ ComponentType::eInterface } );
}

TEST( ComponentType, XMLIO )
{
    ComponentType c1{ ComponentType::eInterface };
    ComponentType c2{ ComponentType::eLibrary };
    ComponentType c3{ ComponentType::TOTAL_COMPONENT_TYPES };

    std::stringstream ss;
    {
        boost::archive::xml_oarchive archive( ss );
        archive&                     boost::serialization::make_nvp( "c1", c1 );
        archive&                     boost::serialization::make_nvp( "c2", c2 );
        archive&                     boost::serialization::make_nvp( "c3", c3 );
    }

    ComponentType r1{ ComponentType::TOTAL_COMPONENT_TYPES };
    ComponentType r2{ ComponentType::TOTAL_COMPONENT_TYPES };
    ComponentType r3{ ComponentType::TOTAL_COMPONENT_TYPES };

    {
        boost::archive::xml_iarchive archive( ss );
        archive&                     boost::serialization::make_nvp( "c1", r1 );
        archive&                     boost::serialization::make_nvp( "c2", r2 );
        archive&                     boost::serialization::make_nvp( "c3", r3 );
    }

    ASSERT_EQ( c1, r1 );
    ASSERT_EQ( c2, r2 );
    ASSERT_EQ( c3, r3 );
}

TEST( ComponentType, BinIO )
{
    ComponentType c1{ ComponentType::eInterface };
    ComponentType c2{ ComponentType::eLibrary };
    ComponentType c3{ ComponentType::TOTAL_COMPONENT_TYPES };

    ComponentType r1{ ComponentType::TOTAL_COMPONENT_TYPES };
    ComponentType r2{ ComponentType::TOTAL_COMPONENT_TYPES };
    ComponentType r3{ ComponentType::TOTAL_COMPONENT_TYPES };

    {
        boost::interprocess::basic_vectorbuf< std::vector< char > > buffer;
        boost::archive::binary_oarchive                             saveArchive( buffer );

        saveArchive& c1;
        saveArchive& c2;
        saveArchive& c3;

        boost::archive::binary_iarchive loadArchive( buffer );

        loadArchive& r1;
        loadArchive& r2;
        loadArchive& r3;
    }

    ASSERT_EQ( c1, r1 );
    ASSERT_EQ( c2, r2 );
    ASSERT_EQ( c3, r3 );
}
