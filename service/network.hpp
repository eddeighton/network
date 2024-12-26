
#pragma once

#include "service/registration.hpp"

#include <mutex>
#include <shared_mutex>

namespace mega::service
{
    class Network
    {
        std::shared_mutex m_mutex;
        Registration m_registration;

    public:
        Network()
        {

        }

        struct RegistrationReadAccess
        {
            std::shared_mutex& m_mutex;
            std::shared_lock< std::shared_mutex > m_shared_lock;
            Registration& m_registration;

            RegistrationReadAccess(
                std::shared_mutex& mut,
                Registration& registration )
                : m_mutex( mut )
                , m_shared_lock( m_mutex, std::defer_lock )
                , m_registration( registration )
            {
            }

            Registration& get()
            {
                // lock the reader on access
                m_mutex.lock();
                return m_registration;
            }
        };
        struct RegistrationWriteAccess
        {
            std::shared_mutex& m_mutex;
            std::lock_guard< std::shared_mutex > m_lock_guard;
            Registration& m_registration;

            RegistrationWriteAccess(
                std::shared_mutex& mut,
                Registration& registration )
                : m_mutex( mut )
                , m_lock_guard( m_mutex )
                , m_registration( registration )
            {
            }

            Registration& get()
            {
                return m_registration;
            }
        };

        RegistrationReadAccess readRegistration()
        {
            return {m_mutex, m_registration};
        }
        RegistrationWriteAccess writeRegistration()
        {
            return {m_mutex, m_registration};
        }

    };
}

