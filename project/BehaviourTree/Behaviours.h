/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the BT version of the Agario Game
/*=============================================================================*/
//#ifndef ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
//#define ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "stdafx.h"
#include "IExamPlugin.h"
#include "IExamInterface.h"
#include "Exam_HelperStructs.h"
#include "EBehaviorTree.h"
#include "ItemManager.h"
#include "Steering.h"
#include "..\inc\EliteMath\EVector2.h"
//#include "projects/DecisionMaking/SmartAgent.h"
//#include "SteeringBehaviours/SteeringBehaviors.h"
//#include "projects/Movement/SteeringBehaviors/PathFollow/PathFollowSteeringBehavior.h"

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------



namespace BT_Actions
{
	//flee purge zone
	Elite::BehaviorState FleePurgeZone(Elite::Blackboard* pBlackboard)
	{
		std::vector<PurgeZoneInfo> purgeZonesInFOV{};
		if (pBlackboard->GetData("PurgeZonesInFOV", purgeZonesInFOV) == false || purgeZonesInFOV.empty())
			return Elite::BehaviorState::Failure;

		Steering* pSteering{ nullptr };
		if (!pBlackboard->GetData("Steering", pSteering))
			return Elite::BehaviorState::Failure;

		IExamInterface* pInterface{ nullptr };
		if (!pBlackboard->GetData("Interface", pInterface))
			return Elite::BehaviorState::Failure;


		auto agentInfo = pInterface->Agent_GetInfo();

		
		float safeMargin{ 50.f };
		
		

		for (const auto& purgeZone : purgeZonesInFOV)
		{
			pSteering->Flee(purgeZone.Center);

			
				pSteering->Flee(purgeZone.Center);
				pSteering->SpinAround();
				
				if (agentInfo.Stamina > 2.f)
					pSteering->Run(true);

				else if (agentInfo.Stamina <= 0.1f)
					pSteering->Run(false);
					
			
			return Elite::BehaviorState::Success;
		}	
	}

	//shoot
	Elite::BehaviorState AimAndShoot(Elite::Blackboard* pBlackboard)
	{
		ItemManager* pInventory;
		if (!pBlackboard->GetData("Inventory", pInventory))
			return Elite::BehaviorState::Failure;

		std::vector<EnemyInfo> pEnemiesVector;
		if (!pBlackboard->GetData("EnemiesInFOV", pEnemiesVector))
			return Elite::BehaviorState::Failure;

		Steering* pSteering{ nullptr };
		if (!pBlackboard->GetData("Steering", pSteering))
			return Elite::BehaviorState::Failure;

		IExamInterface* pInterface{ nullptr };
		if (!pBlackboard->GetData("Interface", pInterface))
			return Elite::BehaviorState::Failure;

		auto agentInfo = pInterface->Agent_GetInfo();
		


		//shoot shot gun or pistol
		//get number of enemies in FOV
		size_t numberOfEnemies{ pEnemiesVector.size()};
		
		EnemyInfo closestEnemy{};
		closestEnemy.Location = { FLT_MAX ,FLT_MAX };
		for (const EnemyInfo& enemy : pEnemiesVector)
		{
			if (agentInfo.Position.Distance(enemy.Location) < agentInfo.Position.Distance(closestEnemy.Location))
			{
				closestEnemy = enemy;
			}
		}
		Elite::Vector2 desiredDirection = (closestEnemy.Location - agentInfo.Position);

		const float angleMargin{ 0.05f };

		
	

		if (!std::abs(agentInfo.Orientation - std::atan2(desiredDirection.y, desiredDirection.x)) < angleMargin)
		{
			pSteering->Face(closestEnemy.Location);
			return Elite::BehaviorState::Running;
		}
		else
		{
			//if more than 1 --> aim to closest one, shotgun if has
			if (numberOfEnemies > 1)
			{

				//Shoot (shotgun preference)
				pInventory->UseGun();
				return Elite::BehaviorState::Success;

			}
			else //if only one, get type of zombie
			{

				if (closestEnemy.Type == eEnemyType::ZOMBIE_RUNNER) //Shotgun for runners
				{
					pInventory->UseGun();
					return Elite::BehaviorState::Success;
				}
				else  //pistol for normal and heavy zombies
				{
					if (pInventory->HasItem(eItemType::PISTOL))
						pInventory->UsePistol();
					else
						pInventory->UseShotGun();

					return Elite::BehaviorState::Success;
				}
			}
		}
		
	}
	//explore
	Elite::BehaviorState Explore(Elite::Blackboard* pBlackboard)
	{
		Steering* pSteering{ nullptr };
		if (!pBlackboard->GetData("Steering", pSteering))
			return Elite::BehaviorState::Failure;

		IExamInterface* pInterface{ nullptr };
		if (!pBlackboard->GetData("Interface", pInterface))
			return Elite::BehaviorState::Failure;

		auto agentInfo = pInterface->Agent_GetInfo();
		
	
	
		pSteering->Wander();
		pSteering->Run(false);	
		pSteering->SpinAround();
		
	
		return Elite::BehaviorState::Success;
	}
}

namespace BT_Conditions
{
	bool IsPurgeZoneInFOV(Elite::Blackboard* pBlackboard)
	{
		std::vector<PurgeZoneInfo> purgeZonesInFOV{};

		if (pBlackboard->GetData("PurgeZonesInFOV", purgeZonesInFOV) == false || purgeZonesInFOV.empty())
		{
			return false;
		}
		else
			return true;
	}

	bool IsInsidePurgeZone(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pInterface{ nullptr };
		if (!pBlackboard->GetData("Interface", pInterface))
			return false;

		auto agentInfo = pInterface->Agent_GetInfo();
	


		std::vector<PurgeZoneInfo> purgeZonesInFOV{};
		if (pBlackboard->GetData("PurgeZonesInFOV", purgeZonesInFOV) == false)
		{
			return false;
		}

		if (purgeZonesInFOV.empty())
			return false;

		else
		{
			bool inZone{ false };
			float safeMargin{ 5.f };
			for (const auto& purgeZone : purgeZonesInFOV)
			{
				if (  Elite::Distance(agentInfo.Position, purgeZone.Center) + safeMargin < purgeZone.Radius )
					inZone = true;

			}
			return inZone;
		}		
	}


	bool IsEnemyInFOV(Elite::Blackboard* pBlackboard)
	{

		std::vector<EnemyInfo> pEnemyVector;
		if (pBlackboard->GetData("EnemiesInFOV", pEnemyVector) == false || pEnemyVector.empty())
		{
			return false;
		}

		return true;
	}

	bool HasAGun(Elite::Blackboard* pBlackboard)
	{
		ItemManager* pInventory;
		if (!pBlackboard->GetData("Inventory", pInventory))
			return false;

		if (pInventory->HasItem(eItemType::PISTOL) || pInventory->HasItem(eItemType::SHOTGUN))
			return true;
		else
			return false;
	}
	
}