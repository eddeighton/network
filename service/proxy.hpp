
#pragma once

#include "service/rtti.hpp"
#include "vocab/service/mptfo.hpp"

#include "common/disable_special_members.hpp"

namespace mega::service
{
    class Access;

    template< typename T >
    class Proxy : public T, Common::DisableCopy, Common::DisableMove
    {
        friend class Registry;
    protected:
        Access&         m_access;
        service::MPTFO  m_mptfo;
        RTTI            m_rtti;
    protected:
        Proxy(Access& access, MPTFO mptfo, const RTTI& rtti)
        :   m_access( access )
        ,   m_mptfo(mptfo)
        ,   m_rtti(rtti)
        {
        }
    public:
        Access& getAccess() const { return m_access; }
        service::MPTFO getMPTFO() const { return m_mptfo; }
        const RTTI& getRTTI() const { return m_rtti; }
    };
}

