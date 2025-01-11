

#pragma once

#include "event.hpp"
#include "device.hpp"

#include "libevdev/libevdev-uinput.h"

namespace mega::controller
{

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

}

