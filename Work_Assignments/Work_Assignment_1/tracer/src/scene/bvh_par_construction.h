#pragma once

#include <vector>
#include <future>
#include <thread>
#include "scene.h"
#include "../math/vec.h"

namespace bvh {
    struct BVH {
        //In case it is box with tho sub-boxes
        tracer::vec3<float> min;
        tracer::vec3<float> max;
        struct BVH *left, *right;

        //In case it is a final triangle
        tracer::vec3<unsigned int> triangle;
        int geometry;

        /*order -> the axies from which the triangles will be ordered (0 = x, 1 = y, 2 = z);
          begin -> the first position to be read
          end   -> the last position to be read*/
        BVH(tracer::scene SceneMesh, tracer::vec3<unsigned int> **triangles, tracer::vec3<float> **centers,
            std::vector<int> *geometries, tracer::vec3<float> **maxT, tracer::vec3<float> **minT,
            int order, int begin, int end, int level, int maxLevel){
            
            min = tracer::vec3<float>(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
            max = tracer::vec3<float>(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
            left = NULL;
            right = NULL;
            triangle = tracer::vec3<unsigned int>(std::numeric_limits<unsigned int>::min(), std::numeric_limits<unsigned int>::min(), std::numeric_limits<unsigned int>::min());
            geometry = -1;

            if(begin == end){
                triangle = (*triangles)[begin];
                geometry = (*geometries)[begin];
                min = (*minT)[begin];
                max = (*maxT)[begin];
            }

            else if(begin < end){
                for(auto i = begin; i <= end; i++){
                    if((*maxT)[i][0] > max[0])
                        max[0] = (*maxT)[i][0];
                    if((*maxT)[i][1] > max[1])
                        max[1] = (*maxT)[i][1];
                    if((*maxT)[i][2] > max[2])
                        max[2] = (*maxT)[i][2];
                    if((*minT)[i][0] < min[0])
                        min[0] = (*minT)[i][0];
                    if((*minT)[i][1] < min[1])
                        min[1] = (*minT)[i][1];
                    if((*minT)[i][2] < min[2])
                        min[2] = (*minT)[i][2];
                }

                for(auto i = begin; i <= end; i++){
                    for(auto j = begin; j <= end-i; j++){
                        if((*centers)[i][order] > (*centers)[j][order]){
                            auto aux1 = (*triangles)[i];
                            (*triangles)[i] = (*triangles)[j];
                            (*triangles)[j] = aux1;

                            auto aux2 = (*centers)[i];
                            (*centers)[i] = (*centers)[j];
                            (*centers)[j] = aux2;

                            auto aux3 = (*geometries)[i];
                            (*geometries)[i] = (*geometries)[j];
                            (*geometries)[j] = aux3;

                            auto aux4 = (*maxT)[i];
                            (*maxT)[i] = (*maxT)[j];
                            (*maxT)[j] = aux4;

                            auto aux5 = (*minT)[i];
                            (*minT)[i] = (*minT)[j];
                            (*minT)[j] = aux5;
                        }
                    }
                }

                auto newOrder = (order+1)%3;
                auto newEnd = (begin+end)/2;
                auto newBegin = newEnd + 1;

                if(level < maxLevel){
                    std::vector<std::thread> threads;

                    threads.push_back(std::thread([&] (tracer::scene SceneMesh, tracer::vec3<unsigned int> **triangles,
                                                       tracer::vec3<float> **centers, std::vector<int> *geometries,
                                                       tracer::vec3<float> **maxT, tracer::vec3<float> **minT,
                                                       int newOrder, int begin, int newEnd, int level, int maxLevel) -> void{
                        left  = new bvh::BVH(SceneMesh, triangles, centers, geometries, maxT, minT, newOrder, begin, newEnd, level, maxLevel);
                    }, SceneMesh, triangles, centers, geometries, maxT, minT, newOrder, begin, newEnd, level+1, maxLevel));

                    threads.push_back(std::thread([&] (tracer::scene SceneMesh, tracer::vec3<unsigned int> **triangles,
                                                       tracer::vec3<float> **centers, std::vector<int> *geometries,
                                                       tracer::vec3<float> **maxT, tracer::vec3<float> **minT,
                                                       int newOrder, int begin, int newEnd, int level, int maxLevel) -> void{
                        right = new bvh::BVH(SceneMesh, triangles, centers, geometries, maxT, minT, newOrder, newBegin, end, level, maxLevel);
                    }, SceneMesh, triangles, centers, geometries, maxT, minT, newOrder, newBegin, end, level+1, maxLevel));
                
                    for(auto &t : threads)
		                t.join();
                }
                else{
                    left  = new bvh::BVH(SceneMesh, triangles, centers, geometries, maxT, minT, newOrder, begin, newEnd, level+1, maxLevel);
                    right = new bvh::BVH(SceneMesh, triangles, centers, geometries, maxT, minT, newOrder, newBegin, end, level+1, maxLevel);
                }
            }
        }
    };

    bool intersect_box(tracer::vec3<float> orig, tracer::vec3<float> invdir,
                       tracer::vec3<float> max, tracer::vec3<float> min, float &t){
        
        float tmin, tmax, tminAux, tmaxAux;

        if(invdir[0] >= 0){
            tmin = (min[0] - orig[0]) * invdir[0];
            tmax = (max[0] - orig[0]) * invdir[0];
        }
        else{
            tmin = (max[0] - orig[0]) * invdir[0];
            tmax = (min[0] - orig[0]) * invdir[0];
        }

        if(invdir[1] >= 0){
            tminAux = (min[1] - orig[1]) * invdir[1];
            tmaxAux = (max[1] - orig[1]) * invdir[1];
        }
        else{
            tminAux = (max[1] - orig[1]) * invdir[1];
            tmaxAux = (min[1] - orig[1]) * invdir[1];
        }


        if((tmin > tmaxAux) || (tmax < tminAux))
            return false;
        
        if(tminAux > tmin)
            tmin = tminAux;
        
        if(tmaxAux < tmax)
            tmax = tmaxAux;


        if(invdir[2] >= 0){
            tminAux = (min[2] - orig[2]) * invdir[2];
            tmaxAux = (max[2] - orig[2]) * invdir[2];
        }
        else{
            tminAux = (max[2] - orig[2]) * invdir[2];
            tmaxAux = (min[2] - orig[2]) * invdir[2];
        }

        if((tmin > tmaxAux) || (tmax < tminAux))
            return false;
        
        t = tmin;
        
        return true;
    }
}