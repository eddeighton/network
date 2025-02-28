//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once

#include <chrono>
#include <cstddef>
#include <memory>
#include <mutex>
#include <queue>

#include <boost/asio.hpp>
#include <boost/assert.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/config.hpp>

#include <boost/fiber/condition_variable.hpp>
#include <boost/fiber/context.hpp>
#include <boost/fiber/mutex.hpp>
#include <boost/fiber/operations.hpp>
#include <boost/fiber/scheduler.hpp>

#include "yield.hpp"

#ifdef BOOST_HAS_ABI_HEADERS
#include BOOST_ABI_PREFIX
#endif

namespace boost
{
namespace fibers
{
namespace asio
{

class round_robin : public algo::algorithm
{
private:
    //[asio_rr_suspend_timer
    std::shared_ptr< boost::asio::io_context > io_ctx_;
    boost::asio::steady_timer                  suspend_timer_;
    //]
    boost::fibers::scheduler::ready_queue_type rqueue_{};
    boost::fibers::mutex                       mtx_{};
    boost::fibers::condition_variable          cnd_{};
    std::size_t                                counter_{ 0 };

public:
    //[asio_rr_service_top
    struct service : public boost::asio::io_context::service
    {
        static boost::asio::io_context::id id;

        std::unique_ptr< boost::asio::io_context::work > work_;

        inline service( boost::asio::io_context& io_ctx )
            : boost::asio::io_context::service( io_ctx )
            , work_{ new boost::asio::io_context::work( io_ctx ) }
        {
        }

        inline virtual ~service() {}

        inline service( service const& )            = delete;
        inline service& operator=( service const& ) = delete;

        inline void shutdown_service() override final
        {
            work_.reset();
        }
    };
    //]

    //[asio_rr_ctor
    inline round_robin(
        std::shared_ptr< boost::asio::io_context > const& io_ctx )
        : io_ctx_( io_ctx )
        , suspend_timer_( *io_ctx_ )
    {
        // We use add_service() very deliberately. This will throw
        // service_already_exists if you pass the same io_context
        // instance to more than one round_robin instance.
        boost::asio::add_service( *io_ctx_, new service( *io_ctx_ ) );
        boost::asio::post(
            *io_ctx_,
            [ this ]() mutable
            {
                //]
                //[asio_rr_service_lambda
                while( !io_ctx_->stopped() )
                {
                    if( has_ready_fibers() )
                    {
                        // run all pending handlers in round_robin
                        while( io_ctx_->poll() )
                            ;
                        // block this fiber till all pending (ready)
                        // fibers are processed
                        // == round_robin::suspend_until() has been
                        // called
                        std::unique_lock< boost::fibers::mutex > lk(
                            mtx_ );
                        cnd_.wait( lk );
                    }
                    else
                    {
                        // run one handler inside io_context
                        // if no handler available, block this thread
                        if( !io_ctx_->run_one() )
                        {
                            break;
                        }
                    }
                }

                //  while ( has_ready_fibers() ) {
                //      // block this fiber till all pending (ready)
                //      fibers are processed
                //      // == round_robin::suspend_until() has been
                //      called std::unique_lock< boost::fibers::mutex
                //      > lk( mtx_); cnd_.wait( lk);
                //  }
                //]
            } );
    }

    inline void awakened( context* ctx ) noexcept
    {
        BOOST_ASSERT( nullptr != ctx );
        BOOST_ASSERT( !ctx->ready_is_linked() );
        ctx->ready_link( rqueue_ ); /*< fiber, enqueue on ready queue
                                       >*/
        if( !ctx->is_context(
                boost::fibers::type::dispatcher_context ) )
        {
            ++counter_;
        }
    }

    inline context* pick_next() noexcept
    {
        context* ctx( nullptr );
        if( !rqueue_.empty() )
        { /*<
pop an item from the ready queue
>*/
            ctx = &rqueue_.front();
            rqueue_.pop_front();
            BOOST_ASSERT( nullptr != ctx );
            BOOST_ASSERT( context::active() != ctx );
            if( !ctx->is_context(
                    boost::fibers::type::dispatcher_context ) )
            {
                --counter_;
            }
        }
        return ctx;
    }

    inline bool has_ready_fibers() const noexcept
    {
        return 0 < counter_;
    }

    //[asio_rr_suspend_until
    inline void
    suspend_until( std::chrono::steady_clock::time_point const&
                       abs_time ) noexcept
    {
        // Set a timer so at least one handler will eventually fire,
        // causing run_one() to eventually return.
        if( ( std::chrono::steady_clock::time_point::max )()
            != abs_time )
        {
            // Each expires_at(time_point) call cancels any previous
            // pending call. We could inadvertently spin like this:
            // dispatcher calls suspend_until() with earliest wake
            // time suspend_until() sets suspend_timer_ lambda loop
            // calls run_one() some other asio handler runs before
            // timer expires run_one() returns to lambda loop lambda
            // loop yields to dispatcher dispatcher finds no ready
            // fibers dispatcher calls suspend_until() with SAME wake
            // time suspend_until() sets suspend_timer_ to same time,
            // canceling previous async_wait() lambda loop calls
            // run_one() asio calls suspend_timer_ handler with
            // operation_aborted run_one() returns to lambda loop...
            // etc. etc. So only actually set the timer when we're
            // passed a DIFFERENT abs_time value.
            suspend_timer_.expires_at( abs_time );
            suspend_timer_.async_wait(
                []( boost::system::error_code const& )
                { this_fiber::yield(); } );
        }
        cnd_.notify_one();
    }
    //]

    //[asio_rr_notify
    inline void notify() noexcept
    {
        // Something has happened that should wake one or more fibers
        // BEFORE suspend_timer_ expires. Reset the timer to cause it
        // to fire immediately, causing the run_one() call to return.
        // In theory we could use cancel() because we don't care
        // whether suspend_timer_'s handler is called with
        // operation_aborted or success. However -- cancel() doesn't
        // change the expiration time, and we use suspend_timer_'s
        // expiration time to decide whether it's already set. If
        // suspend_until() set some specific wake time, then notify()
        // canceled it, then suspend_until() was called again with the
        // same wake time, it would match suspend_timer_'s expiration
        // time and we'd refrain from setting the timer. So instead of
        // simply calling cancel(), reset the timer, which cancels the
        // pending sleep AND sets a new expiration time. This will
        // cause us to spin the loop twice -- once for the
        // operation_aborted handler, once for timer expiration
        // -- but that shouldn't be a big problem.
        suspend_timer_.async_wait(
            []( boost::system::error_code const& )
            { this_fiber::yield(); } );
        suspend_timer_.expires_at( std::chrono::steady_clock::now() );
    }
    //]
};

boost::asio::io_context::id round_robin::service::id;

} // namespace asio
} // namespace fibers
} // namespace boost

#ifdef BOOST_HAS_ABI_HEADERS
#include BOOST_ABI_SUFFIX
#endif
