
#pragma once

#include "vocab/service/mptf.hpp"

#include <boost/fiber/all.hpp>

#include "common/assert_verify.hpp"

#include <iostream>

namespace mega::service
{

    class LogicalThread
    {
        friend class Registry;
    public:
    
    private:
        using Channel = boost::fibers::buffered_channel< int >;

        Channel m_receiveChannel;
        MPTF m_mptf;
    public:

        LogicalThread()
            : m_receiveChannel( 128 )
        {
        }

        void receive()
        {
            //         auto status = m_receiveChannel.pop(iMsg);
            //         if( status == boost::fibers::channel_op_status::success )
            //         {
            //             dispatchMsg( iMsg );
            //         }
            //         else
            //         {
            //             break;
            //         }
        }

        void dispatchMsg(int iMsg)
        {
            THROW_TODO;
        }
    };

}

