#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stop_token>
#include <thread>
#include <utility>
#include <vector>

class ThreadPool {
    std::vector<std::jthread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex mtx;
    std::condition_variable cv;
    bool stop = false;

public:
    ThreadPool(int n) {
        for (int i = 0; i < n; ++i) {
            workers.emplace_back([this](std::stop_token st) {
                while (!st.stop_requested()) {
                    std::function<void()> task;
                    {
                        std::unique_lock lock(mtx);
                        cv.wait(lock, [&] { return !tasks.empty() || st.stop_requested(); });

                        if (st.stop_requested() && tasks.empty()) return;
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
            }); 
        }
    }

    template<typename F>
    auto submit(F&& f) -> std::future<decltype(f())> {
        auto task = std::make_shared<std::packaged_task<decltype(f())()>>(std::forward<F>(f));
        auto fut = task->get_future();
        {
            std::lock_guard lock(mtx);
            tasks.push([task] { (*task)(); });
        }
        cv.notify_one();
        return fut;
    }

    ~ThreadPool() {
        for (auto& w : workers) w.request_stop();
        cv.notify_all();
    }
};