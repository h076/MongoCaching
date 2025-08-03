#ifndef TSQUEUE_H_
#define TSQUEUE_H_

// Taken from https://github.com/h076/asioNetworking/blob/main/networking/src/net_tsQueue.hpp
// Slightly changed all "const T&" to "T&&"

// thread safe network queue
// Must be thread safe as at any one time multiple objects may want to alter the queue
// such as the client interface queue being accesed by the connection object and the client
// object iself

#include <condition_variable>
#include <mutex>
#include <deque>

namespace hjw {

    namespace utils {

        template <typename T>
        class tsqueue {
            public:
                tsqueue() = default;
                // prevent the object to be copied
                tsqueue(const tsqueue<T>&) = delete;
                virtual ~tsqueue() {clear();}

            public:

                // Return and maintain item at the front of the queue
                T&& front() {
                    std::scoped_lock lock(m_oMuxQueue);
                    return m_oDeqQueue.front();
                }

                // return and maintain item at the back of the queue
                T&& back() {
                    std::scoped_lock lock(m_oMuxQueue);
                    return m_oDeqQueue.back();
                }

                // Add item to back of queue
                void push_back(T&& item) {
                    std::scoped_lock lock(m_oMuxQueue);
                    m_oDeqQueue.emplace_back(std::move(item));

                    // signal condition variable to wake up
                    // unlocking the mutex protecting m_cvBlocking
                    std::unique_lock<std::mutex> ul(m_oMuxBlocking);
                    m_cvBlocking.notify_one();
                }

                // Add item to the front of queue
                void push_front(T&& item) {
                    std::scoped_lock lock(m_oMuxQueue);
                    m_oDeqQueue.emplace_front(std::move(item));

                    // signal conditon varaible to wake up
                    std::unique_lock<std::mutex> ul(m_oMuxBlocking);
                    m_cvBlocking.notify_one();
                }

                // Is queue empty
                bool empty() {
                    std::scoped_lock lock(m_oMuxQueue);
                    return m_oDeqQueue.empty();
                }

                // Return size of queue
                size_t count() {
                    std::scoped_lock lock(m_oMuxQueue);
                    return m_oDeqQueue.size();
                }

                // Clear the queue
                void clear() {
                    std::scoped_lock lock(m_oMuxQueue);
                    m_oDeqQueue.clear();
                }

                // Return front item in queue
                T pop_front() {
                    std::scoped_lock lock(m_oMuxQueue);
                    auto t = std::move(m_oDeqQueue.front());
                    m_oDeqQueue.pop_front();
                    return t;
                }

                // Return back item in queue
                T pop_back() {
                    std::scoped_lock lock(m_oMuxQueue);
                    auto t = std::move(m_oDeqQueue.back());
                    m_oDeqQueue.pop_back();
                    return t;
                }

                void wait() {
                    while(empty()) {
                        // we can use the condition variable to block the server
                        // Then we can unlock the mutex to wake up the server
                        std::unique_lock<std::mutex> ul(m_oMuxBlocking);
                        m_cvBlocking.wait(ul);
                    }
                }

            protected:
                std::mutex m_oMuxQueue;
                // double ended queue used as our underlying data structure
                std::deque<T> m_oDeqQueue;

                std::condition_variable m_cvBlocking;
                std::mutex m_oMuxBlocking;
        };
    }
}


#endif // TSQUEUE_H_
