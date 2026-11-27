#pragma once

// Simple threading compatibility layer for old MinGW
// Uses Windows threads on Windows, pthreads on Unix

#ifdef _WIN32
    #include <windows.h>
    #include <process.h>
    
    namespace pebblekv {
    
    // Simple mutex wrapper
    class Mutex {
    public:
        Mutex() { InitializeCriticalSection(&cs_); }
        ~Mutex() { DeleteCriticalSection(&cs_); }
        void lock() { EnterCriticalSection(&cs_); }
        void unlock() { LeaveCriticalSection(&cs_); }
    private:
        CRITICAL_SECTION cs_;
        Mutex(const Mutex&) = delete;
        Mutex& operator=(const Mutex&) = delete;
    };
    
    // RAII lock guard
    class LockGuard {
    public:
        explicit LockGuard(Mutex& m) : mutex_(m) { mutex_.lock(); }
        ~LockGuard() { mutex_.unlock(); }
    private:
        Mutex& mutex_;
        LockGuard(const LockGuard&) = delete;
        LockGuard& operator=(const LockGuard&) = delete;
    };
    
    // Simple atomic bool
    class AtomicBool {
    public:
        AtomicBool(bool value = false) : value_(value ? 1 : 0) {}
        operator bool() const { return InterlockedCompareExchange(const_cast<volatile LONG*>(&value_), 0, 0) != 0; }
        AtomicBool& operator=(bool value) { InterlockedExchange(&value_, value ? 1 : 0); return *this; }
    private:
        volatile LONG value_;
    };
    
    // Thread handle wrapper
    class Thread {
    public:
        Thread() : handle_(NULL), data_(nullptr) {}
        
        // Generic thread function
        static unsigned __stdcall threadFunc(void* param);
        
        // Start thread with function pointer, object, and socket
        void start(void* func_ptr, void* obj_ptr, void* arg_ptr) {
            struct ThreadData {
                void* func;
                void* obj;
                void* arg;
            };
            data_ = new ThreadData{func_ptr, obj_ptr, arg_ptr};
            
            handle_ = (HANDLE)_beginthreadex(nullptr, 0, threadFunc, data_, 0, nullptr);
        }
        
        void join() {
            if (handle_) {
                WaitForSingleObject(handle_, INFINITE);
                CloseHandle(handle_);
                handle_ = NULL;
            }
        }
        
        bool joinable() const { return handle_ != NULL; }
        
    private:
        HANDLE handle_;
        void* data_;
    };
    
    } // namespace pebblekv
    
#else
    #include <pthread.h>
    #include <atomic>
    
    namespace pebblekv {
    
    class Mutex {
    public:
        Mutex() { pthread_mutex_init(&mutex_, nullptr); }
        ~Mutex() { pthread_mutex_destroy(&mutex_); }
        void lock() { pthread_mutex_lock(&mutex_); }
        void unlock() { pthread_mutex_unlock(&mutex_); }
    private:
        pthread_mutex_t mutex_;
        Mutex(const Mutex&) = delete;
        Mutex& operator=(const Mutex&) = delete;
    };
    
    class LockGuard {
    public:
        explicit LockGuard(Mutex& m) : mutex_(m) { mutex_.lock(); }
        ~LockGuard() { mutex_.unlock(); }
    private:
        Mutex& mutex_;
        LockGuard(const LockGuard&) = delete;
        LockGuard& operator=(const LockGuard&) = delete;
    };
    
    using AtomicBool = std::atomic<bool>;
    
    } // namespace pebblekv
    
#endif
