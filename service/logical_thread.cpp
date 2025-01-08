
#include "service/logical_thread.hpp"
#include "service/registry.hpp"

#include <limits>
#include <unordered_map>
#include <shared_mutex>

namespace mega::service
{
namespace detail
{
// Global registry singleton with reader writer lock
static std::shared_mutex g_mutex;
static boost::fibers::fiber_specific_ptr< LogicalThread >
    fiber_local_storage;
using LogicalThreads
    = std::unordered_map< MPTF, LogicalThread*, MPTF::Hash >;
static LogicalThreads g_logicalThreads;

static ThreadID::ValueType   g_threadIDCounter{};
static thread_local ThreadID g_threadID{};
} // namespace detail

LogicalThread& LogicalThread::get( MPTF mptf )
{
    std::shared_lock< std::shared_mutex > lock( detail::g_mutex );
    auto iFind = detail::g_logicalThreads.find( mptf );
    VERIFY_RTE_MSG( iFind != detail::g_logicalThreads.end(),
                    "Failed to locate logical thread: " << mptf );
    return *iFind->second;
}

void LogicalThread::registerThread()
{
    static std::mutex             mut;
    std::lock_guard< std::mutex > l( mut );
    detail::g_threadID = ThreadID{ detail::g_threadIDCounter++ };
}

void LogicalThread::registerLogicalThread(
    MPTF mptf, LogicalThread* pLogicalThread )
{
    std::lock_guard< std::shared_mutex > lock( detail::g_mutex );
    auto ib = detail::g_logicalThreads.insert(
        std::make_pair( mptf, pLogicalThread ) );
    VERIFY_RTE_MSG(
        ib.second,
        "Failed to register logical thread.  Duplicate mptf found: "
            << mptf );
}

void LogicalThread::shutdownAll()
{
    std::lock_guard< std::shared_mutex > lock( detail::g_mutex );
    for( auto& [ mptf, pLogicalThread ] : detail::g_logicalThreads )
    {
        pLogicalThread->shutdown();
    }
    detail::g_logicalThreads.clear();
}

void LogicalThread::registerFiber( MP mp )
{
    static thread_local FiberID::ValueType m_fiberIDCounter{};
    if( nullptr == detail::fiber_local_storage.get() )
    {
        VERIFY_RTE_MSG(
            m_fiberIDCounter
                < std::numeric_limits< FiberID::ValueType >::max(),
            "No remaining fiber IDs available" );
        const FiberID fiberID{ m_fiberIDCounter++ };
        const MPTF    mptf( mp, detail::g_threadID, fiberID );
        detail::fiber_local_storage.reset( new LogicalThread );
        detail::fiber_local_storage->m_mptf = mptf;

        registerLogicalThread(
            mptf, detail::fiber_local_storage.get() );
    }
}

void LogicalThread::resetFiber()
{
    delete detail::fiber_local_storage.release();
}

LogicalThread& LogicalThread::get()
{
    VERIFY_RTE_MSG( detail::fiber_local_storage.get() != nullptr,
                    "Fiber does not have logical thread fiber local "
                    "storage set" );
    return *detail::fiber_local_storage.get();
}
} // namespace mega::service
