
#pragma once

#include "controller/interface/neovim.interface.hpp"
#include "service/access.hpp"
#include "service/registry.hpp"

#include "common/log.hpp"
#include "common/process.hpp"

#include <sstream>

namespace mega::controller
{
class ONeovim : public Neovim
{
    std::string m_strPipe;

public:
    ONeovim( std::string strPipe )
        : m_strPipe( std::move( strPipe ) )
    {
    }

    void loadFileLine( std::string strFileName,
                       int         lineNumber ) override
    {
        // load path to neovim
        std::ostringstream osCmd;
        osCmd << "nvim --server " << m_strPipe
              << " --remote-send \"<C-\\><C-N>:find +" << lineNumber
              << " " << strFileName << "<CR>";

        std::string strOut, strError;
        if( int r = common::runProcess( osCmd.str(), strOut, strError ) )
        {
            LOG( "Error processing: " << osCmd.str() );
            LOG( "Error out: " << strOut );
            LOG( "Error err: " << strError );
        }
    }
};

class ONeovimFactory : public NeovimFactory
{
    service::Access&              m_access;
    service::MPTFO                m_mptfo;
    service::Ptr< NeovimFactory > m_pProxy;

    using NeovimPtr    = std::unique_ptr< ONeovim >;
    using NeovimVector = std::vector< NeovimPtr >;
    NeovimVector m_tests;

public:
    ONeovimFactory( service::Access& access )
        : m_access( access )
    {
        auto reg = m_access.writeRegistry();
        m_mptfo  = reg->createInProcessProxy(
            service::LogicalThread::get().getMPTF(), *this );
        m_pProxy = reg->one< NeovimFactory >( m_mptfo );
    }

    const service::MPTFO          getMPTFO() const { return m_mptfo; }
    service::Ptr< NeovimFactory > getPtr() const { return m_pProxy; }

    service::Ptr< Neovim >
    create_neovim( std::string strNeovimPipe ) override
    {
        NeovimPtr pNeovim
            = std::make_unique< ONeovim >( strNeovimPipe );
        auto                 reg   = m_access.writeRegistry();
        const service::MPTFO mptfo = reg->createInProcessProxy(
            m_mptfo.getMPTF(), *pNeovim );
        LOG( "create_neovim called in ONeovimFactory created: "
             << mptfo );
        m_tests.push_back( std::move( pNeovim ) );
        return reg->one< Neovim >( mptfo );
    }
};
} // namespace mega::controller
