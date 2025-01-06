
#pragma once

#include "service/protocol/message.hpp"
#include "service/protocol/stack.hpp"

#include "vocab/service/mptf.hpp"
#include "vocab/value.hpp"

#include <boost/fiber/all.hpp>

#include "common/assert_verify.hpp"

#include <iostream>
#include <map>

namespace mega::service
{
    class LogicalThread
    {
        friend class Registry;
    private:
        using Channel = boost::fibers::buffered_channel< Message >;

        Channel m_receiveChannel;
        MPTF m_mptf;
        bool m_bContinue = true;
        MessageID m_interProcessMessageID;
        std::vector< Stack > m_stacks;
    public:
        LogicalThread()
            : m_receiveChannel( 16 )
        {
        }

        MPTF getMPTF() const { return m_mptf; }

        void push( const Stack& stack )
        {
            m_stacks.push_back( stack );
        }
        void pop()
        {
            m_stacks.pop_back();
        }
        Stack top()
        {
            Stack top;
            if( !m_stacks.empty() )
            {
                top = m_stacks.back();
            }
            return top;
        }

        struct StackEntry
        {
            LogicalThread& logicalThread;
            const Stack& stack;

            StackEntry( LogicalThread& logicalThread, const Stack& stack )
                : logicalThread( logicalThread )
                , stack( stack )
            {
                logicalThread.push( stack );
            }
            ~StackEntry()
            {
                logicalThread.pop();
            }
        };
        
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
    
        static void registerFiber(MP mp);
        static LogicalThread& get();
    };
}

