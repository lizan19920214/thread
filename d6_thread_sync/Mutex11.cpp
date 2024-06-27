/**
 * g++ -o Mutex11 Mutex11.cpp -std=c++11 -lpthread
 * 
 * c++11下的互斥锁和条件变量的使用
 * 
 * #include <mutex>
 * std::mutex
 * std::mutex 对象提供了独占所有权的特性——即不支持递归地对 std::mutex 对象上锁
 * 
 * 成员函数：
 * lock()，调用线程将锁住该互斥量。线程调用该函数会发生下面 3 种情况：
 *      (1). 如果该互斥量当前没有被锁住，则调用线程将该互斥量锁住，直到调用 unlock之前，该线程一直拥有该锁。
 *      (2). 如果当前互斥量被其他线程锁住，则当前的调用线程被阻塞住。
 *      (3). 如果当前互斥量被当前调用线程锁住，则会产生死锁(deadlock)。
 * 
 * unlock()， 解锁，释放对互斥量的所有权。
 * 
 * try_lock()，尝试锁住互斥量，如果互斥量被其他线程占有，则当前线程也不会被阻塞。线程调用该函数也会出现下面 3 种情况，
 *      (1). 如果当前互斥量没有被其他线程占有，则该线程锁住互斥量，直到该线程调用 unlock 释放互斥量。
 *      (2). 如果当前互斥量被其他线程锁住，则当前调用线程返回 false，而并不会被阻塞掉。
 *      (3). 如果当前互斥量被当前调用线程锁住，则会产生死锁(deadlock)。
 * 
*/

#include <iostream>
#include <thread>
#include <mutex>
#include <unistd.h>

volatile int counter(0);
std::mutex mtx;

void increase(int index)
{
    std::cout << "index:" << index << std::endl;
    for (int i = 0; i < 100; i++)
    {
        //强制加锁，如果被其他线程锁住，则当前线程会阻塞，直到互斥量被解锁 结果为1000
        mtx.lock();
        ++ counter;
        std::cout << "index:" << index << " counter:" << counter << std::endl;
        mtx.unlock();

        //尝试加锁，如果被其他线程锁住，返回false，不阻塞，i的这次循环被跳过，结果就是counter的总值会小于1000
        // if(mtx.try_lock())
        // {
        //     ++ counter;
        //     std::cout << "index:" << index << " counter:" << counter << std::endl;
        //     mtx.unlock();
        // }
    }
}

void increaseUnique(int index)
{
    for (int i = 0; i < 100; i++)
    {
        //使用RAII(资源获得就是初始化)方式来加锁，可以避免内存泄漏，它可以保证任何情况下使用对象时先构造对象，最后析构对象
        //这里lck析构时会自动解锁，不用手动调用unlock
        //当然我们也可以手动 lck.unlock()来释放 lck.lock()进行上锁
        std::unique_lock<std::mutex> lck(mtx);
        ++ counter;
        std::cout << "index:" << index << " counter:" << counter << std::endl;
    }
}

int main()
{
    std::thread threads[10];
    for (int i = 0; i < 10; ++i)
    {
        // threads[i] = std::thread(increase, i);
        threads[i] = std::thread(increaseUnique, i);
    }

    for (auto& i : threads)
    {
        i.join();
    }

    std::cout << "main count:" << counter << std::endl;

    return 0;
}