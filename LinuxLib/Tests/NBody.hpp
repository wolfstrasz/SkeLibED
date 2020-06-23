#pragma once
#ifndef _TEST_NBODY_HPP
#define _TEST_NBODY_HPP


#include <vector>


namespace nbody {
	const float G = 1.0;
	const float delta_t = 0.1;

	// Particle data structure that is used as an element type.
	struct Particle {
		float x, y, z;
		float vx, vy, vz;
		float m;

		// Operator overtide for !=
		bool operator !=(const Particle& a) const;
	};

	/*
	* Array user-function that is used for applying nbody computation,
	* All elements from parr and a single element (named 'pi') are accessible
	* to produce one output element of the same type.
	*/
	Particle move(size_t index, const std::vector<Particle> parr);

	// Generate user-function that is used for initializing particles array.
	Particle init(size_t index, size_t np);

	// Function for Static map testing
	void snbody(std::vector<Particle> &particles);

	// Function for Dynamic map testing
	void dnbody(std::vector<Particle> &particles);

	// Executes and times snbody and dnbody
	void test(size_t threadcount, size_t blockcount, size_t np, size_t iters);

}



#endif
