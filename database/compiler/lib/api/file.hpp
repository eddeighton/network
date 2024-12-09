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

#ifndef INDEXED_FILE_25_MAR_2022
#define INDEXED_FILE_25_MAR_2022

#include "database/object_info.hpp"
#include "database/object_info.hpp"
#include "database/generics.hpp"
#include "database/object.hpp"
#include "database/object_loader.hpp"
#include "database/data_pointer.hpp"
#include "database/file_system.hpp"
#include "database/sources.hpp"
#include "database/file_info.hxx"

#include "common/hash.hpp"
#include "common/stash.hpp"

#include <cstddef>
#include <memory>
#include <optional>

namespace mega::io
{
class Manifest;
class File;
class Loader;

class File
{
public:
    using Ptr    = std::shared_ptr< File >;
    using PtrCst = std::shared_ptr< const File >;

private:
    using ObjectPtr       = std::unique_ptr< Object >;
    using OwnershipVector = std::vector< ObjectPtr >;
    using RawVector       = std::vector< Object* >;

private:
    const FileSystem&         m_fileSystem;
    FileInfo                  m_info;
    FileInfo::Stage           m_stage;
    data::ObjectPartLoader&   m_objectLoader;
    OwnershipVector           m_objects;
    RawVector                 m_rawObjects;
    std::shared_ptr< Loader > m_pLoader;

    void preload( Loader& loader );

public:
    File( const FileSystem& fileSystem, FileInfo info, FileInfo::Stage stage, data::ObjectPartLoader& objectLoader )
        : m_fileSystem( fileSystem )
        , m_info( std::move( info ) )
        , m_stage( stage )
        , m_objectLoader( objectLoader )
    {
    }

    std::size_t getTotalObjects() const { return m_objects.size(); }
    Object*     getObject( ObjectInfo::Index objectIndex ) const
    {
        VERIFY_RTE( objectIndex >= 0 );
        VERIFY_RTE(  static_cast< std::size_t >( objectIndex ) < m_objects.size() );
        return m_rawObjects[ static_cast< std::size_t >( objectIndex ) ];
    }

    FileInfo::Type             getType() const { return m_info.getFileType(); }
    ObjectInfo::FileID         getFileID() const { return m_info.getFileID(); }
    const CompilationFilePath& getFilePath() const { return m_info.getFilePath(); }
    const SourceFilePath&      getObjectSourceFilePath() const { return m_info.getObjectSourceFilePath(); }

    void           load( const Manifest& manifest );
    void           load_post( const Manifest& manifest );
    task::FileHash save_temp( const Manifest& manifest ) const;

    template < typename T, typename... Args >
    inline data::Ptr< T > construct( Args... args )
    {
        auto pNewObject = std::make_unique< T >(
            m_objectLoader, io::ObjectInfo( T::Object_Part_Type_ID, m_info.getFileID(), m_objects.size() ),
            args... );

        auto pRaw = pNewObject.get();

        m_objects.emplace_back( std::move( pNewObject ) );
        m_rawObjects.push_back( pRaw );

        return data::Ptr< T >( m_objectLoader, pRaw );
    }

    inline Range< RawVector::const_iterator > range() const { return { m_rawObjects.cbegin(), m_rawObjects.cend() }; }
    inline Range< RawVector::iterator >       range() { return { m_rawObjects.begin(), m_rawObjects.end() }; }
};

} // namespace mega::io

#endif // INDEXED_FILE_25_MAR_2022
