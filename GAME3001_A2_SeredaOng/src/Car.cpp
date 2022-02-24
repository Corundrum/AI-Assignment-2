#include "Car.h"
#include "glm/gtx/string_cast.hpp"
#include "PlayScene.h"
#include "TextureManager.h"
#include "SoundManager.h"
#include "Util.h"
#include "Game.h"
#include "Target.h"

Car::Car() : m_maxSpeed(10.0f)
{
	TextureManager::Instance().load("../Assets/textures/greenCar2.png", "car");

	auto size = TextureManager::Instance().getTextureSize("car");
	setWidth(size.x);
	setHeight(size.y);

	getTransform()->position = glm::vec2(400.0f, 300.0f);
	getRigidBody()->bounds = glm::vec2(getWidth(), getHeight());
	getRigidBody()->velocity = glm::vec2(0.0f, 0.0f);
	getRigidBody()->acceleration = glm::vec2(0.0f, 0.0f);
	getRigidBody()->isColliding = false;

	m_turnRate = 5.0f;
	m_maxSpeed = 6.0f;
	m_arrivalDistance = 200.0f;
	m_accelerationRate = 3.0f;
	setCurrentHeading(0);

	setLOSDistance(250.0f);

	setType(AGENT);
}


Car::~Car()
= default;

void Car::draw()
{
	// alias for x and y
	const auto x = getTransform()->position.x;
	const auto y = getTransform()->position.y;

	// draw the ship
	TextureManager::Instance().draw("car", x, y, getCurrentHeading(), 255, true);
}


void Car::update()
{
}

void Car::clean()
{
}

void Car::m_move()
{
	const float delta_time = TheGame::Instance().getDeltaTime();
	//compute position term
	const glm::vec2 initial_position = getTransform()->position;

	//compute initial velocity
	const glm::vec2 velocity_term = getRigidBody()->velocity;// *delta_time;

	//compute initial acceleration
	const glm::vec2 acceleration_term = getRigidBody()->acceleration * 0.5f * delta_time;

	//compute new position
	glm::vec2 final_position = initial_position + velocity_term + acceleration_term;

	getTransform()->position = final_position;

	//add our acceleration to velocity
	getRigidBody()->velocity += getRigidBody()->acceleration;

	//clmap velocity at max speed
	getRigidBody()->velocity = Util::clamp(getRigidBody()->velocity, getMaxSpeed());
}

float Car::getMaxSpeed() const
{
	return m_maxSpeed;
}

float Car::getTurnRate() const
{
	return m_turnRate;
}

float Car::getAccelerationRate() const
{
	return m_accelerationRate;
}

glm::vec2 Car::getDesiredVelocity() const
{
	return m_desiredVelocity;
}

void Car::setMaxSpeed(const float newSpeed)
{
	m_maxSpeed = newSpeed;
}

void Car::setTurnRate(const float angle)
{
	m_turnRate = angle;
}

void Car::setAccelerationRate(const float rate)
{
	m_accelerationRate = rate;
}

void Car::setDesiredVelocity(const glm::vec2 target_position)
{
	m_desiredVelocity = Util::normalize(target_position - getTransform()->position);
}

void Car::Seek()
{
	m_move();

	setDesiredVelocity(getTargetPosition());

	const glm::vec2 steering_direction = getDesiredVelocity() - getCurrentDirection();

	LookWhereYoureGoing(steering_direction);

	getRigidBody()->acceleration = getCurrentDirection() * getAccelerationRate();
}

void Car::Flee()
{
	m_move();

	setDesiredVelocity(getTargetPosition());

	const glm::vec2 steering_direction = -getDesiredVelocity() - getCurrentDirection();

	LookWhereYoureGoing(steering_direction);

	getRigidBody()->acceleration = getCurrentDirection() * getAccelerationRate();
}

void Car::Arrive()
{
	if (Util::distance(getTransform()->position, getTargetPosition()) > 25.0f)
	{
		Seek();

		if (Util::distance(getTransform()->position, getTargetPosition()) < m_arrivalDistance)
		{
			getRigidBody()->velocity = Util::clamp(getRigidBody()->velocity, getMaxSpeed() * ((Util::distance(getTransform()->position, getTargetPosition()) - 20) / m_arrivalDistance));
		}
	}
}

void Car::LookWhereYoureGoing(glm::vec2 target_direction)
{
	float target_rotation = Util::signedAngle(getCurrentDirection(), target_direction) - 90;
	//auto left_des = Util::signedAngle(getCurrentDirection(), (Util::normalize(getTransform()->position - getLeftLeftLOSEndPoint()) - getCurrentDirection())) - 90;
	//auto right_des = -Util::signedAngle(getCurrentDirection(), (Util::normalize(getTransform()->position - getRightRightLOSEndPoint()) - getCurrentDirection())) - 90;

	if (target_rotation < 0)
	{
		target_rotation += 180;
	}
	const float turn_sensitivity = 3.0f;

	////middle
	//if (getCollisionWhiskers()[1])
	//{
	//	if (getCollisionWhiskers()[0])
	//	{
	//		target_rotation += getTurnRate() * turn_sensitivity * 2;
	//		getRigidBody()->velocity = getRigidBody()->velocity / 1.7f;
	//	}
	//	else if (getCollisionWhiskers()[2])
	//	{
	//		target_rotation -= getTurnRate() * turn_sensitivity * 2;
	//		getRigidBody()->velocity = getRigidBody()->velocity / 1.7f;
	//	}
	//	else if (target_rotation > 0)
	//	{
	//		target_rotation -= getTurnRate() * turn_sensitivity * 2;
	//		getRigidBody()->velocity = getRigidBody()->velocity / 1.7f;
	//	}
	//	else
	//	{
	//		target_rotation += getTurnRate() * turn_sensitivity * 2;
	//		getRigidBody()->velocity = getRigidBody()->velocity / 1.7f;
	//	}
	//}

	////left left
	//if (getCollisionWhiskers()[3])
	//{
	//	left_des += getTurnRate() * turn_sensitivity;
	//	target_rotation += left_des;
	//}
	////left
	//else if (getCollisionWhiskers()[0])
	//{
	//	left_des += getTurnRate() * turn_sensitivity;
	//	target_rotation += left_des;
	//	getRigidBody()->velocity = getRigidBody()->velocity / 1.5f;
	//}
	////right right
	//if (getCollisionWhiskers()[4])
	//{
	//	right_des += getTurnRate() * turn_sensitivity;
	//	target_rotation -= right_des;
	//}
	////right
	//else if (getCollisionWhiskers()[2])
	//{
	//	right_des += getTurnRate() * turn_sensitivity;
	//	target_rotation -= right_des;
	//	getRigidBody()->velocity = getRigidBody()->velocity / 1.5f;
	//}

	setCurrentHeading(Util::lerpUnclamped(getCurrentHeading(), getCurrentHeading() + target_rotation, getTurnRate() * TheGame::Instance().getDeltaTime()));

	//updateWhiskers(getWhiskerAngle());
}

void Car::m_checkBounds()
{

	if (getTransform()->position.x > Config::SCREEN_WIDTH)
	{
		getTransform()->position = glm::vec2(0.0f, getTransform()->position.y);
	}

	if (getTransform()->position.x < 0)
	{
		getTransform()->position = glm::vec2(800.0f, getTransform()->position.y);
	}

	if (getTransform()->position.y > Config::SCREEN_HEIGHT)
	{
		getTransform()->position = glm::vec2(getTransform()->position.x, 0.0f);
	}

	if (getTransform()->position.y < 0)
	{
		getTransform()->position = glm::vec2(getTransform()->position.x, 600.0f);
	}

}

void Car::m_reset()
{
	getRigidBody()->isColliding = false;
	const int halfWidth = getWidth() * 0.5f;
	const auto xComponent = rand() % (640 - getWidth()) + halfWidth + 1;
	const auto yComponent = -getHeight();
	getTransform()->position = glm::vec2(xComponent, yComponent);
}

