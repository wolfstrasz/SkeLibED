#pragma once
#ifndef _TEST_NBODY_HPP
#define _TEST_NBODY_HPP


#include "Map.hpp"
#include "DynamicMap1.hpp"
#include "DynamicMap2.hpp"
#include "DynamicMap5.hpp"


#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>

namespace nbody {
#define nbody_test_count 1

	const float G = 1.0;
	const float delta_t = 0.1;
	size_t threads;
	size_t blocks;
	size_t iterations;
	size_t particle_num;
	// Particle data structure that is used as an element type.
	struct Particle {
		float x, y, z;
		float vx, vy, vz;
		float m;

		// Operator overtide for !=
		bool operator !=(const Particle& a) const
		{
			return (x != a.x || y != a.y || z != a.z) ||
				(vx != a.vx || vy != a.vy || vz != a.vz) || ( m != a.m);
		}


	};

	/*
 * Array user-function that is used for applying nbody computation,
 * All elements from parr and a single element (named 'pi') are accessible
 * to produce one output element of the same type.
 */
	Particle move(size_t index, const std::vector<Particle> parr) {

		size_t i = index;
		size_t np = parr.size();

		Particle pi = parr[i];

		float ax = 0.0, ay = 0.0, az = 0.0;

		for (size_t j = 0; j < np; ++j)
		{
			if (i != j)
			{
				Particle pj = parr[j];

				float rij = sqrt((pi.x - pj.x) * (pi.x - pj.x)
					+ (pi.y - pj.y) * (pi.y - pj.y)
					+ (pi.z - pj.z) * (pi.z - pj.z));

				float dum = G * pi.m * pj.m / pow(rij, 3);

				ax += dum * (pi.x - pj.x);
				ay += dum * (pi.y - pj.y);
				az += dum * (pi.z - pj.z);
			}
		}

		pi.x += delta_t * pi.vx + delta_t * delta_t / 2 * ax;
		pi.y += delta_t * pi.vy + delta_t * delta_t / 2 * ay;
		pi.z += delta_t * pi.vz + delta_t * delta_t / 2 * az;

		pi.vx += delta_t * ax;
		pi.vy += delta_t * ay;
		pi.vz += delta_t * az;

		return pi;
	}

	// Generate user-function that is used for initializing particles array.
	Particle init(size_t index, size_t np)
	{
		int s = index;
		int d = np / 2 + 1;
		int i = s % np;
		int j = ((s - i) / np) % np;
		int k = (((s - i) / np) - j) / np;

		Particle p;

		p.x = i - d + 1;
		p.y = j - d + 1;
		p.z = k - d + 1;

		p.vx = 0.0;
		p.vy = 0.0;
		p.vz = 0.0;

		p.m = 1;

		return p;
	}

	// Function of Static map
	void snbody(std::vector<Particle> &particles) {

		size_t np = particles.size();
		std::vector<Particle> doublebuffer(particles.size());

		std::vector<size_t> indices(particles.size());

		auto nbody_init = Map(init, threads, blocks);
		auto nbody_simulate_step = Map(move, threads, blocks);

		// initialization of indices vector
		for (size_t i = 0; i < particles.size(); i++) {
			indices[i] = i;
		}
		// particle vectors initialization
		nbody_init(particles, indices, np);

		// std::cout<<" SIZES " << np << " : " << iterations << std::endl;

		for (size_t i = 0; i < iterations; i += 2) {
		//	std::cout << " ITER " << i << std::endl;

			nbody_simulate_step(doublebuffer, indices, particles);
		//	std::cout << " ITER " << i+1 << std::endl;

			nbody_simulate_step(particles, indices, doublebuffer);
		}
	//	std::cout << " SIZES " << np << " : " << iterations << std::endl;

	}
	// Function of Dynamic map
	void dnbody2(std::vector<Particle> &particles) {

		size_t np = particles.size();
		std::vector<Particle> doublebuffer(particles.size());

		std::vector<size_t> indices(particles.size());
		size_t chunkSize = particle_num / (blocks * threads);

		auto nbody_init = DynamicMap2(init, threads, chunkSize);
		auto nbody_simulate_step = DynamicMap2(move, threads, chunkSize);

		// initialization of indices vector
		for (size_t i = 0; i < particles.size(); i++) {
			indices[i] = i;
		}

		// particle vectors initialization
		nbody_init(particles, indices, np);

		// simulation
		for (size_t i = 0; i < iterations; i += 2) {
			nbody_simulate_step(doublebuffer, indices, particles);
			nbody_simulate_step(particles, indices, doublebuffer);
		}
	}

	// Function of Dynamic map
	void dnbody1(std::vector<Particle> &particles) {

		size_t np = particles.size();
		std::vector<Particle> doublebuffer(particles.size());

		std::vector<size_t> indices(particles.size());
		size_t chunkSize = particle_num / (blocks * threads);

		auto nbody_init = DynamicMap1(init, threads, chunkSize);
		auto nbody_simulate_step = DynamicMap1(move, threads, chunkSize);

		// initialization of indices vector
		for (size_t i = 0; i < particles.size(); i++) {
			indices[i] = i;
		}

		// particle vectors initialization
		nbody_init(particles, indices, np);

		// simulation
		for (size_t i = 0; i < iterations; i += 2) {
			nbody_simulate_step(doublebuffer, indices, particles);
			nbody_simulate_step(particles, indices, doublebuffer);
		}
	}

	// Function of Dynamic map
	void dnbody5(std::vector<Particle> &particles) {

		size_t np = particles.size();
		std::vector<Particle> doublebuffer(particles.size());

		std::vector<size_t> indices(particles.size());
		size_t chunkSize = particle_num / (blocks * threads, chunkSize);

		auto nbody_init = DynamicMap5(init, threads);
		auto nbody_simulate_step = DynamicMap5(move, threads, chunkSize);

		// initialization of indices vector
		for (size_t i = 0; i < particles.size(); i++) {
			indices[i] = i;
		}

		// particle vectors initialization
		nbody_init(particles, indices, np);

		// simulation
		for (size_t i = 0; i < iterations; i += 2) {
			nbody_simulate_step(doublebuffer, indices, particles);
			nbody_simulate_step(particles, indices, doublebuffer);
		}
	}


	void test(size_t threadcount, size_t blockcount, size_t np, size_t iters) {
		particle_num = np;
		// initialisation
		threads = threadcount;
		blocks = blockcount;
		iterations = iters;
		std::vector<Particle> sParticles(np);
		std::vector<Particle> dParticles = sParticles;

		std::chrono::duration<double, std::milli> time;
		auto start = std::chrono::system_clock::now();
		time = start - start;


		// test static map
		// -------------------------------------------
		for (int t = 0; t < nbody_test_count; t++) {
			std::cout << "SMAP TEST:" << t << "\n";
			auto start = std::chrono::system_clock::now();
			snbody(sParticles);
			auto end = std::chrono::system_clock::now();
			time += end - start;
		}
		std::cout << "SMAP: " << std::to_string(time.count()/nbody_test_count) << std::endl;
		time = start - start;


		time = start - start;
		// test dynamic map
		// ----------------------------------------------
		for (int t = 0; t < nbody_test_count; t++) {
			std::cout << "DMAP1 TEST:" << t << "\n";
			auto start = std::chrono::system_clock::now();
			dnbody1(sParticles);
			auto end = std::chrono::system_clock::now();
			time += end - start;

		}
		std::cout << "DMAP1: " << std::to_string(time.count() / nbody_test_count) << std::endl;
		time = start - start;

		// test dynamic map
		// ----------------------------------------------
		for (int t = 0; t < nbody_test_count; t++) {
			std::cout << "DMAP2 TEST:" << t << "\n";
			auto start = std::chrono::system_clock::now();
			dnbody2(sParticles);
			auto end = std::chrono::system_clock::now();
			time += end - start;

		}
		std::cout << "DMAP2: " << std::to_string(time.count() / nbody_test_count) << std::endl;
		time = start - start;


		// test dynamic map
		// ----------------------------------------------
		for (int t = 0; t < nbody_test_count; t++) {
			std::cout << "DMAP5 TEST:" << t << "\n";
			auto start = std::chrono::system_clock::now();
			dnbody5(sParticles);
			auto end = std::chrono::system_clock::now();
			time += end - start;

		}
		std::cout << "DMAP5: " << std::to_string(time.count() / nbody_test_count) << std::endl;

	}

}



#endif
