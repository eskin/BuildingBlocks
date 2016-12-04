#include <memory>
#include <mutex>


template<typename T>
class ThreadsafeQueue
{
public:
    ThreadsafeQueue() :
        head(std::make_unique<node>()), tail(head.get()) { }
    ThreadsafeQueue(const ThreadsafeQueue &other) = delete;
    ThreadsafeQueue &operator=(const ThreadsafeQueue &other) = delete;

    std::shared_ptr<T> try_pop()
    {
        std::unique_ptr<node> old_head = pop_head();
        return old_head ? old_head->data : std::shared_ptr<T>();
    }

    void push(T new_value)
    {
        auto new_data = std::make_shared<T>(std::move(new_value));
        auto empty_tail = std::make_unique<node>();
        node *const new_tail = empty_tail.get();
        std::lock_guard<std::mutex> tail_lock(tail_mutex);
        tail->data = new_data;
        tail->next = std::move(empty_tail);
        tail = new_tail;
    }

private:
    struct node {
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
    };

    std::mutex head_mutex;
    std::unique_ptr<node> head;
    std::mutex tail_mutex;
    node *tail;

    node *get_tail()
    {
        std::lock_guard<std::mutex> tail_lock(tail_mutex);
        return tail;
    }

    std::unique_ptr<node> pop_head()
    {
        std::lock_guard<std::mutex> head_lock(head_mutex);
        if (head.get() == get_tail()) {
            return nullptr;
        }
        auto old_head = std::move(head);
        head = std::move(old_head->next);
        return old_head;
    }
};


#include <iostream>

int main()
{
    // TODO: threaded example
    ThreadsafeQueue<int> queue;
    queue.push(1);
    queue.push(2);
    queue.push(3);

    for (int i = 0; i < 5; ++i) {
        auto value = queue.try_pop();
        if (value) {
            std::cout << *value << std::endl;
        }
        else {
            std::cout << "no value in queue" << std::endl;
        }
    }
}
