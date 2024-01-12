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
#include "..\inc\EliteMath\EVector2.h"
//#include "projects/DecisionMaking/SmartAgent.h"
#include "SteeringBehaviours/SteeringBehaviors.h"
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

		SteeringPlugin_Output* pSteeringOutput{ nullptr };
		if (!pBlackboard->GetData("Steering", pSteeringOutput))
			return Elite::BehaviorState::Failure;

		IExamInterface* pInterface{ nullptr };
		if (!pBlackboard->GetData("Interface", pInterface))
			return Elite::BehaviorState::Failure;


		auto agentInfo = pInterface->Agent_GetInfo();

		
		float safeMargin{ 25.f };
		auto steering = new Flee();
		

		for (const auto& purgeZone : purgeZonesInFOV)
		{
			while (Elite::Distance(agentInfo.Position, purgeZone.Center) + safeMargin < purgeZone.Radius)
			{
				steering->SetTarget(purgeZone.Center);
				pSteeringOutput = steering->CalculateSteering(agentInfo);
				


				if (agentInfo.Stamina > 0.4f)
					pSteeringOutput->RunMode = true;

				else if (agentInfo.Stamina <= 0.f)
					pSteeringOutput->RunMode = false;

				pBlackboard->ChangeData("Steering", pSteeringOutput);
				delete steering;
				return Elite::BehaviorState::Running;
			}
			delete steering;
			return Elite::BehaviorState::Success;
		}
		delete steering;
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

		SteeringPlugin_Output* pSteeringOutput{ nullptr };
		if (!pBlackboard->GetData("Steering", pSteeringOutput))
			return Elite::BehaviorState::Failure;

		IExamInterface* pInterface{ nullptr };
		if (!pBlackboard->GetData("Interface", pInterface))
			return Elite::BehaviorState::Failure;

		auto agentInfo = pInterface->Agent_GetInfo();
		pSteeringOutput->AutoOrient = false;


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

		Face* steering{};
		steering->SetTarget(closestEnemy.Location);

		if (!std::abs(agentInfo.Orientation - std::atan2(desiredDirection.y, desiredDirection.x)) < angleMargin)
		{
			pSteeringOutput = steering->CalculateSteering(agentInfo);
			pBlackboard->ChangeData("Steering", pSteeringOutput);
			
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
		SteeringPlugin_Output* pSteeringOutput{ nullptr };
		if (!pBlackboard->GetData("Steering", pSteeringOutput))
			return Elite::BehaviorState::Failure;

		IExamInterface* pInterface{ nullptr };
		if (!pBlackboard->GetData("Interface", pInterface))
			return Elite::BehaviorState::Failure;

		auto agentInfo = pInterface->Agent_GetInfo();
		
		auto wander = new Wander();
	
		pSteeringOutput = wander->CalculateSteering(agentInfo);
		pSteeringOutput->RunMode = false;
		pSteeringOutput->AutoOrient = false;
		//pSteeringOutput->AngularVelocity += 0.2f;

		pBlackboard->ChangeData("Steering", pSteeringOutput);
		
		delete wander;
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