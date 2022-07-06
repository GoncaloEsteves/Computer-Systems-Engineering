#pragma once

#include <vector>
#include <mutex>
#include "../math/vec.h"

namespace lckdq {
    struct LockedQueue {
        //Queue where all information will be stored
        std::vector<tracer::vec4<int>> queue;
        //Number of elements in queue
        size_t size;
        //Mutex
	    std::mutex mut;

        LockedQueue(std::vector<tracer::vec4<int>> toQueue){
            size = toQueue.size();
            for(auto i = 0; i < size; i++)
                queue.push_back(toQueue.at(i));
        }

        bool isEmpty(){
            std::lock_guard<std::mutex> lock(mut);
            
            return (size <= 0);
        }

        tracer::vec4<int> pop(){
            std::lock_guard<std::mutex> lock(mut);
            
            tracer::vec4<int> ret;

            if(size > 0){
                size = size - 1;
                ret = queue.at(size);
				queue.pop_back();
            }
            else
                ret = tracer::vec4<int>(-1, -1, -1, -1);
            
            return ret;
        }
    };
}