//Evan Sinasac - 1081418
//INFO6019 Physics and Simulation and INFO6025 Configuration and Deployment Midterm
//cBullet.cpp description:
//					The cpp file for the cBullet class.  Main difference from cParticle/cCannonShot is the change 
//					to end the particle's lifespan when the Bullet reaches the ground (y=0)

//#include <physics/cBullet.h>
#include "cBullet.h"
#include <iostream>

namespace nPhysics
{
	//oh...my...gawd...this was all I needed for the project...welp, I figured it out eventually lmfao
	cBullet::cBullet(float mass, const glm::vec3& position) : cCannonShot(mass, position)
	{
		SetAcceleration(glm::vec3(0.0f, -9.8f, 0.0f));
		SetIsAlive(true);
	}
	cBullet::~cBullet()
	{

	}

	void cBullet::Integrate(float deltaTime)
	{
		if (GetIsAlive())
		{
			if (GetInverseMass() == 0.f)
			{
				return; // static things don't move!
			}

			SetPosition(glm::vec3(GetPosition() + GetVelocity() * deltaTime));
			// F*(1/m) = a
			SetVelocity(glm::vec3(GetVelocity() + (GetAcceleration() / GetInverseMass() * deltaTime)));

			// apply damping
			SetVelocity(glm::vec3(GetVelocity() * glm::pow(GetDamping(), deltaTime)));

			if (GetPosition().y <= 0)
			{
				SetIsAlive(false);
			}
		}
	}

}