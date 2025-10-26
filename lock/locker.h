#ifndef LOCKER_H
#define LOCKER_H

#include <mutex>
#include <condition_variable>
#include <semaphore.h>
#include <exception>

// 信号量封装（依旧使用 POSIX 信号量，因为 C++20 才有 std::counting_semaphore）
class sem
{
public:
    sem()
    {
        if (sem_init(&m_sem, 0, 0) != 0)
        {
            throw std::exception();
        }
    }
    sem(int num)
    {
        if (sem_init(&m_sem, 0, num) != 0)
        {
            throw std::exception();
        }
    }
    ~sem()
    {
        sem_destroy(&m_sem);
    }
    bool wait()
    {
        return sem_wait(&m_sem) == 0;
    }
    bool post()
    {
        return sem_post(&m_sem) == 0;
    }

private:
    sem_t m_sem;
};

// 互斥锁封装：使用 std::mutex
class locker
{
public:
    locker() = default;
    ~locker() = default;

    bool lock()
    {
        m_mutex.lock();
        return true;
    }
    bool unlock()
    {
        m_mutex.unlock();
        return true;
    }
    std::mutex* get()
    {
        return &m_mutex;
    }

private:
    std::mutex m_mutex;
};

// 条件变量封装：使用 std::condition_variable
class cond
{
public:
    cond() = default;
    ~cond() = default;

    bool wait(std::mutex *mtx)
    {
        std::unique_lock<std::mutex> lock(*mtx, std::adopt_lock);
        m_cond.wait(lock);
        lock.release(); // 不让 unique_lock 析构时再次解锁
        return true;
    }
    bool timewait(std::mutex *mtx, struct timespec t)
    {
        std::unique_lock<std::mutex> lock(*mtx, std::adopt_lock);
        auto timeout = std::chrono::seconds(t.tv_sec) + std::chrono::nanoseconds(t.tv_nsec);
        auto result = m_cond.wait_for(lock, timeout);
        lock.release();
        return result != std::cv_status::timeout;
    }
    bool signal()
    {
        m_cond.notify_one();
        return true;
    }
    bool broadcast()
    {
        m_cond.notify_all();
        return true;
    }

private:
    std::condition_variable m_cond;
};

#endif
