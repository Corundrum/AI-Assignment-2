#pragma once
#ifndef __CAR__
#define __CAR__

#include "TextureManager.h"
#include <glm/vec4.hpp>

#include "Agent.h"

class Car final : public Agent
{
public:
	Car();
	~Car();

	// Inherited via GameObject
	void draw() override;
	void update() override;
	void clean() override;

	// getters
	float getMaxSpeed() const;
	float getTurnRate() const;
	float getAccelerationRate() const;
	glm::vec2 getDesiredVelocity() const;

	// setters
	void setMaxSpeed(float newSpeed);
	void setTurnRate(float angle);
	void setAccelerationRate(float rate);
	void setDesiredVelocity(glm::vec2 target_position);

	//public member functions
	void Seek();
	void Flee();
	void Arrive();
	void LookWhereYoureGoing(glm::vec2 target_direction);

private:
	//variables
	float m_maxSpeed;
	float m_arrivalDistance;
	float m_turnRate;
	float m_accelerationRate;
	glm::vec2 m_desiredVelocity;



	//private member functions

	void m_checkBounds();
	void m_reset();
	void m_move();
};


#endif /* defined (__SHIP__) */

