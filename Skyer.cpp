#include "Skyer.h"
#include "Utility.h"

#include <algorithm>

// Set threads
Skyer::Skyer()
{
    releaseThread = std::thread([&](){
            int recv_ID;
            MPI_Status status;

            while (true)
            {
                MPI_Recv(&recv_ID, 1, MPI_INT, MPI_ANY_SOURCE, RELEASE, MPI_COMM_WORLD, &status);

                // Atomic
                updateClock();
                deleteQueue(recv_ID);

                releaseMutex.unlock();
            }
    });

    requestThread = std::thread([&](){
            int success = 1;
            int data[3];
            MPI_Status status;

            while(true)
            {
                MPI_Recv(&data, 3, MPI_INT, MPI_ANY_SOURCE, REQUEST, MPI_COMM_WORLD, &status);
                Data recv_data;
                recv_data.ID = data[0];
                recv_data.clock = data[1];
                recv_data.weight = data[2];

                // Atomic
                checkClock(recv_data);
                addQueue(recv_data);

                // Send back confirmation message
                MPI_Send(&success, 1, MPI_INT, recv_data.ID, REPLY, MPI_COMM_WORLD);
            } 
    });
}

void Skyer::checkClock(const Data& recv_data)
{
    #ifdef DEBUG
        std::cout << m_data.ID << ": checkClock() with " << recv_data.ID << std::endl;
    #endif 

    clockMutex.lock();
    m_data.clock = std::max(m_data.clock, recv_data.clock) + 1;
    clockMutex.unlock();
}

void Skyer::sortQueue()
{
    #ifdef DEBUG
        std::cout << m_data.ID << ": sortQueue()" <<  std::endl;
    #endif 
    std::sort(queue.begin(), queue.end(), [](const Data &s1, const Data &s2)
    {
         if(s1.clock < s2.clock)
            return true;

        return (s1.clock == s2.clock && s1.ID < s2.ID);
    });
}

void Skyer::addQueue(const Data &data)
{
    #ifdef DEBUG
        std::cout << m_data.ID << ": addQueue() added " << data.ID << std::endl;
    #endif 

    queueMutex.lock();
    queue.emplace_back(data);
    queueMutex.unlock();
    sortQueue();
}

void Skyer::deleteQueue(int ID)
{
    #ifdef DEBUG
        std::cout << m_data.ID << ": deleteQueue() removed " << ID << std::endl;
    #endif 

    queueMutex.lock();
    auto lambda = [ID](const Data &a){ return a.ID == ID; };
    auto remove = std::remove_if(queue.begin(), queue.end(), lambda);
    queue.erase(remove, queue.end());
    queueMutex.unlock();
    sortQueue();
}

int Skyer::countAvailableWeight()
{
    int availableWeight = MAX_SKI_LIFT;
    sortQueue();
    for (const auto &data : queue)
    {
        if (data.ID == m_data.ID)
            break;

        availableWeight -= data.weight;
    }
    return availableWeight;
}

void Skyer::waitForSkiLift()
{
    #ifdef DEBUG
        std::cout << m_data.ID << ": waitForSkiLift()" << std::endl;
    #endif 
    int availableWeight = countAvailableWeight();
    while (m_data.weight > availableWeight)
    {
        // Waiting
        releaseMutex.lock();
        availableWeight = countAvailableWeight();
    }
    releaseMutex.unlock();
}

void Skyer::updateClock()
{
    #ifdef DEBUG
        std::cout << m_data.ID << ": updateClock()" << std::endl;
    #endif 
    clockMutex.lock();
    m_data.clock += 1;
    clockMutex.unlock();
}

void Skyer::localSleep()
{
    #ifdef DEBUG
        std::cout << m_data.ID << ": localSleep()" << std::endl;
    #endif 
    int sleepTime = rand() % MAX_SLEEP_TIME + MIN_SLEEP_TIME;
    sleep(sleepTime);
}

void Skyer::requestForSkiLift()
{
    #ifdef DEBUG
        std::cout << m_data.ID << ": requestForSkiLift()" << std::endl;
    #endif 

    // Dummy variable to store in-out m_data information
    int data[3];
    data[0] = m_data.ID;
    data[1] = m_data.clock;
    data[2] = m_data.weight;

    // Send request to all but yourself
    for (int process_rank = 0; process_rank < MAX_SKYERS; ++process_rank)
    {
        if (process_rank != m_data.ID)
        {
            MPI_Send(&data, 3, MPI_INT, process_rank, REQUEST, MPI_COMM_WORLD);
        }
    }
}

void Skyer::criticalSection()
{
    #ifdef DEBUG
        std::cout << m_data.ID << ": criticalSection()" << std::endl;
    #endif 

    int sleepTime = rand() % MAX_SLEEP_TIME + MIN_SLEEP_TIME;
    std::cout << "Skyer [ID = " << std::setw(3) << m_data.ID << ", weight = " << std::setw(3) << m_data.weight << "] rides ski lift for " << sleepTime << " [s]." << std::endl;
    sleep(sleepTime);
}

void Skyer::releaseSkiLift()
{
    #ifdef DEBUG
        std::cout << m_data.ID << ": releaseSkiLift()" << std::endl;
    #endif 

    for (int process_rank = 0; process_rank < MAX_SKYERS; ++process_rank)
    {
        if (process_rank != m_data.ID)
        {
            MPI_Send(&m_data.ID, 1, MPI_INT, process_rank, RELEASE, MPI_COMM_WORLD);
        }
    }
    std::cout << "Skyer [ID = " << std::setw(3) << m_data.ID << ", weight = " << std::setw(3) << m_data.weight << "] quits ski lift." << std::endl;
}

void Skyer::waitForConfirm()
{
    #ifdef DEBUG
        std::cout << m_data.ID << ": waitForConfirm()" << std::endl;
    #endif 
   
    MPI_Status status;
    int success;
    for (int process_rank = 0; process_rank < MAX_SKYERS; ++process_rank)
    {
        if (process_rank != m_data.ID)
        {
            MPI_Recv(&success, 1, MPI_INT, process_rank, REPLY, MPI_COMM_WORLD, &status);
        }
    }
}

void Skyer::mainActivity()
{
    while (true)
    {
        // Local section
        localSleep();
        updateClock();
        
        // Send request to other skyers for ski lift place
        requestForSkiLift();

        // Add to local queue your request
        addQueue({m_data.ID, m_data.clock, m_data.weight});

        // Wait for confirmation from other S-1 skiers
        waitForConfirm();

        // Skyer waits for available ski lift
        waitForSkiLift();

        // Critical section - skyer enters ski lift
        criticalSection();

        // Skyer quits ski lift - sends RELEASE signal to others skyers
        releaseSkiLift();

        // Delete your request from the queue
        deleteQueue(m_data.ID);
    }
}