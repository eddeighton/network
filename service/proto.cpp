



#include "service/daemon.hpp"
#include "service/connectivity.hpp"

#include <boost/program_options.hpp>

#include <iostream>
#include <filesystem>
#include <memory>
#include <thread>

int main( int argc, const char* argv[] )
{
    using namespace mega::service;

    MP mp{};

    mega::service::LogicalThread::registerFiber(mp);

    std::atomic<bool>           bStopped{false};
    IOContextPtr pIOContext = std::make_shared< boost::asio::io_context >();
    std::unique_ptr< Server >   pServer;

    std::thread t( [&]()
        {
            mega::service::init_fiber_scheduler(pIOContext);

            pServer = std::make_unique< Server >( 
                *pIOContext,
                PortNumber{1234},
                // receive callback
                [&](Connection::WeakPtr pResponseConnection,
                        const mega::service::PacketBuffer& buffer)
                { 
                    // receive( responseSender, buffer ); 
                },
                // connection callback
                [&](mega::service::Connection::Ptr pConnection)
                {
                    // connection( pConnection );
                },
                // disconnect callback
                [&](mega::service::Connection::Ptr pConnection)
                {
                    // disconnect( pConnection );
                }
            );

            pIOContext->run();
        });

    pIOContext->post( [&]()
        {
            // must avoid blocking asio while
            // shutting down the server since the
            // sockets need to return for each
            // connection to close
            boost::fibers::fiber( [&]()
            {
                pServer->stop();
                bStopped = true;
                std::cout << "Server stop complete" << std::endl;
            }).detach();
        });
  
    while( !bStopped )
    {
        usleep(100);
    }

    mega::service::LogicalThread::registerFiber(mp);

    pIOContext->stop();
    std::cout << "io context stopped" << std::endl;

    t.join();
    std::cout <<  "thread stopped" << std::endl;

    std::cout << "main completing" << std::endl;
    return 0;
}

