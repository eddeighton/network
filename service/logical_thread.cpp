
#include "service/logical_thread.hpp"
#include "service/registry.hpp"

#include "common/log.hpp"

#include <limits>
#include <unordered_map>
#include <shared_mutex>

// using namespace std::string_literals;
// #define LOG_LOGICAL_THREAD( msg ) LOG( "LOGICAL_THREAD: "s + msg )
#define LOG_LOGICAL_THREAD( msg )

namespace mega::service
{
namespace detail
{
static std::shared_mutex    g_mutex;
static thread_local FiberID m_fiberIDCounter{};
// Global registry singleton with reader writer lock
static boost::fibers::fiber_specific_ptr< LogicalThread >
    fiber_local_storage;
using LogicalThreads
    = std::unordered_map< MPTF, LogicalThread*, MPTF::Hash >;
static LogicalThreads        g_logicalThreads;
static ThreadID              g_threadIDCounter{};
static thread_local ThreadID g_threadID{};
} // namespace detail

void LogicalThread::reset()
{
    LOG_LOGICAL_THREAD( "reset" );
    std::lock_guard< std::shared_mutex > lock( detail::g_mutex );
    detail::m_fiberIDCounter  = FiberID{};
    detail::g_threadIDCounter = ThreadID{};
    detail::g_logicalThreads.clear();
    delete detail::fiber_local_storage.release();
}

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
    std::lock_guard< std::shared_mutex > lock( detail::g_mutex );
    LOG_LOGICAL_THREAD(
        "registerThread: " << detail::g_threadIDCounter );
    VERIFY_RTE_MSG(
        detail::g_threadIDCounter.getValue()
            < std::numeric_limits< ThreadID::ValueType >::max(),
        "No remaining thread IDs available" );
    detail::g_threadID = detail::g_threadIDCounter++;
    LOG_LOGICAL_THREAD( "registerThread: " << detail::g_threadID );
}

void LogicalThread::shutdownAll()
{
    LOG_LOGICAL_THREAD( "shutdownAll" );
    std::lock_guard< std::shared_mutex > lock( detail::g_mutex );
    for( auto& [ mptf, pLogicalThread ] : detail::g_logicalThreads )
    {
        pLogicalThread->shutdown();
    }
    detail::g_logicalThreads.clear();
}

void LogicalThread::registerFiber( MP mp )
{
    std::lock_guard< std::shared_mutex > lock( detail::g_mutex );

    if( detail::fiber_local_storage.get() )
    {
        delete detail::fiber_local_storage.release();
    }

    VERIFY_RTE_MSG(
        detail::m_fiberIDCounter.getValue()
            < std::numeric_limits< FiberID::ValueType >::max(),
        "No remaining fiber IDs available" );
    const FiberID fiberID{ detail::m_fiberIDCounter++ };
    const MPTF    mptf( mp, detail::g_threadID, fiberID );
    LOG_LOGICAL_THREAD( "registerFiber: " << mptf );
    detail::fiber_local_storage.reset( new LogicalThread{ mptf } );
    auto pLogicalThread = detail::fiber_local_storage.get();

    auto ib = detail::g_logicalThreads.insert(
        std::make_pair( mptf, pLogicalThread ) );
    VERIFY_RTE_MSG(
        ib.second,
        "Failed to register logical thread.  Duplicate mptf found: "
            << mptf );
}

LogicalThread& LogicalThread::get()
{
    std::shared_lock< std::shared_mutex > lock( detail::g_mutex );
    VERIFY_RTE_MSG( detail::fiber_local_storage.get() != nullptr,
                    "Fiber does not have logical thread fiber local "
                    "storage set" );
    return *detail::fiber_local_storage.get();
}
} // namespace mega::service
