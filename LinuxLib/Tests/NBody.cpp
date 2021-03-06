#include "NBody.hpp"

#include <iostream>
#include <chrono>
#include <fstream>
#include <string>
#include <cmath>
#include <vector>

#include "../Skeletons/Map.hpp"
#include "../Skeletons/DynamicMap.hpp"

namespace nbody {

    bool Particle::operator !=(const Particle& a) const
		{
			return (x != a.x || y != a.y || z != a.z) ||
				(vx != a.vx || vy != a.vy || vz != a.vz) || ( m != a.m);
		}

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

    void snbody(std::vector<Particle> &particles, int threads, int blocks, int iterations) {

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

		// iterative simulation
		for (size_t i = 0; i < iterations; i += 2) {
			nbody_simulate_step(doublebuffer, indices, particles);
			nbody_simulate_step(particles, indices, doublebuffer);
		}
	}

    void dnbody(std::vector<Particle> &particles, int threads, int blocks, int iterations) {

		size_t np = particles.size();
		std::vector<Particle> doublebuffer(particles.size());
		std::vector<size_t> indices(particles.size());

		auto nbody_init = DynamicMap(init, threads);
		auto nbody_simulate_step = DynamicMap(move, threads);

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

		// initialisation
		std::vector<Particle> sParticles(np);
		std::vector<Particle> dParticles = sParticles;
		std::chrono::duration<double, std::milli> time;

		// test static map
		// -------------------------------------------
		auto start = std::chrono::system_clock::now();
		snbody(sParticles, threadcount, blockcount, iters);
		auto end = std::chrono::system_clock::now();
		time = end - start;
		std::cout << "SMAP: " << std::to_string(time.count()) << std::endl;

		// test dynamic map
		// ----------------------------------------------
		start = std::chrono::system_clock::now();
		dnbody(dParticles, threadcount, blockcount, iters);
		end = std::chrono::system_clock::now();
		time = end - start;
		std::cout << "DMAP:" << std::to_string(time.count()) << std::endl;
	}
}