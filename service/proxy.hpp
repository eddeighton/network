
#pragma once

#include "service/rtti.hpp"
#include "vocab/service/mptfo.hpp"

#include "common/disable_special_members.hpp"

namespace mega::service
{
    template< typename T >
    class Proxy : public T, Common::DisableCopy, Common::DisableMove
    {
        friend class Registry;
    protected:
        service::MPTFO  m_mptfo;
        RTTI            m_rtti;
    protected:
        Proxy(service::MPTFO mptfo, const RTTI& rtti)
        :   m_mptfo(mptfo)
        ,   m_rtti(rtti)
        {
        }
    public:
        service::MPTFO getMPTFO() const { return m_mptfo; }
        const RTTI& getRTTI() const { return m_rtti; }
    };
}

