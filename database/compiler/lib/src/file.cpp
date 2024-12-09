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

#include "database/file.hpp"
#include "database/loader.hpp"
#include "database/object_info.hpp"
#include "database/storer.hpp"
#include "database/manifest.hxx"

#include <boost/filesystem.hpp>

#include <common/hash.hpp>

namespace mega::io
{

void File::preload( Loader& loader )
{
    std::size_t szNumObjects = 0U;
    loader.load( szNumObjects );

    m_rawObjects.resize( szNumObjects );
    m_objects.resize( szNumObjects );

    for( std::size_t sz = 0U; sz < szNumObjects; ++sz )
    {
        ObjectInfo objectInfo;
        loader.load( objectInfo );

        // map the stored fileID to the runtime file ID
        // which basically means setting the fileID to this files
        objectInfo = ObjectInfo( objectInfo.getType(), getFileID(), objectInfo.getIndex() );

        // test the stored index is valid
        VERIFY_RTE( objectInfo.getIndex() >= 0 );
        const auto szIndex = static_cast< std::size_t >( objectInfo.getIndex() );
        VERIFY_RTE( szIndex < m_rawObjects.size() );

        // test the object NOT already created
        Object* pObject = m_rawObjects[ szIndex ];
        VERIFY_RTE( !pObject );

        // create the object
        pObject = data::Factory::create( m_objectLoader, objectInfo );

        m_rawObjects[ szIndex ] = pObject;
        m_objects[ szIndex ].reset( pObject );
    }
}

void File::load( const Manifest& )
{
    VERIFY_RTE( !m_pLoader );
    try
    {
        if( m_fileSystem.exists( m_info.getFilePath() ) )
        {
            m_pLoader = std::make_shared< Loader >( m_fileSystem, m_info.getFilePath(), m_objectLoader );
            preload( *m_pLoader );
            for( Object* pObject : m_rawObjects )
            {
                pObject->load( *m_pLoader );
            }
        }
    }
    catch( boost::archive::archive_exception& ex )
    {
        THROW_RTE( "Exception in stage: " << m_stage << " from boost archive when reading: "
                                          << m_info.getFilePath().path().string() << " code: " << ex.code << " : "
                                          << ex.what() );
    }
}

void File::load_post( const Manifest& manifest )
{
    try
    {
        if( m_pLoader )
        {
            VERIFY_RTE( m_pLoader );
            m_pLoader->postLoad( manifest );
            for( Object* pObject : m_rawObjects )
            {
                pObject->set_inheritance_pointer();
            }
            m_pLoader.reset();
        }
    }
    catch( boost::archive::archive_exception& ex )
    {
        THROW_RTE( "Exception in stage: " << m_stage << " from boost archive when reading: "
                                          << m_info.getFilePath().path().string() << " code: " << ex.code << " : "
                                          << ex.what() );
    }
}

task::FileHash File::save_temp( const Manifest& manifest ) const
{
    boost::filesystem::path tempFile;
    {
        try
        {
            Storer storer( m_fileSystem, m_info.getFilePath(), tempFile );

            storer.store( m_rawObjects.size() );
            for( Object* pObject : m_rawObjects )
            {
                storer.store( pObject->getObjectInfo() );
            }
            for( Object* pObject : m_rawObjects )
            {
                pObject->store( storer );
            }

            // generate reduced manifest based on used files
            auto manifestData = manifest.filterToObjects( storer.getObjectInfos() );
            storer.store( manifestData );
        }
        catch( boost::archive::archive_exception& ex )
        {
            THROW_RTE( "Exception from boost archive when writing: " << m_info.getFilePath().path().string()
                                                                     << " code: " << ex.code << " : " << ex.what() );
        }
    }
    return task::FileHash( tempFile );
}

} // namespace mega::io
