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
	SteeringPlugin_Output* steering { new SteeringPlugin_Output() };

	Vector2 currentPosition = agent.Position; 
	Vector2 targetPos = m_Target.Position;

	Vector2 direction = targetPos - currentPosition;
	direction.Normalize();

	steering->LinearVelocity = direction * agent.MaxLinearSpeed; 

	//debug
	m_pInterface->Draw_SolidCircle(m_Target.Position, .7f, { 0,0 }, { 1, 0, 0 });

	return steering;
}

//FLEE
//****
SteeringPlugin_Output* Flee::CalculateSteering( AgentInfo agent)
{
	SteeringPlugin_Output* steering{ new SteeringPlugin_Output() };

	Vector2 currentPosition = agent.Position;
	Vector2 targetPos = m_Target.Position;

	Vector2 toTarget = currentPosition - targetPos;
	
	//float distanceSquared = toTarget.MagnitudeSquared();
	//if (distanceSquared > m_FleeRadius * m_FleeRadius) //not using the flee radius now
	//{
		//steering. = false;
		//return steering; //
	//}

	steering->LinearVelocity = toTarget;
	steering->LinearVelocity.Normalize();
	steering->LinearVelocity *= agent.MaxLinearSpeed;

	return steering;
}



//ARRIVE
//****
SteeringPlugin_Output* Arrive::CalculateSteering( AgentInfo agent)
{
	SteeringPlugin_Output* steering{ new SteeringPlugin_Output() };

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

	steering->LinearVelocity = toTarger * speed;

	if (distanceSquared < m_TargetRadius * m_TargetRadius) //stop when arrived
	{
		steering->LinearVelocity = Vector2{ 0,0 };
	}

	return steering;
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