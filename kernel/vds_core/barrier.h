#ifndef __VDS_CORE_BARRIER_H_
#define __VDS_CORE_BARRIER_H_

/*
Copyright (c) 2017, Vadim Malyshev, lboss75@gmail.com
All rights reserved
*/

#include <condition_variable>
#include <mutex>

namespace vds {

    class barrier
    {
    public:
        barrier(size_t init_value = 1);
        ~barrier();

        void set();
        void wait();
        void reset(size_t init_value = 1);
        
        barrier & operator ++ ();
        barrier & operator -- ();
        
        template<typename _Rep, typename _Period>
        bool wait_for(const std::chrono::duration<_Rep, _Period> & period)
        {
          std::unique_lock<std::mutex> lock(this->mutex_);

          if (0 == this->count_) {
            return true;
          }
          
          return this->cond_.wait_for(
                lock,
                period,
                [this] { return 0 == this->count_; });
        }


    private:
        std::mutex mutex_;
        size_t count_;
        std::condition_variable cond_;
    };
}

#endif // !__VDS_CORE_BARRIER_H_



