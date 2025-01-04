

#pragma once

#include "meta/environment.hpp"

#include "pipeline/stash.hpp"

#include "common/string.hpp"

namespace mega::io
{
    class MetaStashEnvironment : public MetaEnvironment
    {
        mega::pipeline::Stash& m_stash;

    public:
        MetaStashEnvironment( const Directories& directories, mega::pipeline::Stash& stash )
            :  MetaEnvironment( directories )
            ,  m_stash( stash )
        {

        }

        template < typename TFilePathType >
        task::FileHash getBuildHashCode( const TFilePathType& filePath ) const
        {
            return m_stash.getBuildHashCode( toPath( filePath ) );
        }

        task::FileHash getBuildHashCodePath( const boost::filesystem::path& filePath ) const
        {
            return m_stash.getBuildHashCode( filePath );
        }

        template < typename TFilePathType >
        void setBuildHashCode( const TFilePathType& filePath, task::FileHash hashCode ) const
        {
            m_stash.setBuildHashCode( toPath( filePath ), hashCode );
        }

        inline void setBuildHashCodePath( const boost::filesystem::path& filePath ) const
        {
            m_stash.setBuildHashCode( filePath, task::FileHash( filePath ) );
        }

        template < typename TFilePathType >
        void setBuildHashCode( const TFilePathType& filePath ) const
        {
            m_stash.setBuildHashCode( toPath( filePath ), task::FileHash( toPath( filePath ) ) );
        }

        template < typename TFilePathType >
        void stash( const TFilePathType& filePath, task::DeterminantHash hashCode ) const
        {
            m_stash.stash( toPath( filePath ), hashCode );
        }

        inline void stashPath( const boost::filesystem::path& filePath, task::DeterminantHash hashCode ) const
        {
            m_stash.stash( filePath, hashCode );
        }

        template < typename TFilePathType >
        bool restore( const TFilePathType& filePath, task::DeterminantHash hashCode ) const
        {
            return m_stash.restore( toPath( filePath ), hashCode );
        }

        inline bool restorePath( const boost::filesystem::path& filePath, task::DeterminantHash hashCode ) const
        {
            return m_stash.restore( filePath, hashCode );
        }

        bool restore( const CompilationFilePath& filePath, task::DeterminantHash hashCode ) const;

        virtual mega::SymbolTable getSymbolTable() const { return m_stash.getSymbolTable(); }
        virtual mega::SymbolTable newSymbols( const mega::SymbolRequest& request ) const { return m_stash.newSymbols( request ); }
    };
}

