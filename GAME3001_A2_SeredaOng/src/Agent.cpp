#include "Agent.h"

#include <iostream>

#include "Util.h"

Agent::Agent()
{
	//initialize whisker colour
	m_lineColour[0] = glm::vec4(0, 1, 0, 1);
	m_lineColour[1] = glm::vec4(0, 1, 0, 1);
	m_lineColour[2] = glm::vec4(0, 1, 0, 1);
	m_lineColour[3] = glm::vec4(0, 1, 0, 1);
	m_lineColour[4] = glm::vec4(0, 1, 0, 1);

	//initialize whisker collisions
	m_collisionWhiskers[0] = false;
	m_collisionWhiskers[1] = false;
	m_collisionWhiskers[2] = false;
	m_collisionWhiskers[3] = false;
	m_collisionWhiskers[4] = false;

	m_whiskerAngle = 45;
}

Agent::~Agent()
= default;

glm::vec2 Agent::getTargetPosition() const
{
	return m_targetPosition;
}

glm::vec2 Agent::getCurrentDirection() const
{
	return m_currentDirection;
}

float Agent::getLOSDistance() const
{
	return m_LOSDistance;
}

bool Agent::hasLOS() const
{
	return m_hasLOS;
}

float Agent::getCurrentHeading() const
{
	return m_currentHeading;
}

glm::vec4 Agent::getLOSColour() const
{
	return m_LOSColour;
}

glm::vec2 Agent::getLeftLOSEndPoint() const
{
	return m_leftLOSEndPoint;
}

glm::vec2 Agent::getMiddleLOSEndPoint() const
{
	return m_middleLOSEndPoint;
}

glm::vec2 Agent::getRightLOSEndPoint() const
{
	return m_rightLOSEndpoint;
}

glm::vec2 Agent::getLeftLeftLOSEndPoint() const
{
	return m_leftLeftLOSEndPoint;
}

glm::vec2 Agent::getRightRightLOSEndPoint() const
{
	return m_rightRightLOSEndPoint;
}

bool* Agent::getCollisionWhiskers()
{
	return m_collisionWhiskers;
}

glm::vec4 Agent::getLineColour(int index)
{
	return m_lineColour[index];
}

float Agent::getWhiskerAngle() const
{
	return m_whiskerAngle;
}

void Agent::setTargetPosition(const glm::vec2 new_position)
{
	m_targetPosition = new_position;
}

void Agent::setCurrentDirection(const glm::vec2 new_direction)
{
	m_currentDirection = new_direction;
}

void Agent::setLOSDistance(const float distance)
{
	m_LOSDistance = distance;
}

void Agent::setHasLOS(const bool state)
{
	m_hasLOS = state;
	m_LOSColour = (m_hasLOS) ? glm::vec4(0, 1, 0, 1) : glm::vec4(1, 0, 0, 1);
}

void Agent::setCurrentHeading(const float heading)
{
	m_currentHeading = heading;
	m_changeDirection();
}

void Agent::setLOSColour(const glm::vec4 colour)
{
	m_LOSColour = colour;
}

void Agent::setLeftLOSEndPoint(const glm::vec2 point)
{
	m_leftLOSEndPoint = point;
}

void Agent::setMiddleLOSEndPoint(const glm::vec2 point)
{
	m_middleLOSEndPoint = point;
}

void Agent::setRightLOSEndPoint(const glm::vec2 point)
{
	m_rightLOSEndpoint = point;
}

void Agent::setLeftLeftLOSEndPoint(const glm::vec2 point)
{
	m_leftLeftLOSEndPoint = point;
}

void Agent::setRightRightLOSEndPoint(const glm::vec2 point)
{
	m_rightRightLOSEndPoint = point;
}

void Agent::setLineColour(const int index, const glm::vec4 colour)
{
	m_lineColour[index] = colour;
}

void Agent::setWhiskerAngle(const float angle)
{
	m_whiskerAngle = angle;
}

void Agent::updateWhiskers(const float angle)
{
	m_whiskerAngle = angle;
	setMiddleLOSEndPoint(getTransform()->position + getCurrentDirection() * getLOSDistance());

	float x = sin((getCurrentHeading() - m_whiskerAngle + 105) * Util::Deg2Rad);
	float y = cos((getCurrentHeading() - m_whiskerAngle + 105) * Util::Deg2Rad);
	setLeftLOSEndPoint(getTransform()->position + glm::vec2(x, -y) * getLOSDistance() * 0.85f);

	x = sin((getCurrentHeading() + m_whiskerAngle + 75) * Util::Deg2Rad);
	y = cos((getCurrentHeading() + m_whiskerAngle + 75) * Util::Deg2Rad);
	setRightLOSEndPoint(getTransform()->position + glm::vec2(x, -y) * getLOSDistance() * 0.85f);

	x = sin((getCurrentHeading() - m_whiskerAngle + 65) * Util::Deg2Rad);
	y = cos((getCurrentHeading() - m_whiskerAngle + 65) * Util::Deg2Rad);
	setLeftLeftLOSEndPoint(getTransform()->position + glm::vec2(x, -y) * getLOSDistance() * 0.65f);

	x = sin((getCurrentHeading() + m_whiskerAngle + 115) * Util::Deg2Rad);
	y = cos((getCurrentHeading() + m_whiskerAngle + 115) * Util::Deg2Rad);
	setRightRightLOSEndPoint(getTransform()->position + glm::vec2(x, -y) * getLOSDistance() * 0.65f);

}

void Agent::m_changeDirection()
{
	const auto x = cos(m_currentHeading * Util::Deg2Rad);
	const auto y = sin(m_currentHeading * Util::Deg2Rad);
	m_currentDirection = glm::vec2(x, y);
}
