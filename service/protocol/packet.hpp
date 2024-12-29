

#pragma once

#include "service/protocol/buffer.hpp"

#include "service/fibers.hpp"

#include "common/assert_verify.hpp"

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/buffer.hpp>

#include <boost/interprocess/interprocess_fwd.hpp>
#include <boost/interprocess/streams/vectorstream.hpp>

#include <boost/system/error_code.hpp>

namespace mega::service
{
    inline void onSocketError( const boost::system::error_code& ec )
    {
        if( ec.value() == boost::asio::error::eof )
        {
            //  This is what happens when close socket normally
        }
        else if( ec.value() == boost::asio::error::operation_aborted )
        {
            //  This is what happens when close socket normally
        }
        else if( ec.value() == boost::asio::error::connection_reset )
        {
        }
        else if( ec.failed() )
        {
            THROW_RTE( "Unexpected socket error: " << ec.what() );
        }
    }

    template< typename TSocket, typename TPayload >
    boost::system::error_code send( TSocket& socket, const TPayload& payload )
    {
        PacketBuffer packetBuffer;
        {
            // encode the size
            const PacketSizeType size = payload.size();
            const std::string_view sizeView( reinterpret_cast< const char* >( &size ), PacketSizeSize );
            packetBuffer.reserve( size + PacketSizeSize );
            std::copy( sizeView.begin(), sizeView.end(), std::back_inserter( packetBuffer ) );

            // encode the message
            std::copy( payload.begin(), payload.end(), std::back_inserter( packetBuffer ) );
        }

        boost::system::error_code ec;
        const auto szBytesWritten
            = boost::asio::async_write( socket, 
                boost::asio::buffer( packetBuffer ),
                boost::fibers::asio::yield[ ec ] );
        if( !ec )
        {
            VERIFY_RTE( szBytesWritten == packetBuffer.size() );
        }
        return ec;
    }

    template< typename TSocket >
    boost::system::error_code receive(TSocket& socket, PacketBuffer& packetBuffer)
    {
        boost::system::error_code ec;

        // read message size
        PacketSizeType packetSize;
        {
            std::array< char, PacketSizeSize > messageSizeBuffer;
            const auto szBytesTransferred
                = boost::asio::async_read( socket,
                    boost::asio::buffer( messageSizeBuffer ),
                    boost::fibers::asio::yield[ ec ] );
            if( !ec )
            {
                VERIFY_RTE_MSG( szBytesTransferred == PacketSizeSize,
                    "Unexpected message size size recevied of: " << szBytesTransferred );
                packetSize = *reinterpret_cast< const PacketSizeType* >( messageSizeBuffer.data() );
            }
            else
            {
                return ec;
            }
        }

        packetBuffer.resize( packetSize );
        const auto szBytesTransferred
            = boost::asio::async_read( socket,
                boost::asio::buffer( packetBuffer ),
                boost::fibers::asio::yield[ ec ] );
        if( !ec )
        {
            VERIFY_RTE_MSG( szBytesTransferred == packetSize,
                "Unexpected message size recevied of: " << szBytesTransferred );
        }
        return ec;
    }
}

