#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>
#include <mutex>

#include "math/vec.h"
#include "scene/bvh.h"
#include "scene/locked_queue.h"
#include "scene/camera.h"
#include "scene/ray_triangle.h"
#include "scene/scene.h"
#include "scene/sceneloader.h"

bool intersect(const tracer::scene &SceneMesh, const tracer::vec3<float> &ori,
			   const tracer::vec3<float> &dir, float &t, float &u, float &v,
	   		   size_t &geomID, size_t &primID){
	for (auto i = 0; i < SceneMesh.geometry.size(); i++) {
		for (auto f = 0; f < SceneMesh.geometry[i].face_index.size(); f++) {
			auto face = SceneMesh.geometry[i].face_index[f];
			if (tracer::intersect_triangle(
					ori, dir, SceneMesh.geometry[i].vertex[face[0]],
					SceneMesh.geometry[i].vertex[face[1]],
					SceneMesh.geometry[i].vertex[face[2]], t, u, v)
			){
				geomID = i;
				primID = f;
			}
		}
	}

	return (geomID != -1 && primID != -1);
}

bool intersectBVH(const tracer::scene &SceneMesh, bvh::BVH bvh, const tracer::vec3<float> &ori,
				  const tracer::vec3<float> &dir, const tracer::vec3<float> &invdir,
				  float &t, float &u, float &v, size_t &geomID, tracer::vec3<unsigned int> &face){
	
	float tmin = std::numeric_limits<float>::max();

	if(bvh.geometry == -1){	
		if(bvh::intersect_box(ori, invdir, bvh.max, bvh.min, tmin) && (tmin < t)){

			tmin = std::numeric_limits<float>::max();
			bool ret1 = bvh::intersect_box(ori, invdir, (bvh.left)->max, (bvh.left)->min, tmin);
			float tmin2 = std::numeric_limits<float>::max();
			bool ret2 = bvh::intersect_box(ori, invdir, (bvh.right)->max, (bvh.right)->min, tmin2);

			if(ret1 && ret2){
				if(tmin < tmin2){
					ret1 = intersectBVH(SceneMesh, *(bvh.left), ori, dir, invdir, t, u, v, geomID, face);
					ret2 = intersectBVH(SceneMesh, *(bvh.right), ori, dir, invdir, t, u, v, geomID, face);
				}
				else{
					ret1 = intersectBVH(SceneMesh, *(bvh.right), ori, dir, invdir, t, u, v, geomID, face);
					ret2 = intersectBVH(SceneMesh, *(bvh.left), ori, dir, invdir, t, u, v, geomID, face);
				}
				return (ret1 || ret2);
			}
			else if(ret1)
				return intersectBVH(SceneMesh, *(bvh.left), ori, dir, invdir, t, u, v, geomID, face);
			else if(ret2)
				return intersectBVH(SceneMesh, *(bvh.right), ori, dir, invdir, t, u, v, geomID, face);
			else
				return false;
		}
		else
			return false;
	}

	else{
		auto i = bvh.geometry;
		auto faceAux = bvh.triangle;
		
		if (tracer::intersect_triangle(
				ori, dir, SceneMesh.geometry[i].vertex[faceAux[0]],
				SceneMesh.geometry[i].vertex[faceAux[1]],
				SceneMesh.geometry[i].vertex[faceAux[2]], t, u, v)
		){
			geomID = i;
			face = faceAux;
		}
		
		return (geomID != -1);
	}
}

bool occlusion(const tracer::scene &SceneMesh, const tracer::vec3<float> &ori,
	   		   const tracer::vec3<float> &dir, float &t){
	float u, v;
	for (auto i = 0; i < SceneMesh.geometry.size(); i++) {
		for (auto f = 0; f < SceneMesh.geometry[i].face_index.size(); f++) {
			auto face = SceneMesh.geometry[i].face_index[f];
			if (tracer::intersect_triangle(
					ori, dir, SceneMesh.geometry[i].vertex[face[0]],
					SceneMesh.geometry[i].vertex[face[1]],
					SceneMesh.geometry[i].vertex[face[2]], t, u, v)
			){
				return true;
			}
		}
	}

	return false;
}

bool occlusionBVH(const tracer::scene &SceneMesh, bvh::BVH bvh, const tracer::vec3<float> &ori,
				  const tracer::vec3<float> &dir, const tracer::vec3<float> &invdir, float &t){
	
	float tmin = std::numeric_limits<float>::max();

	if(bvh.geometry == -1){	
		if(bvh::intersect_box(ori, invdir, bvh.max, bvh.min, tmin)){

			bool ret1 = bvh::intersect_box(ori, invdir, (bvh.left)->max, (bvh.left)->min, tmin);
			bool ret2 = bvh::intersect_box(ori, invdir, (bvh.right)->max, (bvh.right)->min, tmin);

			if(ret1 && ret2){
					if(occlusionBVH(SceneMesh, *(bvh.left), ori, dir, invdir, t))
						return true;
					else
						return occlusionBVH(SceneMesh, *(bvh.right), ori, dir, invdir, t);
			}
			else if(ret1)
				return occlusionBVH(SceneMesh, *(bvh.left), ori, dir, invdir, t);
			else if(ret2)
				return occlusionBVH(SceneMesh, *(bvh.right), ori, dir, invdir, t);
			else
				return false;
		}
		else
			return false;
	}

	else{
		auto i = bvh.geometry;
		auto faceAux = bvh.triangle;
		float u, v;
		
		if (tracer::intersect_triangle(
				ori, dir, SceneMesh.geometry[i].vertex[faceAux[0]],
				SceneMesh.geometry[i].vertex[faceAux[1]],
				SceneMesh.geometry[i].vertex[faceAux[2]], t, u, v)
		){
			return true;
		}
		
		return false;
	}
}

bvh::BVH initializeBVH(tracer::scene SceneMesh, int cores){
	int num_triangles = 0;

	for (auto i = 0; i < SceneMesh.geometry.size(); i++)
		num_triangles += SceneMesh.geometry[i].face_index.size();

	tracer::vec3<unsigned int> *triangles = new tracer::vec3<unsigned int>[num_triangles];
	tracer::vec3<float> *centers = new tracer::vec3<float>[num_triangles];
	tracer::vec3<float> *max = new tracer::vec3<float>[num_triangles];
	tracer::vec3<float> *min = new tracer::vec3<float>[num_triangles];
	std::vector<int> geometries;
	
	int aux = 0;
	for (int i = 0; i < SceneMesh.geometry.size(); i++){
		for (int f = 0; f < SceneMesh.geometry[i].face_index.size(); f++){
			auto face = SceneMesh.geometry[i].face_index[f];
			
			triangles[aux] = face;

			centers[aux][0] = (SceneMesh.geometry[i].vertex[face[0]][0] + SceneMesh.geometry[i].vertex[face[1]][0] + SceneMesh.geometry[i].vertex[face[2]][0])/3.0;
			centers[aux][1] = (SceneMesh.geometry[i].vertex[face[0]][1] + SceneMesh.geometry[i].vertex[face[1]][1] + SceneMesh.geometry[i].vertex[face[2]][1])/3.0;
			centers[aux][2] = (SceneMesh.geometry[i].vertex[face[0]][2] + SceneMesh.geometry[i].vertex[face[1]][2] + SceneMesh.geometry[i].vertex[face[2]][2])/3.0;
			
			min[aux][0] = std::min(SceneMesh.geometry[i].vertex[face[0]][0], 
           	              	  	   std::min(SceneMesh.geometry[i].vertex[face[1]][0],
            	              			    SceneMesh.geometry[i].vertex[face[2]][0]
                        	      ));
	        min[aux][1] = std::min(SceneMesh.geometry[i].vertex[face[0]][1], 
    	                    	   std::min(SceneMesh.geometry[i].vertex[face[1]][1],
        	                                SceneMesh.geometry[i].vertex[face[2]][1]
            	                  ));
	        min[aux][2] = std::min(SceneMesh.geometry[i].vertex[face[0]][2], 
    	                           std::min(SceneMesh.geometry[i].vertex[face[1]][2],
        	                          		SceneMesh.geometry[i].vertex[face[2]][2]
            	                  ));
	        max[aux][0] = std::max(SceneMesh.geometry[i].vertex[face[0]][0], 
    	                           std::max(SceneMesh.geometry[i].vertex[face[1]][0],
        	                          		SceneMesh.geometry[i].vertex[face[2]][0]
            	                  ));
	        max[aux][1] = std::max(SceneMesh.geometry[i].vertex[face[0]][1], 
    	                           std::max(SceneMesh.geometry[i].vertex[face[1]][1],
        	                          		SceneMesh.geometry[i].vertex[face[2]][1]
            	                  ));
	        max[aux][2] = std::max(SceneMesh.geometry[i].vertex[face[0]][2], 
    	                           std::max(SceneMesh.geometry[i].vertex[face[1]][2],
        	                          		SceneMesh.geometry[i].vertex[face[2]][2]
            	                  ));

			geometries.push_back(i);

			aux = aux + 1;
		}
	}

	int maxLevel = 0;
	int i = 1;
	aux = 0;
	bool flag = true;

	while(flag){
		if(aux <= cores){
			maxLevel = i;
			aux = aux + pow(2, i);
			i = i + 1;
		}
		else
			flag = false;
	}

	bvh::BVH bvh(SceneMesh, &triangles, &centers, &geometries, &max, &min, 0, 0, num_triangles-1, 0, maxLevel);

	return bvh;
}

void execution(const int h0, const int hmax, const int w0, const int wmax, const int image_height,
			   const int image_width, tracer::scene SceneMesh, bvh::BVH bvh, tracer::camera cam,
			   tracer::vec3<float> *image, std::uniform_real_distribution<float> distrib){
	
	std::random_device rd;
	std::mt19937 gen(rd());

	for(int h = h0; h < hmax; h++){
		for (int w = w0; w < wmax; w++){
		size_t geomID = -1;
		size_t primID = -1;

		auto is = float(w) / (image_width - 1);
		auto it = float(h) / (image_height - 1);
		auto ray = cam.get_ray(is, it);

		float t = std::numeric_limits<float>::max();
		float u = 0;
		float v = 0;

		tracer::vec3<float> invdir = tracer::vec3<float>(1/ray.dir[0], 1/ray.dir[1], 1/ray.dir[2]);
		tracer::vec3<unsigned int> face = tracer::vec3<unsigned int>(0, 0, 0);
	
		//if (intersect(SceneMesh, ray.origin, ray.dir, t, u, v, geomID, primID)) {
		if (intersectBVH(SceneMesh, bvh, ray.origin, ray.dir, invdir, t, u, v, geomID, face)) {
			
			auto i = geomID;
			auto f = primID;
			//auto face = SceneMesh.geometry[i].face_index[f];
			auto N = normalize(cross(SceneMesh.geometry[i].vertex[face[1]] - SceneMesh.geometry[i].vertex[face[0]],
									 SceneMesh.geometry[i].vertex[face[2]] - SceneMesh.geometry[i].vertex[face[0]]));

			if (!SceneMesh.geometry[i].normals.empty()) {
				auto N0 = SceneMesh.geometry[i].normals[face[0]];
				auto N1 = SceneMesh.geometry[i].normals[face[1]];
				auto N2 = SceneMesh.geometry[i].normals[face[2]];
				N = normalize(N1 * u + N2 * v + N0 * (1 - u - v));
			}

			auto t0 = t;
			for (auto &lightID : SceneMesh.light_sources) {
				auto light = SceneMesh.geometry[lightID];
				light.face_index.size();
				std::uniform_int_distribution<int> distrib1(0, light.face_index.size() - 1);

				int faceID = distrib1(gen);
				const auto &v0 = light.vertex[faceID];
				const auto &v1 = light.vertex[faceID];
				const auto &v2 = light.vertex[faceID];

				auto P = v0 + ((v1 - v0) * float(distrib(gen)) + (v2 - v0) * float(distrib(gen)));

				auto hit = ray.origin + ray.dir * (t0 - float(pow(10, -6)));
				auto L = P - hit;

				auto len = tracer::length(L);

				t0 = len - float(pow(10, -6));

				L = tracer::normalize(L);

				auto mat = SceneMesh.geometry[i].object_material;
				auto c = (mat.ka * 0.5f + mat.ke) / float(SceneMesh.light_sources.size());

				tracer::vec3<float> invL = tracer::vec3<float>(1/L[0], 1/L[1], 1/L[2]);

				if (occlusionBVH(SceneMesh, bvh, hit, L, invL, t0))
				//if (occlusion(SceneMesh, hit, L, t0))
					continue;

				auto d = dot(N, L);

				if (d <= 0)
					continue;

				auto H = normalize((N + L) * 2.f);

				c = c + (mat.kd * d + mat.ks * pow(dot(N, H), mat.Ns)) / float(SceneMesh.light_sources.size());

				image[h * image_width + w].r += c.r;
				image[h * image_width + w].g += c.g;
				image[h * image_width + w].b += c.b;
			}

			auto nSamples = 14;

			auto pos = sqrt(float(1)/float(3));
			auto neg = 0 - pos;
			tracer::vec3<float> *vecs = new tracer::vec3<float>[nSamples];
			vecs[0]  = tracer::vec3<float>(  1,   0,   0);
			vecs[1]  = tracer::vec3<float>( -1,   0,   0);
			vecs[2]  = tracer::vec3<float>(  0,   1,   0);
			vecs[3]  = tracer::vec3<float>(  0,  -1,   0);
			vecs[4]  = tracer::vec3<float>(  0,   0,   1);
			vecs[5]  = tracer::vec3<float>(  0,   0,  -1);
			vecs[6]  = tracer::vec3<float>(pos, pos, pos);
			vecs[7]  = tracer::vec3<float>(pos, pos, neg);
			vecs[8]  = tracer::vec3<float>(pos, neg, pos);
			vecs[9]  = tracer::vec3<float>(pos, neg, neg);
			vecs[10] = tracer::vec3<float>(neg, pos, pos);
			vecs[11] = tracer::vec3<float>(neg, pos, neg);
			vecs[12] = tracer::vec3<float>(neg, neg, pos);
			vecs[13] = tracer::vec3<float>(neg, neg, neg);

			for(auto sample = 0; sample < nSamples; sample++){
				tracer::vec3<float> P = vecs[sample];
				auto hit = ray.origin + ray.dir * (t - float(pow(10, -6)));
				auto L = P - hit;

				auto len = 0.2f;
				t = len - float(pow(10, -6));

				L = tracer::normalize(L);
				auto d = dot(N, L);

				auto mat = SceneMesh.geometry[i].object_material;
				tracer::vec3<float> invL = tracer::vec3<float>(1/L[0], 1/L[1], 1/L[2]);

				//if (occlusion(SceneMesh, hit, L, t))
				if (occlusionBVH(SceneMesh, bvh, hit, L, invL, t))
					continue;

				auto c = (mat.ka * 0.5f)/float(nSamples);

				image[h * image_width + w].r += c.r;
				image[h * image_width + w].g += c.g;
				image[h * image_width + w].b += c.b;
			}
		}
		}
	}
}

void executeTasks(const int image_height, const int image_width, tracer::scene SceneMesh,
			   	  bvh::BVH bvh, tracer::camera cam, tracer::vec3<float> *image, int cores){

  	std::vector<std::future<void>> tasks;
  	std::uniform_real_distribution<float> distrib(0, 1.f);

	for (int h = image_height - 1; h >= 0; --h)
		tasks.push_back(std::async(std::launch::async, execution, h, h+1, 0, image_width, image_height, image_width, SceneMesh, bvh, cam, image, distrib));
		//execution(h, h+1, 0, image_width, image_height, image_width, SceneMesh, bvh, cam, image, distrib);

	for(auto &t : tasks)
		t.get();
}

void executeLockedQueue(const int image_height, const int image_width, tracer::scene SceneMesh,
			   	  		bvh::BVH bvh, tracer::camera cam, tracer::vec3<float> *image, int cores){

  	std::uniform_real_distribution<float> distrib(0, 1.f);
	
	std::vector<tracer::vec4<int>> blocks;

	std::vector<std::thread> threads;

	int blockSizeH = int(ceil((double)image_height/(double)16));
	int blockSizeW = int(ceil((double)image_width/(double)16));
	
	for(int h = 0; h < image_height; h+=blockSizeH){
		for(int w = image_width; w > 0; w-=blockSizeW)
			blocks.push_back(tracer::vec4<int>(h, std::min(h+blockSizeH, image_height), std::max(w-blockSizeW, 0), w));
	}

	lckdq::LockedQueue queue(blocks);

	for(auto i = 0; i < cores; i++){
		threads.push_back(std::thread([&](const int image_height, const int image_width, tracer::scene SceneMesh,
										  bvh::BVH bvh, tracer::camera cam, tracer::vec3<float> *image,
										  std::uniform_real_distribution<float> distrib, lckdq::LockedQueue *queue){
			while(!(queue->isEmpty())){
				auto vec = queue->pop();
				if((vec[0] != -1) && (vec[1] != -1) && (vec[2] != -1) && (vec[3] != -1)){
					execution(vec[0], vec[1], vec[2], vec[3], image_height, image_width, SceneMesh, bvh, cam, image, distrib);
				}
			}
		}, image_height, image_width, SceneMesh, bvh, cam, image, distrib, &queue));
	}

	for(auto &t : threads)
		t.join();
}

int main(int argc, char *argv[]) {
	std::string modelname;
	std::string outputname = "output.ppm";
	bool hasEye{false}, hasLook{false};
	tracer::vec3<float> eye(0, 1, 3), look(0, 1, 0);
	tracer::vec2<uint> windowSize(1024, 768);

	for (int arg = 0; arg < argc; arg++) {
		if (std::string(argv[arg]) == "-m") {
			modelname = std::string(argv[arg + 1]);
			arg++;
			continue;
		}
		if (std::string(argv[arg]) == "-o") {
			outputname = std::string(argv[arg + 1]);
			arg++;
			continue;
		}
		if (std::string(argv[arg]) == "-v") {
			char *token = std::strtok(argv[arg + 1], ",");
			int i = 0;
			while (token != NULL) {
				eye[i++] = atof(token);
				token = std::strtok(NULL, ",");
			}

			if (i != 3)
				throw std::runtime_error("Error parsing view");
			hasEye = true;
			arg++;
			continue;
		}
		if (std::string(argv[arg]) == "-l") {
			char *token = std::strtok(argv[arg + 1], ",");
			int i = 0;

			while (token != NULL) {
				look[i++] = atof(token);
				token = std::strtok(NULL, ",");
			}

			if (i != 3)
				throw std::runtime_error("Error parsing view");
			hasLook = true;
			arg++;
			continue;
		}
		if (std::string(argv[arg]) == "-w") {
			char *token = std::strtok(argv[arg + 1], ",");
			int i = 0;

			while (token != NULL) {
				look[i++] = atof(token);
				token = std::strtok(NULL, ",");
			}

			if (i != 2)
				throw std::runtime_error("Error parsing window size");
			hasLook = true;
			arg++;
			continue;
		}
	}

	tracer::scene SceneMesh;
  	bool ModelLoaded = false;

  	if (modelname != "") {
    	SceneMesh = model::loadobj(modelname);
    	ModelLoaded = true;
  	}

  	int image_width = windowSize.x;
  	int image_height = windowSize.y;

  	tracer::camera cam(eye, look, tracer::vec3<float>(0, 1, 0), 60,
                       float(image_width) / image_height);
  	// Render

  	tracer::vec3<float> *image = new tracer::vec3<float>[image_height * image_width];

	int cores = std::thread::hardware_concurrency();
	//bvh::BVH bvh = initializeBVH(SceneMesh, cores);

  	auto start_time = std::chrono::high_resolution_clock::now();
	
	bvh::BVH bvh = initializeBVH(SceneMesh, cores);

	//executeTasks(image_height, image_width, SceneMesh, bvh, cam, image, cores);
	executeLockedQueue(image_height, image_width, SceneMesh, bvh, cam, image, cores);


	auto end_time = std::chrono::high_resolution_clock::now();

	std::cerr << "\n\n Duration : "
						<< std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time)
									 .count()
						<< std::endl;

	std::ofstream file(outputname, std::ios::out);

	file << "P3\n" << image_width << " " << image_height << "\n255\n";
  	for (int h = image_height - 1; h >= 0; --h) {
    	for (int w = 0; w < image_width; ++w) {
      		auto &img = image[h * image_width + w];
      		img.r = (img.r > 1.f) ? 1.f : img.r;
      		img.g = (img.g > 1.f) ? 1.f : img.g;
      		img.b = (img.b > 1.f) ? 1.f : img.b;

      		file << int(img.r * 255) << " " << int(img.g * 255) << " "
           		 << int(img.b * 255) << "\n";
    	}
  	}
  
  	delete[] image;
  
  	return 0;
}
