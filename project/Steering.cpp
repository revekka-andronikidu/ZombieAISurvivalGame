#include "stdafx.h"
#include "Steering.h"


Steering::Steering(IExamInterface* pInterface, SteeringPlugin_Output* pSteering)
	:m_pInterface{ pInterface }
	, m_pSteering{ pSteering }
{
}

void Steering::AutoOrient(bool state)
{
	m_pSteering->AutoOrient = state;
}

void Steering::Seek(const Elite::Vector2& target)
{
	auto agentInfo = m_pInterface->Agent_GetInfo();
	Elite::Vector2 currentPosition = agentInfo.Position;
	Elite::Vector2 direction = target - currentPosition;
	direction.Normalize();

	m_pSteering->LinearVelocity = direction * agentInfo.MaxLinearSpeed;

	//debug
	m_pInterface->Draw_SolidCircle(target, .7f, { 0,0 }, { 1, 0, 0 });

}
void Steering::Flee(const Elite::Vector2& target)
{
	auto agentInfo = m_pInterface->Agent_GetInfo();
	Elite::Vector2 currentPosition = agentInfo.Position;
	Elite::Vector2 toTarget = currentPosition - target;

	//float distanceSquared = toTarget.MagnitudeSquared();
	//if (distanceSquared > m_FleeRadius * m_FleeRadius) //not using the flee radius now
	//{
		//steering. = false;
		//return steering; //
	//}

	m_pSteering->LinearVelocity = toTarget;
	m_pSteering->LinearVelocity.Normalize();
	m_pSteering->LinearVelocity *= agentInfo.MaxLinearSpeed;
}


void Steering::Face(const Elite::Vector2& target)
{
	auto agentInfo = m_pInterface->Agent_GetInfo();

	Elite::Vector2 desiredDirection = (target - agentInfo.Position);
	desiredDirection.Normalize();

	const float agentRot{ agentInfo.Orientation + 0.5f * static_cast<float>(M_PI) };
	Elite::Vector2 agentDirection{ std::cosf(agentRot),std::sinf(agentRot) };

	m_pSteering->AutoOrient = false;
	m_pSteering->AngularVelocity = (desiredDirection.Dot(agentDirection)) * agentInfo.MaxAngularSpeed;

	//debug
	auto dir = agentInfo.Position - target;
	auto dist = dir.Normalize();
	m_pInterface->Draw_Direction(agentInfo.Position, dir, dist, Elite::Vector3{ 1.f,0,1.0f });
}

void Steering::SpinAround()
{
	auto agentInfo = m_pInterface->Agent_GetInfo();
	m_pSteering->AutoOrient = false;
	m_pSteering->AngularVelocity = agentInfo.MaxAngularSpeed/2.f;
}

void Steering::Stop()
{
	
	m_pSteering->LinearVelocity = {0.f,0.f};
	
}

void Steering::Run(bool state)
{
	
		m_pSteering->RunMode = state;

	
}

void Steering::Wander()
{
	auto agentInfo = m_pInterface->Agent_GetInfo();
	const float m_Radius = 4.f;
	const float m_OffsetDistance = 0.f;
	const int m_MaxAngleChange = 180;
	float m_WanderAngle = 360.f;

	Elite::Vector2 currentPosition = agentInfo.Position;
	Elite::Vector2 circleCenter{ currentPosition + agentInfo.LinearVelocity.GetNormalized() * m_OffsetDistance };

	m_WanderAngle += Elite::ToRadians((rand() % m_MaxAngleChange) - (float(m_MaxAngleChange) / 2.f));


	Elite::Vector2 targetPos = { std::cosf(m_WanderAngle), std::sinf(m_WanderAngle) };
	targetPos *= m_Radius;
	targetPos += circleCenter;


	Seek(targetPos);
	
	//debug
	Elite::Vector2 targetVector{ targetPos - agentInfo.Position };
	m_pInterface->Draw_Circle(circleCenter, m_Radius, Elite::Vector3{ 0,1,0 });
	m_pInterface->Draw_Direction(agentInfo.Position, targetVector, m_OffsetDistance, Elite::Vector3{ 0,1,0 });
}