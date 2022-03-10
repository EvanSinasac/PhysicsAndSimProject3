//Evan Sinasac - 1081418
//INFO6019 Physics and Simulation and INFO6025 Configuration and Deployment Midterm
//cBullet.h description:
//					cBullet header class for the cannon's Bullet shot

#pragma once

//#include <physics/cCannonShot.h>
#include "cCannonShot.h"

namespace nPhysics
{
	class cParticle : public cCannonShot
	{
	public:
		cBullet(float mass, const glm::vec3& position);
		virtual ~cBullet();

		virtual void Integrate(float deltaTime);

	};
}