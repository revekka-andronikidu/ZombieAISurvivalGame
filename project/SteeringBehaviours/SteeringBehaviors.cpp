//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"

//#include "framework\EliteMath\EMatrix2x3.h"
#include <limits>

using namespace Elite;
//SEEK
//****
SteeringPlugin_Output* Seek::CalculateSteering( AgentInfo agent)
{
	m_pSteering = new SteeringPlugin_Output();

	Vector2 currentPosition = agent.Position; 
	Vector2 targetPos = m_Target.Position;

	Vector2 direction = targetPos - currentPosition;
	direction.Normalize();

	m_pSteering->LinearVelocity = direction * agent.MaxLinearSpeed;

	//debug
	//m_pInterface->Draw_SolidCircle(m_Target.Position, .7f, { 0,0 }, { 1, 0, 0 });

	return m_pSteering;
}

//FACE
SteeringPlugin_Output* Face::CalculateSteering(AgentInfo agent)
{
	m_pSteering = new SteeringPlugin_Output();

	Elite::Vector2 desiredDirection = (m_Target.Position - agent.Position);
	desiredDirection.Normalize();

	const float agentRot{ agent.Orientation + 0.5f * static_cast<float>(M_PI) };
	Elite::Vector2 agentDirection{ std::cosf(agentRot),std::sinf(agentRot) };

	m_pSteering->AngularVelocity = (desiredDirection.Dot(agentDirection)) * agent.MaxAngularSpeed;


	//debug
	auto dir = agent.Position - m_Target.Position;
	auto dist = dir.Normalize();
	//m_pInterface->Draw_Direction(agent.Position, dir, dist, Elite::Vector3{ 1.f,0,1.0f });

	return m_pSteering;
}

//FLEE
//****
SteeringPlugin_Output* Flee::CalculateSteering( AgentInfo agent)
{
	m_pSteering =  new SteeringPlugin_Output() ;

	Vector2 currentPosition = agent.Position;
	Vector2 targetPos = m_Target.Position;

	Vector2 toTarget = currentPosition - targetPos;
	
	//float distanceSquared = toTarget.MagnitudeSquared();
	//if (distanceSquared > m_FleeRadius * m_FleeRadius) //not using the flee radius now
	//{
		//steering. = false;
		//return steering; //
	//}

	m_pSteering->LinearVelocity = toTarget;
	m_pSteering->LinearVelocity.Normalize();
	m_pSteering->LinearVelocity *= agent.MaxLinearSpeed;

	return m_pSteering;
}



//ARRIVE
//****
SteeringPlugin_Output* Arrive::CalculateSteering( AgentInfo agent)
{
	m_pSteering = new SteeringPlugin_Output();

	Vector2 currentPosition = agent.Position;
	Vector2 targetPos = m_Target.Position;

	Vector2 toTarger = targetPos - currentPosition;

	float distanceSquared = toTarger.MagnitudeSquared();
	toTarger.Normalize();
	float speed = agent.MaxLinearSpeed;

	//float a = distance - m_TargetRadius;
	float b = m_SlowRadius - m_TargetRadius;
	
	if (distanceSquared < m_SlowRadius * m_SlowRadius) //start slowing down
	{
		speed *= distanceSquared / (m_SlowRadius * m_SlowRadius);
	}

	m_pSteering->LinearVelocity = toTarger * speed;

	if (distanceSquared < m_TargetRadius * m_TargetRadius) //stop when arrived
	{
		m_pSteering->LinearVelocity = Vector2{ 0,0 };
	}

	return m_pSteering;
}

//PURSUIT
//****
SteeringPlugin_Output* Pursuit::CalculateSteering( AgentInfo agent)
{
	Vector2 toTarget = m_Target.Position - agent.Position;
	float distance = toTarget.Magnitude();
	float time = distance / agent.MaxLinearSpeed;

	Vector2 predictedPos = m_Target.Position + m_Target.LinearVelocity * time;

	m_Target.Position = predictedPos;

	//return steering;
	return Seek::CalculateSteering( agent);
}

//EVADE
//****
SteeringPlugin_Output* Evade::CalculateSteering( AgentInfo agent)
{
	Vector2 toTarget = m_Target.Position - agent.Position;
	float distanceSqrt = toTarget.MagnitudeSquared();
	float distance = sqrt(distanceSqrt);
	float time = distance / agent.MaxLinearSpeed;
	
	Vector2 predictedPos = m_Target.Position + m_Target.LinearVelocity * time;
	m_Target.Position = predictedPos;
	
	return Flee::CalculateSteering( agent);
}

//WANDER
//****
SteeringPlugin_Output* Wander::CalculateSteering( AgentInfo agent)
{
	Vector2 currentPosition = agent.Position; 
	Vector2 circleCenter{ currentPosition + agent.LinearVelocity.GetNormalized() * m_OffsetDistance };

	m_WanderAngle += Elite::ToRadians((rand() % m_MaxAngleChange) - (float(m_MaxAngleChange) / 2.f));
	

	Vector2 targetPos = {std::cosf(m_WanderAngle), std::sinf(m_WanderAngle )};
	targetPos *= m_Radius;
	targetPos += circleCenter;


	SetTarget(targetPos);
	return Seek::CalculateSteering(agent);	
}

//OffsetPursuit
//****
SteeringPlugin_Output* OffsetPursuit::CalculateSteering( AgentInfo agent)
{
	Vector2 displacement = m_Offset - agent.Position;
	float distance = displacement.Magnitude();

	//Get the character's speed
	float speed = agent.LinearVelocity.Magnitude();

	//Calculate the prediction time 
	const Vector2 distanceBetween = m_Target.Position - agent.Position;
	const float time = distanceBetween.Magnitude() / agent.MaxLinearSpeed;
	const Vector2 futurePosition = m_Target.Position + m_Offset + m_Target.LinearVelocity * time;

	m_Target = futurePosition;

	return Pursuit::CalculateSteering( agent);
}

//SPIN AROUND
SteeringPlugin_Output* SpinAround::CalculateSteering(AgentInfo agent)
{

	m_pSteering = new SteeringPlugin_Output();

	m_pSteering->AngularVelocity = agent.MaxAngularSpeed;

	return m_pSteering;
}

