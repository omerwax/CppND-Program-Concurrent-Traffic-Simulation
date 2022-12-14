#include <iostream>
#include <random>
#include <chrono>
#include "TrafficLight.h"
#include <thread>

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
    // perform queue modification under the lock
    std::unique_lock<std::mutex> uLock(_mutex);
    _cond.wait(uLock, [this] { return !_messages.empty(); });

    // remove last vector element from queue
    TrafficLightPhase msg = std::move(_messages.back());
    _messages.pop_back();

    return msg; 
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

        // perform queue modification under the lock
        std::lock_guard<std::mutex> uLock(_mutex);

        // First we need to clear to queue, to drop old messages, they cause acting on un-relevant data
        _messages.clear();

        // add the message to queue
        _messages.emplace_back(std::move(msg));
        _cond.notify_one(); // notify client after pushing new Vehicle into vector
}

/* Implementation of class "TrafficLight" */

 
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    
    while (_messageQueue.receive() != TrafficLightPhase::green)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    return;
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method ???cycleThroughPhases??? should be started in a thread when the public method ???simulate??? is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));

}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    auto lastUpdate = std::chrono::system_clock::now();
    double cycleDuration;
    srand(_id);
      

    while (true)
    {
        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // Choose a random value between 4.0 and 6.0 seconds
        std::uniform_int_distribution<int> distribution(0, 2000);

        std::mt19937 mt(_id);

        // cycleDuration = 4000 + (mt() % 2000);
        cycleDuration = 4000 + (1.0 * mt())/mt.max() * 2000.0;

        // compute time difference to stop watch
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();
        
        // Check if cycleDuration has passed
        if (timeSinceLastUpdate >= cycleDuration)
        {
            if (_currentPhase == TrafficLightPhase::red){
                // change phase
                _currentPhase = TrafficLightPhase::green;
            }
            else{
                // change phase
                _currentPhase = TrafficLightPhase::red;
            }
            
            // Push the new phase to the queue
            _messageQueue.send(std::move(_currentPhase));
            // Update the last update time
            lastUpdate = std::chrono::system_clock::now();

            // Debug print
            // std::lock_guard<std::mutex> lock(_mtx);
            // std::cout << "traffic-light # " << _id << ": state is changed to: " << (_currentPhase == TrafficLightPhase::red ? "red" : "green") << std::endl;
            // std::cout << "traffic-light # " << _id << ": CycleDurtion = : " << cycleDuration << std::endl;
        
        }
    }


}

