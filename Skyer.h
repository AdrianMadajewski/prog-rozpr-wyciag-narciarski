#ifndef SKYER_H
#define SKYER_H

#include <vector>
#include <algorithm>
#include <mutex>
#include <memory>   
#include <unistd.h> // for sleep()
#include <mpi.h>
#include <iomanip>
#include <thread>

struct Data
{
    int ID;
    int clock;
    int weight;
};

class Skyer
{
public:
    Data m_data;
    Skyer();
    void mainActivity();

private:
    // 2 threads per skier
    std::thread requestThread;
    std::thread releaseThread;
    
    // Mutexes
    std::mutex queueMutex;
    std::mutex clockMutex;
    std::mutex releaseMutex;

    // Functionality
    std::vector<Data> queue;
    void sortQueue();
    void addQueue(const Data &data);
    void deleteQueue(int id);
    int countAvailableWeight();
    void waitForSkiLift();
    void updateClock();
    void checkClock(const Data& recv_data);
    void localSleep();
    void requestForSkiLift();
    void waitForConfirm();
    void criticalSection();
    void releaseSkiLift();
};

#endif