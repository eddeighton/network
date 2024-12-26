
#pragma once

#include "vocab/service/mptf.hpp"

#include <boost/fiber/all.hpp>

#include <iostream>

namespace mega::service
{

    class LogicalThread
    {
        using Channel = boost::fibers::buffered_channel< int >;

        Channel m_receiveChannel;
    public:

        LogicalThread()
            : m_receiveChannel( 128 )
        {
            boost::fibers::fiber( [this]
            {
                while(true)
                {
                    std::cout << "fiber ran" << std::endl;
                    int  iMsg = 0;
                    auto status = m_receiveChannel.pop(iMsg);
                    if( status == boost::fibers::channel_op_status::success )
                    {
                        dispatchMsg( iMsg );
                    }
                    else
                    {
                        break;
                    }
                }
            }).detach();
        }

        void dispatchMsg(int iMsg)
        {
            THROW_TODO;
        }
    };

}

