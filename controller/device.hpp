
#pragma once

#include "event.hpp"

#include "common/assert_verify.hpp"
#include "common/log.hpp"

#include <boost/filesystem.hpp>

#include <memory>
#include <ostream>

#include "libevdev/libevdev.h"

#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>

namespace mega::controller
{

class EVDevice
{
public:
    friend class VirtualEVDevice;

    using Ptr = std::unique_ptr< EVDevice >;

    EVDevice( boost::filesystem::path deviceFilePath,
              bool                    bReadWrite )
        : m_deviceFilePath( std::move( deviceFilePath ) )
        , m_bReadWrite( bReadWrite )
    {
        VERIFY_RTE_MSG( boost::filesystem::exists( m_deviceFilePath ),
                        "Could not locate device at: "
                            << m_deviceFilePath.string() );

        m_fileHandle = open( m_deviceFilePath.string().c_str(),
                             m_bReadWrite ? O_RDWR : O_RDONLY );
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
            !m_bReadWrite, "Cannot set LED on readonly device" );
        VERIFY_RTE_MSG( iLEDIndex >= 0 && iLEDIndex < LED_MAX,
                        "Invalid LED index specified of: "
                            << iLEDIndex << " MUST be less than; "
                            << LED_MAX );
        if( bOn )
        {
            LOG( "Setting LED: " << iLEDIndex << " ON" );
            libevdev_kernel_set_led_value(
                m_pDevice, iLEDIndex, LIBEVDEV_LED_ON );
        }
        else
        {
            LOG( "Setting LED: " << iLEDIndex << " OFF" );
            libevdev_kernel_set_led_value(
                m_pDevice, iLEDIndex, LIBEVDEV_LED_OFF );
        }
    }

    std::string getInfo() const
    {
        std::ostringstream os;
        // clang-format off
        os << "Input Device ID:   " << libevdev_get_id_bustype(m_pDevice) << "\n"
           << "Evdev Version:     " << libevdev_get_name(m_pDevice) << "\n"
           << "Physical Location: " << libevdev_get_phys(m_pDevice) << "\n"
           << "Unique Identifier: " << libevdev_get_uniq(m_pDevice) << "\n"
           << "Driver Version: "    << libevdev_get_driver_version(m_pDevice) << "\n"
           << std::endl;
        // clang-format on
        return os.str();
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
                    LOG( "LIBEVDEV_READ_STATUS_SYNC" );
                }
            }
            else if( rc == LIBEVDEV_READ_STATUS_SUCCESS )
            {
                LOG( "Got keyboard event"
                     << " type: " << ev.type << ' '
                     << libevdev_event_type_get_name( ev.type )
                     << " code: " << ev.code << ' '
                     << libevdev_event_code_get_name(
                            ev.type, ev.code )
                     << " value: " << ev.value );

                if( ev.type == EV_SYN )
                {
                    LOG( "" );
                }
            }
        } while( rc == LIBEVDEV_READ_STATUS_SYNC
                 || rc == LIBEVDEV_READ_STATUS_SUCCESS
                 || rc == -EAGAIN );
    }

private:
    boost::filesystem::path m_deviceFilePath;
    bool                    m_bReadWrite = true;
    int                     m_fileHandle = -1;
    struct libevdev*        m_pDevice    = nullptr;
    bool                    m_bGrabbed   = false;
};

} // namespace mega::controller
