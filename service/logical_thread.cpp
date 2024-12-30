
#include "service/logical_thread.hpp"
#include "service/registry.hpp"

#include <limits>

namespace mega::service
{
    namespace detail
    {
        // Global registry singleton with reader writer lock
        static std::shared_mutex g_mutex;
        static Registry g_registry;
        static boost::fibers::fiber_specific_ptr< LogicalThread > fiber_local_storage;
    }

    Registry::RegistryReadAccess Registry::getReadAccess()
    {
        return {detail::g_mutex, detail::g_registry};
    }
    Registry::RegistryWriteAccess Registry::getWriteAccess()
    {
        return {detail::g_mutex, detail::g_registry};
    }

    void LogicalThread::registerFiber( MP mp )
    {
        static thread_local FiberID::ValueType m_fiberIDCounter{};
        if( nullptr == detail::fiber_local_storage.get() )
        {
            VERIFY_RTE_MSG( m_fiberIDCounter <
                std::numeric_limits<FiberID::ValueType>::max(),
                "No remaining fiber IDs available" );
            const FiberID fiberID{ m_fiberIDCounter++ };
            const MPTF mptf( mp, ThreadID{}, fiberID );
            detail::fiber_local_storage.reset( new LogicalThread );
            detail::fiber_local_storage->m_mptf = mptf;

            Registry::getWriteAccess()->registerLogicalThread(mptf,
                detail::fiber_local_storage.get());
            std::cout << "Register fiber: " << mptf << std::endl;
        }
    }

    LogicalThread& LogicalThread::get()
    {
        VERIFY_RTE_MSG( detail::fiber_local_storage.get() != nullptr,
            "Fiber does not have logical thread fiber local storage set" );
        return *detail::fiber_local_storage.get();
    }
}

