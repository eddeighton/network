
#pragma once

#include "vocab/service/mptf.hpp"
#include "vocab/value.hpp"

#include <boost/fiber/all.hpp>

#include "common/assert_verify.hpp"

#include <iostream>

namespace mega::service
{
    class LogicalThread;

    static inline boost::fibers::fiber_specific_ptr< LogicalThread > fiber_local_storage;

    using Functor = std::function< void() >;
    
    class LogicalThread
    {
        friend class Registry;
    public:
    
    private:
        using Channel = boost::fibers::buffered_channel< Functor >;

        Channel m_receiveChannel;
        MPTF m_mptf;
    public:

        LogicalThread()
            : m_receiveChannel( 128 )
        {
        }

        MPTF getMPTF() const { return m_mptf; }

        void receive()
        {
            Functor functor;
            auto status = m_receiveChannel.pop(functor);
            if( status == boost::fibers::channel_op_status::success )
            {
                functor();
            }
            else
            {
                THROW_RTE("Failed to dispatch message");
            }
        }

        void send(Functor functor)
        {
            auto status = m_receiveChannel.push(std::move(functor));
            VERIFY_RTE_MSG(status == boost::fibers::channel_op_status::success,
                "Error sending message to channel" );
        }

        void dispatchMsg(int iMsg)
        {
            THROW_TODO;
        }
    
        static inline void registerFiber( MP mp )
        {
            static thread_local FiberID::ValueType m_fiberIDCounter{};
            if( nullptr == fiber_local_storage.get() )
            {
                VERIFY_RTE_MSG( m_fiberIDCounter < std::numeric_limits<FiberID::ValueType>::max(),
                    "No remaining fiber IDs available" );
                const FiberID fiberID{ ++m_fiberIDCounter };
                const MPTF mptf( mp, ThreadID{}, fiberID );
                fiber_local_storage.reset( new LogicalThread );
                fiber_local_storage->m_mptf = mptf;
            }
        }

        static inline LogicalThread& get()
        {
            VERIFY_RTE_MSG( fiber_local_storage.get() != nullptr,
                "Fiber does not have logical thread fiber local storage set" );
            return *fiber_local_storage.get();
        }
    };

}

