/**
 * 
 * g++ -o Condition11 Condition11.cpp -lpthread -std=c++11
 * 
 * #include <condition_variable>
 * std::condition_variable 是 C++11 多线程编程中的条件变量。
 * 
 * 当 std::condition_variable 对象的某个 wait 类函数被调用的时候，它使用 std::unique_lock（通过 std::mutex）来锁住当前的线程，
 * 当前的线程会被系统调入 blocking thread 队列，一直被阻塞（进入睡眠等待状态），直到有其他的线程在同一个 std::condition_variable 
 * 对象上调用 notify 等相关函数来唤醒它。在此阻塞过程中，wait 会释放所在线程持有的 mutex 锁。
 * 
 * std::condition_variable 对象通常使用 std::unique_lock<std::mutex> 来等待
 * 
*/

#include <condition_variable>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>

std::mutex mtx;
std::condition_variable cv;
//全局标志位
bool ready = false;

void PrintId(int id)
{
    std::unique_lock<std::mutex> lck(mtx);
    while(!ready)
    {
        //线程阻塞等待
        cv.wait(lck);
    }

    std::cout << "Id: " << id << " tid:" << std::this_thread::get_id() << std::endl;
}

void go()
{
    std::unique_lock<std::mutex> lck(mtx);
    //改变全局标志位
    ready = true;
    //唤醒所有等待线程
    cv.notify_all();
    //唤醒一个等待线程
    // cv.notify_one();
}

int main()
{
    std::thread threads[10];

    for (int i = 0; i < 10; i++)
    {
        threads[i] = std::thread(PrintId, i);
    }

    std::cout << "create thread done" << std::endl;

    go();

    for (auto& i : threads)
    {
        i.join();
    }

    std::cout << "all threads done" << std::endl;
    
    return 0;
}
