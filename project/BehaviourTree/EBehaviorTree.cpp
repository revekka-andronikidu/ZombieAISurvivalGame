//=== General Includes ===
#include "stdafx.h"
#include "EBehaviorTree.h"
using namespace Elite;

//-----------------------------------------------------------------
// BEHAVIOR TREE COMPOSITES (IBehavior)
//-----------------------------------------------------------------
#pragma region COMPOSITES
//SELECTOR
BehaviorState BehaviorSelector::Execute(Blackboard* pBlackBoard)
{
	// BT TODO:
	//TODO: Fill in this code
	for (auto& child : m_ChildBehaviors) 	// Loop over all children in m_ChildBehaviors
	{
		m_CurrentState = child->Execute(pBlackBoard); //Every Child: Execute and store the result in m_CurrentState

		switch (m_CurrentState)
		{
		case BehaviorState::Success://if a child returns Success:
			return m_CurrentState; //stop looping over all children and return Success
			break;
		case BehaviorState::Running://if a child returns Running:
			return m_CurrentState;//Running: stop looping and return Running
			break;
		case BehaviorState::Failure://The selector fails if all children failed.
			continue;//All children failed
			break;
		}
	}
	m_CurrentState = BehaviorState::Failure;
	return m_CurrentState;
}
//SEQUENCE
BehaviorState BehaviorSequence::Execute(Blackboard* pBlackBoard)
{
	// BT TODO:
	for (auto& child : m_ChildBehaviors) 	// Loop over all children in m_ChildBehaviors
	{

		m_CurrentState = child->Execute(pBlackBoard); //Every Child: Execute and store the result in m_CurrentState

		switch (m_CurrentState)
		{
		case BehaviorState::Failure://if a child returns Failed:
			return m_CurrentState; //stop looping over all children and return Failed
			break;
		case BehaviorState::Running://if a child returns Running:
			return m_CurrentState;//Running: stop looping and return Running
			break;
		case BehaviorState::Success://The selector fails if all children failed.
			continue;//All children failed
			break;
		}
	}
		
	//The selector succeeds if all children succeeded.
	//All children succeeded 
	m_CurrentState = BehaviorState::Success;
	return m_CurrentState;
}
//PARTIAL SEQUENCE
BehaviorState BehaviorPartialSequence::Execute(Blackboard* pBlackBoard)
{
	while (m_CurrentBehaviorIndex < m_ChildBehaviors.size())
	{
		m_CurrentState = m_ChildBehaviors[m_CurrentBehaviorIndex]->Execute(pBlackBoard);
		switch (m_CurrentState)
		{
		case BehaviorState::Failure:
			m_CurrentBehaviorIndex = 0;
			return m_CurrentState;
		case BehaviorState::Success:
			++m_CurrentBehaviorIndex;
			m_CurrentState = BehaviorState::Running;
			return m_CurrentState;
		case BehaviorState::Running:
			return m_CurrentState;
		}
	}

	m_CurrentBehaviorIndex = 0;
	m_CurrentState = BehaviorState::Success;
	return m_CurrentState;
}
#pragma endregion
//-----------------------------------------------------------------
// BEHAVIOR TREE CONDITIONAL (IBehavior)
//-----------------------------------------------------------------
BehaviorState BehaviorConditional::Execute(Blackboard* pBlackBoard)
{
	if (m_fpConditional == nullptr)
		return BehaviorState::Failure;

	// This used to be a switch case for some reason, now it's not, be happy :)
	if (m_fpConditional(pBlackBoard))
	{
		m_CurrentState = BehaviorState::Success;
	}
	else
	{
		m_CurrentState = BehaviorState::Failure;
	}
	return m_CurrentState;
}
//-----------------------------------------------------------------
// BEHAVIOR TREE ACTION (IBehavior)
//-----------------------------------------------------------------
BehaviorState BehaviorAction::Execute(Blackboard* pBlackBoard)
{
	if (m_fpAction == nullptr)
		return BehaviorState::Failure;

	m_CurrentState = m_fpAction(pBlackBoard);
	return m_CurrentState;
}