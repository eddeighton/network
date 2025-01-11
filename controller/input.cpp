
#include <iostream>

#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "libevdev-1.0/libevdev/libevdev.h"
#include "libevdev-1.0/libevdev/libevdev-uinput.h"

#include "common/file.hpp"
#include "common/assert_verify.hpp"

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
// #include <boost/circular_buffer.hpp>

#include <memory>
#include <vector>
#include <variant>

namespace mega::controller
{

// using EventRing = boost::circular_buffer< input_event >;
using EventRing = std::vector< input_event >;

struct EventGroup
{
    input_event scan, key, syn;
};

using EventSequence = std::vector< input_event >;
using Event         = std::variant< EventGroup, EventSequence >;
using EventVector   = std::vector< Event >;

class EVDevice
{
public:
    friend class VirtualEVDevice;

    using Ptr = std::unique_ptr< EVDevice >;

    EVDevice( boost::filesystem::path deviceFilePath, bool bReadOnly )
        : m_deviceFilePath( std::move( deviceFilePath ) )
        , m_bReadOnly( bReadOnly )
    {
        VERIFY_RTE_MSG( boost::filesystem::exists( m_deviceFilePath ),
                        "Could not locate device at: "
                            << m_deviceFilePath.string() );

        m_fileHandle = open( m_deviceFilePath.string().c_str(),
                             m_bReadOnly ? O_RDONLY : O_RDWR );
        VERIFY_RTE_MSG(
            m_fileHandle >= 0,
            "Failed to open device: " << m_deviceFilePath.string() );

        auto result
            = libevdev_new_from_fd( m_fileHandle, &m_pDevice );
        VERIFY_RTE_MSG(
            result >= 0,
            "Failed to initialise evdev device from file: "
                << m_deviceFilePath
                << " with error: " << strerror( -result ) );
    }
    ~EVDevice()
    {
        if( m_bGrabbed )
        {
            ungrab();
        }
        if( m_pDevice )
        {
            libevdev_free( m_pDevice );
        }
    }

    void grab()
    {
        if( !m_bGrabbed )
        {
            const auto result
                = libevdev_grab( m_pDevice, LIBEVDEV_GRAB );
            VERIFY_RTE_MSG( result >= 0, "Failed to grab device" );
            m_bGrabbed = true;
        }
    }
    void ungrab()
    {
        if( m_bGrabbed )
        {
            libevdev_grab( m_pDevice, LIBEVDEV_UNGRAB );
            m_bGrabbed = false;
        }
    }

    void setLED( int iLEDIndex, bool bOn )
    {
        VERIFY_RTE_MSG(
            !m_bReadOnly, "Cannot set LED on readonly device" );
        VERIFY_RTE_MSG( iLEDIndex >= 0 && iLEDIndex < LED_MAX,
                        "Invalid LED index specified of: "
                            << iLEDIndex << " MUST be less than; "
                            << LED_MAX );
        if( bOn )
        {
            std::cout << "Setting LED: " << iLEDIndex << " ON"
                      << std::endl;
            libevdev_kernel_set_led_value(
                m_pDevice, iLEDIndex, LIBEVDEV_LED_ON );
        }
        else
        {
            std::cout << "Setting LED: " << iLEDIndex << " OFF"
                      << std::endl;
            libevdev_kernel_set_led_value(
                m_pDevice, iLEDIndex, LIBEVDEV_LED_OFF );
        }
    }

    void getInfo( std::ostream& os ) const
    {
        // clang-format off
        os << "Input Device ID:   " << libevdev_get_id_bustype(m_pDevice) << "\n"
           << "Evdev Version:     " << libevdev_get_name(m_pDevice) << "\n"
           << "Physical Location: " << libevdev_get_phys(m_pDevice) << "\n"
           << "Unique Identifier: " << libevdev_get_uniq(m_pDevice) << "\n"
           << "Driver Version: "    << libevdev_get_driver_version(m_pDevice) << "\n"
           << std::endl;
        // clang-format on
    }

    void read( input_event& ev )
    {
        int rc = libevdev_next_event(
            m_pDevice,
            LIBEVDEV_READ_FLAG_NORMAL | LIBEVDEV_READ_FLAG_BLOCKING,
            &ev );

        if( rc == LIBEVDEV_READ_STATUS_SYNC )
        {
            while( rc == LIBEVDEV_READ_STATUS_SYNC )
            {
                rc = libevdev_next_event(
                    m_pDevice, LIBEVDEV_READ_FLAG_SYNC, &ev );
            }
        }
        else if( rc == LIBEVDEV_READ_STATUS_SUCCESS )
        {
            return;
        }
        return read( ev );
    }

    void read( EventGroup& eventGroup )
    {
        do
        {
            read( eventGroup.scan );
        } while( eventGroup.scan.type != EV_MSC );

        read( eventGroup.key );
        VERIFY_RTE_MSG( eventGroup.key.type == EV_KEY,
                        "Unexpected event following scan code" );

        read( eventGroup.syn );
        VERIFY_RTE_MSG( eventGroup.syn.type == EV_SYN,
                        "Unexpected event following key code" );
    }

    void printEvents()
    {
        int rc{};
        do
        {
            struct input_event ev;
            rc = libevdev_next_event(
                m_pDevice,
                LIBEVDEV_READ_FLAG_NORMAL
                    | LIBEVDEV_READ_FLAG_BLOCKING,
                &ev );
            if( rc == LIBEVDEV_READ_STATUS_SYNC )
            {
                while( rc == LIBEVDEV_READ_STATUS_SYNC )
                {
                    rc = libevdev_next_event(
                        m_pDevice, LIBEVDEV_READ_FLAG_SYNC, &ev );
                    std::cout << "LIBEVDEV_READ_STATUS_SYNC"
                              << std::endl;
                }
            }
            else if( rc == LIBEVDEV_READ_STATUS_SUCCESS )
            {
                std::cout << "Got keyboard event"
                          << " type: " << ev.type << ' '
                          << libevdev_event_type_get_name( ev.type )
                          << " code: " << ev.code << ' '
                          << libevdev_event_code_get_name(
                                 ev.type, ev.code )
                          << " value: " << ev.value << std::endl;

                if( ev.type == EV_SYN )
                {
                    std::cout << std::endl;
                }
            }
        } while( rc == LIBEVDEV_READ_STATUS_SYNC
                 || rc == LIBEVDEV_READ_STATUS_SUCCESS
                 || rc == -EAGAIN );
    }

private:
    boost::filesystem::path m_deviceFilePath;
    bool                    m_bReadOnly  = true;
    int                     m_fileHandle = -1;
    struct libevdev*        m_pDevice    = nullptr;
    bool                    m_bGrabbed   = false;
};

class VirtualEVDevice
{
public:
    VirtualEVDevice( const EVDevice& realDevice )
        : m_deviceFilePath( "/dev/uinput" )
    {
        VERIFY_RTE_MSG(
            realDevice.m_pDevice, "Real device not initialised" );

        m_fileHandle
            = open( m_deviceFilePath.string().c_str(), O_RDWR );
        VERIFY_RTE_MSG(
            m_fileHandle >= 0,
            "Failed to open device: " << m_deviceFilePath.string() );

        auto result = libevdev_uinput_create_from_device(
            realDevice.m_pDevice, m_fileHandle, &m_pDevice );
        VERIFY_RTE_MSG(
            result >= 0,
            "Failed to initialise evdev device with file: "
                << m_deviceFilePath
                << " with error: " << strerror( -result ) );
    }
    ~VirtualEVDevice()
    {
        if( m_pDevice )
        {
            libevdev_uinput_destroy( m_pDevice );
        }
    }

    void write( unsigned int type, unsigned int code, int value )
    {
        const auto result = libevdev_uinput_write_event(
            m_pDevice, type, code, value );
        VERIFY_RTE_MSG(
            result >= 0, "Failed to write event to virtual device" );
    }

    void write( const EventGroup& eventGroup )
    {
        write( eventGroup.scan.type,
               eventGroup.scan.code,
               eventGroup.scan.value );
        write( eventGroup.key.type,
               eventGroup.key.code,
               eventGroup.key.value );
        write( eventGroup.syn.type,
               eventGroup.syn.code,
               eventGroup.syn.value );
    }

private:
    boost::filesystem::path m_deviceFilePath;
    int                     m_fileHandle = -1;
    struct libevdev_uinput* m_pDevice    = nullptr;
};
} // namespace mega::controller

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
