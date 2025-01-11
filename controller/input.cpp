

#include "service/connect.hpp"

#include "controller/event.hpp"
#include "controller/device.hpp"
#include "controller/virtual_device.hpp"

#include "common/file.hpp"
#include "common/log.hpp"

#include <boost/program_options.hpp>

#include <memory>
#include <iostream>

int main( int argc, const char* argv[] )
{
    std::string strDevInputFilePath;
    int         iLEDOn     = LED_MAX;
    int         iLEDOff    = LED_MAX;
    bool        bMonitor   = false;
    bool        bReadWrite = false;

    mega::service::PortNumber port{ 1234 };
    mega::service::IPAddress  ipAddress{ "localhost"s };

    // clang-format off
    namespace po = boost::program_options;
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
      ("help", "produce help message")
      ("device,d",    po::value< std::string >( &strDevInputFilePath ),          "/dev/input file for input device")
      ("on,o",        po::value< int >( &iLEDOn ),                               "Turn ON LED with specified integer index")
      ("off,f",       po::value< int >( &iLEDOff ),                              "Turn OFF LED with specified integer index")
      ("monitor,m",   po::value< bool >( &bMonitor )->implicit_value( true ),    "Monitor input events")
      ("readwrite,w", po::value< bool >( &bReadWrite )->implicit_value( true ),  "Open the device as read-write.  Can set LED states.")
      ( "ip,i",       po::value( &ipAddress ),                                   "Daemon IP Address" )
      ( "port,p",     po::value( &port ),                                        "Daemon Port Number" )
    ;
    // clang-format on

    boost::program_options::variables_map vm;
    boost::program_options::store(
        boost::program_options::parse_command_line(
            argc, argv, desc ),
        vm );
    boost::program_options::notify( vm );

    if( vm.count( "help" ) )
    {
        std::cout << desc << "\n";
        return 0;
    }

    try
    {
        mega::service::Connect connection( ipAddress, port );

        // attempt to find Controller
        mega::service::Ptr< mega::controller::Controller >
            pController;
        {
            using namespace mega::controller;
            using namespace mega::service;
            pController
                = connection.readRegistry()->one< Controller >();
        }

        using Path            = boost::filesystem::path;
        const Path devicePath = boost::filesystem::edsCannonicalise(
            boost::filesystem::absolute( strDevInputFilePath ) );

        using namespace mega::controller;

        EVDevice::Ptr pDevice
            = std::make_unique< EVDevice >( devicePath, bReadWrite );
        LOG( "Successfully acquired EV Device: "
             << pDevice->getInfo() );

        if( iLEDOn != LED_MAX )
        {
            pDevice->setLED( iLEDOn, true );
            return 0;
        }
        else if( iLEDOff != LED_MAX )
        {
            pDevice->setLED( iLEDOff, false );
            return 0;
        }

        if( bMonitor )
        {
            pDevice->printEvents();
        }
        else
        {
            VirtualEVDevice vDevice( *pDevice );

            pDevice->grab();

            while( true )
            {
                EventGroup eg;
                pDevice->read( eg );

                bool bConsumed = false;

                const auto& key = eg.key;
                if( key.type == EV_KEY )
                {
                    if( pController->onKeyboardEvent(
                            mega::controller::KeyboardEvent{
                                libevdev_event_code_get_name(
                                    key.type, key.code ),
                                key.value == 1 } ) )
                    {
                        // filter event
                        LOG( "Filtering event for controller: "
                             << " type: " << key.type << ' '
                             << libevdev_event_type_get_name(
                                    key.type )
                             << " code: " << key.code << ' '
                             << libevdev_event_code_get_name(
                                    key.type, key.code )
                             << " value: " << key.value );
                        bConsumed = true;
                    }
                }

                if( !bConsumed )
                {
                    vDevice.write( eg );
                }
            }
        }
    }
    catch( std::exception& ex )
    {
        LOG( "Exception: " << ex.what() );
        return -1;
    }

    return 0;
}
