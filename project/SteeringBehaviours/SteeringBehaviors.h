/*=============================================================================*/
// Copyright 2023-2024 Elite Engine
// Authors: Matthieu Delaere, Thomas Goussaert
/*=============================================================================*/
// SteeringBehaviors.h: SteeringBehaviors interface and different implementations
/*=============================================================================*/
#pragma once
#include <Exam_HelperStructs.h>
#include <IExamInterface.h>
#include "SteeringHelpers.h"

//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------

class SteeringAgent;
class Obstacle;

#pragma region **ISTEERINGBEHAVIOR** (BASE)
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() { delete m_pSteering; };

	virtual SteeringPlugin_Output* CalculateSteering( AgentInfo agent) = 0;

	//Seek Functions
	void SetTarget(const TargetData& target) { m_Target = target; }

	template<class T, typename std::enable_if<std::is_base_of<ISteeringBehavior, T>::value>::type* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	TargetData m_Target;
	SteeringPlugin_Output* m_pSteering;
	//IExamInterface* m_pInterface = nullptr;
};
#pragma endregion

///////////////////////////////////////
//SEEK
//****
class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() = default;

	//Seek Behaviour
	SteeringPlugin_Output* CalculateSteering( AgentInfo agent) override;
};

///////////////////////////////////////
//FACE
//****
class Face : public ISteeringBehavior
{
public:
	Face() = default;
	virtual ~Face() = default;


	SteeringPlugin_Output* CalculateSteering(AgentInfo agent) override;
	
};


///////////////////////////////////////
//FLEE
//****

class Flee : public ISteeringBehavior
{
public:
	Flee(float fleeRadius = 10.f) : m_FleeRadius{ fleeRadius }{};
	virtual ~Flee() = default;

	//Flee Behaviour
	SteeringPlugin_Output* CalculateSteering( AgentInfo agent) override;
	
	float GetFleeRadius(){	return m_FleeRadius;}
	void SetFleeRadius(float fleeRadius){ m_FleeRadius = fleeRadius;	}

private:
	float m_FleeRadius = 40.f;
	

};

///////////////////////////////////////
//ARRIVE
//****
class Arrive : public ISteeringBehavior
{
public:
	Arrive() = default;
	virtual ~Arrive() = default;

	//Arrive Behaviour
	SteeringPlugin_Output* CalculateSteering( AgentInfo agentInfo) override;
	void SetTargetRadius(float radius) { m_TargetRadius = radius; };
	void SetSlowRadius(float radius) { m_SlowRadius = radius; };

private:
	float m_SlowRadius = 15.f;
	float m_TargetRadius = 2.f;
	
};


///////////////////////////////////////
//PURSUIT
//****
class Pursuit : public Seek
{
public:
	Pursuit() = default;
	virtual ~Pursuit() = default;

	//Pursuit Behaviour
	SteeringPlugin_Output* CalculateSteering(AgentInfo agent) override;

};


///////////////////////////////////////
//EVADE
//****
class Evade : public Flee
{
public:
	Evade(float evadeRadius = 15.f) : Flee(evadeRadius) { m_evadeRadius = evadeRadius; };
	virtual ~Evade() = default;

	//Evade Behaviour
	SteeringPlugin_Output* CalculateSteering( AgentInfo agent) override;
private:
	float m_evadeRadius{};
};

///////////////////////////////////////
//WANDER
//****
class Wander : public Seek
{
public:
	Wander() = default;
	virtual ~Wander() = default;

	//Wander Behaviour
	SteeringPlugin_Output* CalculateSteering( AgentInfo agent) override;

	void SetWanderOffset(float offset) { m_OffsetDistance = offset; }
	void SetWanderRadius(float radius) { m_Radius = radius; }
	void SetMaxAngleChange(float rad) { m_MaxAngleChange = rad; }

protected:
	float m_Radius = 4.f;
	float m_OffsetDistance = 6.f;
	int m_MaxAngleChange = 45;
	float m_WanderAngle = 0.f;
};

///////////////////////////////////////
//OFFSET PURSUIT
//****
class OffsetPursuit : public Pursuit
{
public:
	OffsetPursuit() = default;
	virtual ~OffsetPursuit() = default;
	SteeringPlugin_Output* CalculateSteering(AgentInfo agent) override;

protected:
	Elite::Vector2 m_Offset{ 5.f,5.f };
	

};

///////////////////////////////////////
//Spin Arround
//****
class SpinAround : public ISteeringBehavior
{
public:
	SpinAround() = default;
	virtual ~SpinAround() = default;

	
	SteeringPlugin_Output* CalculateSteering(AgentInfo agent) override;

};

