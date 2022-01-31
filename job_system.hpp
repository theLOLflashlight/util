#pragma once
#include <iostream>
#include <queue>
#include <mutex>
#include <thread>
#include <vector>
#include <atomic>
#include <cassert>
#include <functional>
#include <condition_variable>

// Holds a pool of worker threads which are 
// used to dispatch jobs via a job queue.
class job_system
{
    bool enabled = true; // controls whether idle threads terminate or wait for a job
    mutable std::mutex utx; // locks access to job queue
    mutable std::condition_variable worker_thread, join_thread;
    std::vector< std::thread > workers; // pool of worker threads
    std::deque< std::function< void() > > jobs; // queue of jobs waiting to be exectuted

    // Presumes workers.emplace_back() does not allocate or invalidate for efficiency.
    void add_worker()
    {
        workers.emplace_back( [this]
        {
            for ( std::unique_lock lk( utx ); enabled; )
            {
                if ( jobs.empty() ) // wait for a job to be dispatched
                    { worker_thread.wait( lk ); continue; }
                // take the next job
                job_t job = move( jobs.front() );
                jobs.pop_front();
                if ( jobs.empty() ) // allow joining if I removed the last job
                    join_thread.notify_all();
                
                lk.unlock();
                job(); // do the job then relock
                lk.lock();
            }
        } );
    }

    void allocate_and_launch_threads( size_t num_threads )
    {
        workers.reserve( workers.size() + num_threads );
        while ( num_threads-- > 0 )
            add_worker();
    }

    _Releases_lock_( lk )
    void disable_and_stop_all_threads( std::unique_lock< std::mutex >& lk )
    {
        enabled = false; // idle threads will now terminate
        lk.unlock();
        worker_thread.notify_all(); // awaken all threads

        for ( std::thread& t : workers ) t.join(); // join on all threads
    }

public:

    using job_t = decltype( jobs )::value_type;

    job_system() = default;

    // Allocates and launches threads which immediately wait for jobs.
    explicit job_system( size_t num_threads )
        : job_system()
    {
        allocate_and_launch_threads( num_threads );
    }

    // Dispatches a job.
    void operator <<( job_t job ) { dispatch( move( job ) ); }

    // Pushes a new job onto the job queue for processing.
    void dispatch( job_t job )
    {
        if ( job == nullptr ) return;

        std::unique_lock lk( utx );
        if ( !enabled || workers.empty() ) // do the job in serial if disabled
            { lk.unlock(); return job(); }

        jobs.push_back( move( job ) ); // otherwise, take the job and notify a thread
        lk.unlock();
        worker_thread.notify_one();
    }

    // Cancels all pending jobs and prevents further jobs from 
    // being dispatched to worker pool. Returns unprocessed jobs.
    auto cancel() -> std::vector< job_t >
    {
        std::unique_lock lk( utx );

        enabled = false; // idle threads will now terminate
        std::vector< job_t > remaining;
        remaining.reserve( jobs.size() );

        while ( !jobs.empty() ) // move jobs into vector
        {
            remaining.emplace_back( move( jobs.front() ) );
            jobs.pop_front();
        }

        lk.unlock();
        join_thread.notify_all(); // allow joining
        return remaining; // return jobs
    }

    // Blocks the calling thread until all jobs are completed
    // and all worker threads have joined. Disables job system.
    void join()
    {
        std::unique_lock lk( utx );
        while ( !jobs.empty() ) // wait until job queue is empty
            join_thread.wait( lk );

        disable_and_stop_all_threads( lk );
    }

    // Pauses dispatching of jobs until all workers have joined.
    // Dispatching is then resumed using a new pool of worker threads.
    void reset( size_t num_threads )
    {
        std::unique_lock lk( utx );
        disable_and_stop_all_threads( lk );

        lk.lock();
        enabled = true; // idle threads will now wait for jobs
        workers.clear(); // deallocate all threads

        if ( num_threads > 0 )
            allocate_and_launch_threads( num_threads );
        else while ( !jobs.empty() )
        {   // serially execute and consume remaining jobs
            jobs.front()();
            jobs.pop_front();
        }
        // allow joining if there are no remaining jobs
        if ( jobs.empty() )
            join_thread.notify_all();
    }

    job_system( job_system&& sys ) noexcept
        : job_system()
    {
        std::unique_lock syslk( sys.utx );
        enabled = sys.enabled;
        jobs = move( sys.jobs );
        syslk.unlock();
        sys.join();

        workers = move( sys.workers );
        size_t num_threads = workers.size();
        workers.clear();
        allocate_and_launch_threads( num_threads );
    }

#ifndef JOB_SYSTEM_ENABLE_COPY
    job_system( const job_system& ) = delete;
#else
    job_system( const job_system& sys )
        : job_system( sys.workers.size() )
    {
        std::unique_lock lk( utx ), syslk( sys.utx );
        jobs = sys.jobs;
        lk.unlock();
        worker_thread.notify_all();
    }

    job_system& operator =( const job_system& sys )
    {
        cancel(); join();
        this->~job_system();
        new( this ) job_system( sys );
        return *this;
    }
#endif
};

int test_job()
{
    using namespace std;
    using namespace std::chrono_literals;

    job_system e( 2 );

    e << [] { cout << this_thread::get_id() << " 1\n"; };
    e << [] { cout << this_thread::get_id() << " 2\n"; };
    e << [&] { cout << this_thread::get_id() << " 3\n"; e.cancel(); };
    e << [] { cout << this_thread::get_id() << " 4\n"; };
    e << [] { cout << this_thread::get_id() << " 5\n"; };
    e << [] { cout << this_thread::get_id() << " 6\n"; };

    e.join();
    return 0;
}
