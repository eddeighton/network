
#pragma once

#include "service/protocol/message.hpp"

#include "vocab/service/mptf.hpp"
#include "vocab/value.hpp"

#include <boost/fiber/all.hpp>

#include "common/assert_verify.hpp"

#include <iostream>
#include <map>

namespace mega::service
{
    class LogicalThread;

    static inline boost::fibers::fiber_specific_ptr< LogicalThread > fiber_local_storage;
    
    class LogicalThread
    {
        friend class Registry;
    public:
    
    private:
        using Channel = boost::fibers::buffered_channel< Message >;

        Channel m_receiveChannel;
        MPTF m_mptf;
        bool m_bContinue = true;
        MessageID m_interProcessMessageID;
    public:

        LogicalThread()
            : m_receiveChannel( 128 )
        {
        }

        MPTF getMPTF() const { return m_mptf; }
        
        auto getUniqueMessageID()
        {
            ++m_interProcessMessageID;
            return m_interProcessMessageID;
        }

        inline void operator()( const InProcessRequest& inProcessRequest )
        {
            inProcessRequest.m_functor();
        }

        inline void operator()( const InProcessResponse& inProcessResponse )
        {
            inProcessResponse.m_functor();
        }

        inline void operator()( const InterProcessRequest& interProcessRequest )
        {
            interProcessRequest.m_functor();
        }

        std::map< MessageID, 
            std::function< void(const InterProcessResponse& interProcessResponse) > >
            m_interProcessResponseCallbacks;

        inline void operator()( const InterProcessResponse& interProcessResponse )
        {
            // check for callbacks
            auto iFind = m_interProcessResponseCallbacks.find(
                interProcessResponse.m_header.m_messageID );
            if( iFind != m_interProcessResponseCallbacks.end() )
            {
                iFind->second(interProcessResponse);
                m_interProcessResponseCallbacks.erase( iFind );
            }
            else
            {
                THROW_RTE("Failed to locate response callback for interprocess response: " <<
                    interProcessResponse.m_header );
            }

        }

        template< typename T >
        inline void setInterProcessResponseCallback(MessageID messageID, T&& callback)
        {
            // record the callback
            m_interProcessResponseCallbacks.insert( std::make_pair( messageID, std::move( callback ) ) );
        }

        inline void operator()( const Other& other )
        {
        }

        inline void receive()
        {
            Message message;
            auto status = m_receiveChannel.pop(message);
            if( status == boost::fibers::channel_op_status::success )
            {
                std::visit(*this, message); 
            }
            else
            {
                THROW_RTE("Failed to dispatch message");
            }
        }

        inline void runMessageLoop()
        {
            while(m_bContinue)
            {
                receive();
            }
        }

        inline void stop()
        {
            m_bContinue = false;
            auto status = m_receiveChannel.push(Message{Other{}});
            VERIFY_RTE_MSG(status == boost::fibers::channel_op_status::success,
                "Error sending message to channel" );
        }

        template< typename T >
        inline void send(T&& message)
        {
            auto status = m_receiveChannel.push(std::forward<T>(message));
            VERIFY_RTE_MSG(status == boost::fibers::channel_op_status::success,
                "Error sending message to channel" );
        }
    
        static inline void registerFiber( MP mp )
        {
            static thread_local FiberID::ValueType m_fiberIDCounter{};
            if( nullptr == fiber_local_storage.get() )
            {
                VERIFY_RTE_MSG( m_fiberIDCounter <
                    std::numeric_limits<FiberID::ValueType>::max(),
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

