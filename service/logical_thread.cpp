
#include "service/logical_thread.hpp"
#include "service/registry.hpp"

#include <limits>

namespace mega::service
{
    static boost::fibers::fiber_specific_ptr< LogicalThread > fiber_local_storage;
    
    void LogicalThread::registerFiber( MP mp )
    {
        static thread_local FiberID::ValueType m_fiberIDCounter{};
        if( nullptr == fiber_local_storage.get() )
        {
            VERIFY_RTE_MSG( m_fiberIDCounter <
                std::numeric_limits<FiberID::ValueType>::max(),
                "No remaining fiber IDs available" );
            const FiberID fiberID{ m_fiberIDCounter++ };
            const MPTF mptf( mp, ThreadID{}, fiberID );
            fiber_local_storage.reset( new LogicalThread );
            fiber_local_storage->m_mptf = mptf;

            Registry::getWriteAccess()->registerLogicalThread(mptf,
                fiber_local_storage.get());
        }
    }

    LogicalThread& LogicalThread::get()
    {
        VERIFY_RTE_MSG( fiber_local_storage.get() != nullptr,
            "Fiber does not have logical thread fiber local storage set" );
        return *fiber_local_storage.get();
    }
}

