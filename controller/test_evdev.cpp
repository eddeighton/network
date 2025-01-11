
#include "controller/event.hpp"
#include "controller/device.hpp"
#include "controller/virtual_device.hpp"

#include "common/file.hpp"
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

    // clang-format off
    namespace bpo = boost::program_options;
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("device,d",    bpo::value< std::string >( &strDevInputFilePath ),      "/dev/input file for input device")
        ("on,o",        bpo::value< int >( &iLEDOn ),                           "Turn ON LED with specified integer index")
        ("off,f",       bpo::value< int >( &iLEDOff ),                          "Turn OFF LED with specified integer index")
        ("monitor,m",   bpo::value< bool >( &bMonitor )->implicit_value( true ),  "Monitor input events")
        ("readwrite,w",   bpo::value< bool >( &bReadWrite )->implicit_value( true ),  "Open the device as read-write.  Can set LED states.")
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
        using Path            = boost::filesystem::path;
        const Path devicePath = boost::filesystem::edsCannonicalise(
            boost::filesystem::absolute( strDevInputFilePath ) );

        using namespace mega::controller;

        EVDevice::Ptr pDevice
            = std::make_unique< EVDevice >( devicePath, bReadWrite );
        std::cout << "Successfully acquired EV Device" << std::endl;

        pDevice->getInfo( std::cout );

        if( iLEDOn != LED_MAX )
        {
            pDevice->setLED( iLEDOn, true );
        }
        else if( iLEDOff != LED_MAX )
        {
            pDevice->setLED( iLEDOff, false );
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

                const auto& key = eg.key;

                std::cout << "Got keyboard event"
                          << " type: " << key.type << ' '
                          << libevdev_event_type_get_name( key.type )
                          << " code: " << key.code << ' '
                          << libevdev_event_code_get_name(
                                 key.type, key.code )
                          << " value: " << key.value << std::endl;

                if( key.code == 57 )
                {
                    std::cout << "Filtering space key" << std::endl;
                }
                else
                {
                    vDevice.write( eg );
                }
            }
        }
    }
    catch( std::exception& ex )
    {
        std::cout << "Exception: " << ex.what() << std::endl;
        return -1;
    }

    return 0;
}
