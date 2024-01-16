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
	
		float safeMargin{ 10.f };
		
		for (const auto& purgeZone : purgeZonesInFOV)
		{		
			Elite::Vector2 direction = agentInfo.Position - purgeZone.Center;
			direction.Normalize();
			Elite::Vector2 target = (purgeZone.Center + (purgeZone.Radius + safeMargin) * direction);
			const Elite::Vector2 nextTargetPos = pInterface->NavMesh_GetClosestPathPoint(target);
			
				pSteering->Seek(nextTargetPos);
				pSteering->SpinAround();
				
				if (agentInfo.Stamina > 2.f)
					pSteering->Run(true);
				else if (agentInfo.Stamina <= 0.1f)
					pSteering->Run(false);
			
			return Elite::BehaviorState::Success;
		}	
		return Elite::BehaviorState::Success;
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
	

		if (std::abs(agentInfo.Orientation - std::atan2(desiredDirection.y, desiredDirection.x)) < angleMargin)
		{	
			pSteering->Stop();
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
		else
		{
			pSteering->Face(closestEnemy.Location);
			pSteering->Flee(closestEnemy.Location);
			if (agentInfo.Stamina > 2.f)
				pSteering->Run(true);

			else if (agentInfo.Stamina <= 0.1f)
				pSteering->Run(false);

			return Elite::BehaviorState::Running;
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
		
		Explorer* pExplorer{ nullptr };
		if(!pBlackboard->GetData("Explorer", pExplorer))
			return Elite::BehaviorState::Failure;

		auto target = pExplorer->NextClosestCell();

		const Elite::Vector2 nextTargetPos = pInterface->NavMesh_GetClosestPathPoint(target);
	
		pSteering->Seek(nextTargetPos);
		if (agentInfo.Stamina > 8.f)
			pSteering->Run(true);

		else if (agentInfo.Stamina <= 4.f)
			pSteering->Run(false);
		pSteering->SpinAround();
		
		
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState RememberHouse(Elite::Blackboard* pBlackboard)
	{
		std::vector<HouseInfo> pHousesInFOV;
		if (pBlackboard->GetData("HousesInFOV", pHousesInFOV) == false || pHousesInFOV.empty())
			return Elite::BehaviorState::Failure;

		std::vector<HouseSearch> pHousesMemory;
		if (pBlackboard->GetData("HousesInMemory", pHousesMemory) == false)
			return Elite::BehaviorState::Failure;

		if (pHousesMemory.empty())
		{
			for (const auto& houseInFOV : pHousesInFOV)
			{
				HouseSearch newHouse{};
				newHouse.Center = houseInFOV.Center;
				newHouse.Size = houseInFOV.Size;
				newHouse.CurrentCorner = 0;
				newHouse.FinishedSearch = false;
				//newHouse.lastVisited;
				pHousesMemory.push_back(newHouse);

			}
			pBlackboard->ChangeData("HousesInMemory", pHousesMemory);
			return Elite::BehaviorState::Success;
		}
		else
		{
			for (const auto& houseInFOV : pHousesInFOV)
			{
				bool found{ false };
				for (const auto& memory : pHousesMemory)
				{
					if (houseInFOV.Center == memory.Center)
						found = true;
				}
				if (!found)
				{
					HouseSearch newHouse{};
					newHouse.Center = houseInFOV.Center;
					newHouse.Size = houseInFOV.Size;
					newHouse.CurrentCorner = 0;
					newHouse.FinishedSearch = false;
					pHousesMemory.push_back(newHouse);
				}
			}
			pBlackboard->ChangeData("HousesInMemory", pHousesMemory);
			return Elite::BehaviorState::Success;
		}
	}


	//search house
	Elite::BehaviorState SearchClosestHouseInMemory(Elite::Blackboard* pBlackboard)
	{
		Steering* pSteering{ nullptr };
		if (!pBlackboard->GetData("Steering", pSteering))
			return Elite::BehaviorState::Failure;

		IExamInterface* pInterface{ nullptr };
		if (!pBlackboard->GetData("Interface", pInterface))
			return Elite::BehaviorState::Failure;
		auto agentInfo = pInterface->Agent_GetInfo();

		std::vector<HouseSearch> pHousesMemory;
		if (pBlackboard->GetData("HousesInMemory", pHousesMemory) == false || pHousesMemory.empty())
			return Elite::BehaviorState::Failure;

		
		

		HouseSearch closestHouse{};
		closestHouse.Center = {FLT_MAX ,FLT_MAX};		
		for (const auto& house : pHousesMemory)
		{

			if (agentInfo.Position.Distance(house.Center) < agentInfo.Position.Distance(closestHouse.Center))
			{
				if(!house.FinishedSearch)
				closestHouse = house;
			}

		}
		if(closestHouse.Center.x == FLT_MAX) //no house left to search
			return Elite::BehaviorState::Failure;

		

		Elite::Vector2 target{};	
		switch (closestHouse.CurrentCorner)
		{
		case 0:
		{
			pSteering->SpinAround();
			target = closestHouse.Center;
			
			break;
		}
		case 1:
		{
			target =
			{
				closestHouse.Center.x - closestHouse.Size.x / 3.0f,
				closestHouse.Center.y + closestHouse.Size.y / 3.0f
			};
			
			break;
		}
		case 2:
		{
			target =
			{
				closestHouse.Center.x + closestHouse.Size.x / 3.0f,
				closestHouse.Center.y - closestHouse.Size.y / 3.0f
			};
			
			break;
		}
		case 3:
		{
			target =
			{
				closestHouse.Center.x + closestHouse.Size.x / 3.0f,
				closestHouse.Center.y + closestHouse.Size.y / 3.0f
			};
			
			break;
		}
		case 4:
		{
			target =
			{
				closestHouse.Center.x - closestHouse.Size.x / 3.0f,
				closestHouse.Center.y - closestHouse.Size.y / 3.0f
			};
			
			break;
		}
		case 5:
		{
			closestHouse.FinishedSearch = true;
			closestHouse.lastVisited = std::chrono::steady_clock::now();
			std::cout << "House fully searched" << std::endl;
			break;
		}
		}


		const float margin{ 4.f };
		if (target.Distance(agentInfo.Position) < margin)	
		{
			closestHouse.CurrentCorner++;
		}

		for (size_t idx{}; idx < pHousesMemory.size(); ++idx) //update data in blackboard
		{
			if (pHousesMemory[idx].Center == closestHouse.Center)
			{
				pHousesMemory.erase(pHousesMemory.begin() + idx);
				pHousesMemory.push_back(closestHouse);
			}
		}

		pBlackboard->ChangeData("HousesInMemory", pHousesMemory);
		const Elite::Vector2 nextTargetPos = pInterface->NavMesh_GetClosestPathPoint(target);
		pSteering->Seek(nextTargetPos);
		if (agentInfo.Stamina > 8.f)
			pSteering->Run(true);
		else if (agentInfo.Stamina <= 4.f)
			pSteering->Run(false);

		if(closestHouse.CurrentCorner != 0)
		pSteering->Face(nextTargetPos);
		return Elite::BehaviorState::Success;
	}



	Elite::BehaviorState FleeFromEnemy(Elite::Blackboard* pBlackboard)
	{
		Steering* pSteering{ nullptr };
		if (!pBlackboard->GetData("Steering", pSteering))
			return Elite::BehaviorState::Failure;

		IExamInterface* pInterface{ nullptr };
		if (!pBlackboard->GetData("Interface", pInterface))
			return Elite::BehaviorState::Failure;

		std::vector<EnemyInfo> pEnemiesVector;
		if (pBlackboard->GetData("EnemiesInFOV", pEnemiesVector) == false || pEnemiesVector.empty())
			return Elite::BehaviorState::Failure;

		auto agentInfo = pInterface->Agent_GetInfo();

		EnemyInfo closestEnemy{};
		closestEnemy.Location = { FLT_MAX ,FLT_MAX };
		for (const EnemyInfo& enemy : pEnemiesVector)
		{
			if (agentInfo.Position.Distance(enemy.Location) < agentInfo.Position.Distance(closestEnemy.Location))
			{
				closestEnemy = enemy;
			}
		}

		
		if (agentInfo.Stamina > 2.f)
			pSteering->Run(true);

		else if (agentInfo.Stamina <= 0.1f)
			pSteering->Run(false);


		Elite::Vector2 target = (agentInfo.Position  - closestEnemy.Location) * 5.f  ;
		

		const Elite::Vector2 nextTargetPos = pInterface->NavMesh_GetClosestPathPoint(target);

		pSteering->Seek(nextTargetPos);
		//pSteering->SpinAround();
		pSteering->Face(closestEnemy.Location);

		pBlackboard->ChangeData("InDanger", true);

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState TakeTheItem(Elite::Blackboard* pBlackboard)
	{
		ItemManager* pInventory;
		if (!pBlackboard->GetData("Inventory", pInventory))
			return Elite::BehaviorState::Failure;

		std::vector<ItemInfo> pItemsInFOV;
		if (pBlackboard->GetData("ItemsInFOV", pItemsInFOV) == false || pItemsInFOV.empty())
			return Elite::BehaviorState::Failure;

		Steering* pSteering{ nullptr };
		if (!pBlackboard->GetData("Steering", pSteering))
			return Elite::BehaviorState::Failure;

		IExamInterface* pInterface{ nullptr };
		if (!pBlackboard->GetData("Interface", pInterface))
			return Elite::BehaviorState::Failure;
		auto agentInfo = pInterface->Agent_GetInfo();

		std::vector<ItemInfo> pMemory;
		if (pBlackboard->GetData("ItemsInMemory", pMemory) == false || pItemsInFOV.empty())
			return Elite::BehaviorState::Failure;

		

		//come close to item
		ItemInfo closestItem{};
		closestItem.Location = { FLT_MAX ,FLT_MAX };
		for (const ItemInfo& item : pItemsInFOV)
		{
			if (agentInfo.Position.Distance(item.Location) < agentInfo.Position.Distance(closestItem.Location))
			{
				closestItem = item;
			}
		}
		

		pSteering->Seek(closestItem.Location);
		pSteering->Face(closestItem.Location);
		//pSteering->AutoOrient(true);

		if ((agentInfo.Position.Distance(closestItem.Location) < agentInfo.GrabRange ))
		{
			if (closestItem.Type == eItemType::GARBAGE)
			{
				pInterface->DestroyItem(closestItem);
				return Elite::BehaviorState::Success;
			}
			else 
			{
				for (size_t i{}; i < pMemory.size(); i++) //if item in memory erase
				{
					if (closestItem.Location == pMemory[i].Location)
					{
						pMemory.erase(pMemory.begin() + i);
						pBlackboard->ChangeData("ItemsInMemory", pMemory);
					}
				}

				pInterface->GrabItem(closestItem);
				pInventory->AddItem(closestItem);
				
				return Elite::BehaviorState::Success;
			}
		}
		else
		{
			return Elite::BehaviorState::Running;
		}
		return Elite::BehaviorState::Running;
	}

	Elite::BehaviorState SeekClosestFood(Elite::Blackboard* pBlackboard)
	{

		std::vector<ItemInfo> pMemory;
		if (pBlackboard->GetData("ItemsInMemory", pMemory) == false || pMemory.empty())
			return Elite::BehaviorState::Failure;

		Steering* pSteering{ nullptr };
		if (!pBlackboard->GetData("Steering", pSteering))
			return Elite::BehaviorState::Failure;

		IExamInterface* pInterface{ nullptr };
		if (!pBlackboard->GetData("Interface", pInterface))
			return Elite::BehaviorState::Failure;
		auto agentInfo = pInterface->Agent_GetInfo();

		ItemInfo closestFood{};
		closestFood.Location = { FLT_MAX ,FLT_MAX };

		for (auto item : pMemory)
		{
			if (item.Type == eItemType::FOOD)
			{
				if (agentInfo.Position.Distance(item.Location) < agentInfo.Position.Distance(closestFood.Location))
				{
					closestFood.Location = item.Location;
				}
			}
		}
		if (closestFood.Location.x == FLT_MAX)
			return Elite::BehaviorState::Failure;

		const Elite::Vector2 nextTargetPos = pInterface->NavMesh_GetClosestPathPoint(closestFood.Location);
		pSteering->Seek(nextTargetPos);
		if (agentInfo.Stamina > 8.f)
			pSteering->Run(true);
		else if (agentInfo.Stamina <= 4.f)
			pSteering->Run(false);
		pSteering->SpinAround();
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState SeekClosestMedKit(Elite::Blackboard* pBlackboard)
	{

		std::vector<ItemInfo> pMemory;
		if (pBlackboard->GetData("ItemsInMemory", pMemory) == false || pMemory.empty())
			return Elite::BehaviorState::Failure;

		Steering* pSteering{ nullptr };
		if (!pBlackboard->GetData("Steering", pSteering))
			return Elite::BehaviorState::Failure;

		IExamInterface* pInterface{ nullptr };
		if (!pBlackboard->GetData("Interface", pInterface))
			return Elite::BehaviorState::Failure;
		auto agentInfo = pInterface->Agent_GetInfo();

		ItemInfo closestMedkit{};
		closestMedkit.Location = { FLT_MAX ,FLT_MAX };

		for (auto item : pMemory)
		{
			if (item.Type == eItemType::MEDKIT)
			{
				if (agentInfo.Position.Distance(item.Location) < agentInfo.Position.Distance(closestMedkit.Location))
				{
					closestMedkit.Location = item.Location;
				}
			}
		}
		if (closestMedkit.Location.x == FLT_MAX)
			return Elite::BehaviorState::Failure;

		const Elite::Vector2 nextTargetPos = pInterface->NavMesh_GetClosestPathPoint(closestMedkit.Location);
		pSteering->Seek(nextTargetPos);
		if (agentInfo.Stamina > 8.f)
			pSteering->Run(true);

		else if (agentInfo.Stamina <= 4.f)
			pSteering->Run(false);
		pSteering->SpinAround();
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState SeekClosestPistol(Elite::Blackboard* pBlackboard)
	{

		std::vector<ItemInfo> pMemory;
		if (pBlackboard->GetData("ItemsInMemory", pMemory) == false || pMemory.empty())
			return Elite::BehaviorState::Failure;

		Steering* pSteering{ nullptr };
		if (!pBlackboard->GetData("Steering", pSteering))
			return Elite::BehaviorState::Failure;

		IExamInterface* pInterface{ nullptr };
		if (!pBlackboard->GetData("Interface", pInterface))
			return Elite::BehaviorState::Failure;
		auto agentInfo = pInterface->Agent_GetInfo();

		ItemInfo closestPistol{};
		closestPistol.Location = { FLT_MAX ,FLT_MAX };

		for (const auto& item : pMemory)
		{
			if (item.Type == eItemType::PISTOL)
			{
				if (agentInfo.Position.Distance(item.Location) < agentInfo.Position.Distance(closestPistol.Location))
				{
					closestPistol.Location = item.Location;
				}
			}
		}
		if (closestPistol.Location.x == FLT_MAX)
			return Elite::BehaviorState::Failure;

		const Elite::Vector2 nextTargetPos = pInterface->NavMesh_GetClosestPathPoint(closestPistol.Location);
		pSteering->Seek(nextTargetPos);
		pSteering->SpinAround();
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState SeekClosestShotgun(Elite::Blackboard* pBlackboard)
	{

		std::vector<ItemInfo> pMemory;
		if (pBlackboard->GetData("ItemsInMemory", pMemory) == false || pMemory.empty())
			return Elite::BehaviorState::Failure;

		Steering* pSteering{ nullptr };
		if (!pBlackboard->GetData("Steering", pSteering))
			return Elite::BehaviorState::Failure;

		IExamInterface* pInterface{ nullptr };
		if (!pBlackboard->GetData("Interface", pInterface))
			return Elite::BehaviorState::Failure;
		auto agentInfo = pInterface->Agent_GetInfo();

		ItemInfo closestShotgun{};
		closestShotgun.Location = { FLT_MAX ,FLT_MAX };

		for (const auto& item : pMemory)
		{
			if (item.Type == eItemType::SHOTGUN)
			{
				if (agentInfo.Position.Distance(item.Location) < agentInfo.Position.Distance(closestShotgun.Location))
				{
					closestShotgun.Location = item.Location;
				}
			}
		}
		if (closestShotgun.Location.x == FLT_MAX)
			return Elite::BehaviorState::Failure;

		const Elite::Vector2 nextTargetPos = pInterface->NavMesh_GetClosestPathPoint(closestShotgun.Location);
		pSteering->Seek(nextTargetPos);
		pSteering->SpinAround();
		return Elite::BehaviorState::Success;
	}
	Elite::BehaviorState SeekClosestGun(Elite::Blackboard* pBlackboard)
	{

		std::vector<ItemInfo> pMemory;
		if (pBlackboard->GetData("ItemsInMemory", pMemory) == false || pMemory.empty())
			return Elite::BehaviorState::Failure;

		Steering* pSteering{ nullptr };
		if (!pBlackboard->GetData("Steering", pSteering))
			return Elite::BehaviorState::Failure;

		IExamInterface* pInterface{ nullptr };
		if (!pBlackboard->GetData("Interface", pInterface))
			return Elite::BehaviorState::Failure;
		auto agentInfo = pInterface->Agent_GetInfo();
	

		ItemInfo closestGun{};
		closestGun.Location = { FLT_MAX ,FLT_MAX };

		for (const auto& item : pMemory)
		{
			if (item.Type == eItemType::PISTOL || item.Type == eItemType::SHOTGUN)
			{
				if (agentInfo.Position.Distance(item.Location) < agentInfo.Position.Distance(closestGun.Location))
				{
					closestGun.Location = item.Location;
				}
			}
		}
		if (closestGun.Location.x == FLT_MAX)
			return Elite::BehaviorState::Failure;

		const Elite::Vector2 nextTargetPos = pInterface->NavMesh_GetClosestPathPoint(closestGun.Location);
		pSteering->Seek(nextTargetPos);
		pSteering->SpinAround();
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState RevisitHouses(Elite::Blackboard* pBlackboard)
	{
		std::vector<HouseSearch> pHouseVector;
		if (pBlackboard->GetData("HousesInMemory", pHouseVector) == false || pHouseVector.empty())
			return Elite::BehaviorState::Failure;

		if (pHouseVector.size() < 8)
			return Elite::BehaviorState::Failure;

		for (size_t idx{}; idx < pHouseVector.size(); idx ++)
		{
			if (pHouseVector[idx].FinishedSearch)
			{
				const std::chrono::duration<float> time = std::chrono::steady_clock::now() - pHouseVector[idx].lastVisited;
				auto durationFloat = std::chrono::duration_cast<std::chrono::duration<float>>(time);
				float durationInSeconds = durationFloat.count();
				if (durationInSeconds >= 240.0f)
				{
					pHouseVector[idx].FinishedSearch = false;
					pHouseVector[idx].CurrentCorner = 0;
					std::cout << "House to revisit added" << std::endl;
					pBlackboard->ChangeData("HousesInMemory", pHouseVector);
					return Elite::BehaviorState::Success;
				}
			}
		}
		return Elite::BehaviorState::Success;
	}
	Elite::BehaviorState GoOutside(Elite::Blackboard* pBlackboard)
	{
		Steering* pSteering{ nullptr };
		if (!pBlackboard->GetData("Steering", pSteering))
			return Elite::BehaviorState::Failure;

		std::vector<HouseSearch> pHouseVector;
		if (pBlackboard->GetData("HousesInMemory", pHouseVector) == false || pHouseVector.empty())
			return Elite::BehaviorState::Failure;

		IExamInterface* pInterface{ nullptr };
		if (!pBlackboard->GetData("Interface", pInterface))
			return Elite::BehaviorState::Failure;
		auto agentInfo = pInterface->Agent_GetInfo();

		if(!agentInfo.IsInHouse)
			return Elite::BehaviorState::Success;


		HouseSearch closestHouse{};
		closestHouse.Center = { FLT_MAX ,FLT_MAX };
		for (const auto& house : pHouseVector)
		{

			if (agentInfo.Position.Distance(house.Center) < agentInfo.Position.Distance(closestHouse.Center))
			{
				if (!house.FinishedSearch)
					closestHouse = house;
			}

		}
		if (closestHouse.Center.x == FLT_MAX) //no house left to search
			return Elite::BehaviorState::Failure;

		Elite::Vector2 target{}; //outside corner
		target =
		{
			closestHouse.Center.x - closestHouse.Size.x / 2.0f,
			closestHouse.Center.y - closestHouse.Size.y / 2.0f
		};

		const float margin{ 4.f };
		if (target.Distance(agentInfo.Position) < margin)
		{
			return Elite::BehaviorState::Success;
		}

		const Elite::Vector2 nextTargetPos = pInterface->NavMesh_GetClosestPathPoint(target);
		pSteering->Seek(nextTargetPos);
		return Elite::BehaviorState::Running;
	}

	Elite::BehaviorState RememberItem(Elite::Blackboard* pBlackboard)
	{
		std::vector<ItemInfo> pItemsInFOV;
		if (pBlackboard->GetData("ItemsInFOV", pItemsInFOV) == false || pItemsInFOV.empty())
			return Elite::BehaviorState::Failure;

		std::vector<ItemInfo> pMemory;
		if (pBlackboard->GetData("ItemsInMemory", pMemory) == false)
			return Elite::BehaviorState::Failure;

		IExamInterface* pInterface{ nullptr };
		if (!pBlackboard->GetData("Interface", pInterface))
			return Elite::BehaviorState::Failure;
		auto agentInfo = pInterface->Agent_GetInfo();

		ItemInfo closestItem{};
		closestItem.Location = { FLT_MAX ,FLT_MAX };
		for (const ItemInfo& item : pItemsInFOV)
		{
			if (agentInfo.Position.Distance(item.Location) < agentInfo.Position.Distance(closestItem.Location))
			{
				closestItem = item;
			}
		}


		for (const auto& item : pMemory)
		{

			if (item.Location == closestItem.Location) //if item already in memory return
				return Elite::BehaviorState::Success;
		}



		pMemory.push_back(closestItem);
		pBlackboard->ChangeData("ItemsInMemory", pMemory);
		return Elite::BehaviorState::Success;

	}

	Elite::BehaviorState HideInHouse(Elite::Blackboard* pBlackboard)
	{
		
		std::vector<HouseSearch> pHouseVector;
		if (pBlackboard->GetData("HousesInMemory", pHouseVector) == false || pHouseVector.empty())
			return Elite::BehaviorState::Failure;
		
		Steering* pSteering{ nullptr };
		if (!pBlackboard->GetData("Steering", pSteering))
			return Elite::BehaviorState::Failure;

		IExamInterface* pInterface{ nullptr };
		if (!pBlackboard->GetData("Interface", pInterface))
			return Elite::BehaviorState::Failure;
		auto agentInfo = pInterface->Agent_GetInfo();

		HouseSearch closestHouse{};
		closestHouse.Center = { FLT_MAX ,FLT_MAX };
		for (const auto& house : pHouseVector)
		{

			if (agentInfo.Position.Distance(house.Center) < agentInfo.Position.Distance(closestHouse.Center))
			{

					closestHouse.Center = house.Center;
			}

		}
		if (closestHouse.Center.x == FLT_MAX) //something went wrong
			return Elite::BehaviorState::Failure;

		const float margin{ 3.f };
		if (closestHouse.Center.Distance(agentInfo.Position) < margin)	
		{
			return Elite::BehaviorState::Success;
			
		}

		
		const Elite::Vector2 nextTargetPos = pInterface->NavMesh_GetClosestPathPoint(closestHouse.Center);
		pSteering->Seek(nextTargetPos);
		
		if (agentInfo.Stamina > 2.f)
			pSteering->Run(true);

		else if (agentInfo.Stamina <= 0.1f)
			pSteering->Run(false);
		pSteering->SpinAround();
		

		

		pBlackboard->ChangeData("Steering", pSteering);
		return Elite::BehaviorState::Running;
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

	bool IsHouseInFOV(Elite::Blackboard* pBlackboard)
	{

		std::vector<HouseInfo> pHouseVector;
		if (pBlackboard->GetData("HousesInFOV", pHouseVector) == false || pHouseVector.empty())
		{
			return false;
		}
		return true;
	}
	bool IsInsideOfHouse(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pInterface{ nullptr };
		if (!pBlackboard->GetData("Interface", pInterface))
			return false;
		auto agentInfo = pInterface->Agent_GetInfo();

		return agentInfo.IsInHouse;

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

	bool WasBitten(Elite::Blackboard* pBlackboard) //was bitten or enemy in fov
	{
		IExamInterface* pInterface{ nullptr };
		if (!pBlackboard->GetData("Interface", pInterface))
			return false;


		auto agentInfo = pInterface->Agent_GetInfo();
		if (agentInfo.WasBitten)
			return true;
		else
			return false;
	}
	
	bool IsInDanger(Elite::Blackboard* pBlackboard) //was bitten or enemy in fov
	{
		bool danger{};
		if(!pBlackboard->GetData("InDanger", danger))
			return false;

		bool rt =(WasBitten(pBlackboard) || IsEnemyInFOV(pBlackboard) || danger );
		return rt;
	}

	bool IsItemInFOV(Elite::Blackboard* pBlackboard)
	{

		std::vector<ItemInfo> pItemVector;
		if (pBlackboard->GetData("ItemsInFOV", pItemVector) == false || pItemVector.empty())
		{
			return false;
		}
		//std::cout << "Item in FOV" << std::endl;
		return true;
	}

	bool NeedClosestItem(Elite::Blackboard* pBlackboard)
	{
		ItemManager* pInventory;
		if (!pBlackboard->GetData("Inventory", pInventory))
			return false;

		std::vector<ItemInfo> pItemsInFOV;
		if (pBlackboard->GetData("ItemsInFOV", pItemsInFOV) == false || pItemsInFOV.empty())
		{
			return false;
		}

		IExamInterface* pInterface{ nullptr };
		if (!pBlackboard->GetData("Interface", pInterface))
			return false;
		auto agentInfo = pInterface->Agent_GetInfo();
		

		ItemInfo closestItem{};
		closestItem.Location = { FLT_MAX ,FLT_MAX };
		for (const ItemInfo& item : pItemsInFOV)
		{
			if (agentInfo.Position.Distance(item.Location) < agentInfo.Position.Distance(closestItem.Location))
			{
				closestItem = item;
			}
		}

		bool need{true};
			if (pInventory->HasItem(closestItem.Type))
				need = false;
			//take extra food
			if (closestItem.Type == eItemType::FOOD && !pInventory->IsFull())
			{
				if (pInventory->HowManyIHave(eItemType::FOOD) < 2 )
					need = true;
			}
			if (closestItem.Type == eItemType::MEDKIT && !pInventory->IsFull())
			{
				if (pInventory->HowManyIHave(eItemType::MEDKIT) < 2)
					need = true;
			}
			//take a extra gun
			/*if (closestItem.Type == eItemType::PISTOL || closestItem.Type == eItemType::SHOTGUN && !pInventory->IsFull())
			{
				if (pInventory->HowManyIHave(eItemType::PISTOL) < 2 && pInventory->HowManyIHave(eItemType::SHOTGUN) < 2)
					need = true;
			}*/
			
	
		return need;
	}

	bool IsHouseInFOVNew(Elite::Blackboard* pBlackboard)
	{
		std::vector<HouseInfo> pHousesInFOV;
		if (pBlackboard->GetData("HousesInFOV", pHousesInFOV) == false || pHousesInFOV.empty())
			return false;
		
		std::vector<HouseSearch> pHousesMemory;
		if (pBlackboard->GetData("HousesInMemory", pHousesMemory) == false)
			return false;
		
		if (pHousesMemory.empty())
			return true;

		for (const auto& houseInFOV : pHousesInFOV)
		{
			for (const auto& memory : pHousesMemory)
			{
				if (memory.Center == houseInFOV.Center)
				{
						return false;
				}
			}
		}
		return true;
		
	}

	bool DoesNeedFood(Elite::Blackboard* pBlackboard)
	{
		ItemManager* pInventory;
		if (!pBlackboard->GetData("Inventory", pInventory))
			return false;


		return (!pInventory->HasItem(eItemType::FOOD));		
	}
	
	bool DoesNeedMedKit(Elite::Blackboard* pBlackboard)
	{
		ItemManager* pInventory;
		if (!pBlackboard->GetData("Inventory", pInventory))
			return false;


		return (!pInventory->HasItem(eItemType::MEDKIT));
	}

	bool DoesNeedGun(Elite::Blackboard* pBlackboard)
	{
		ItemManager* pInventory;
		if (!pBlackboard->GetData("Inventory", pInventory))
			return false;

		return( !pInventory->HasItem(eItemType::PISTOL) && !pInventory->HasItem(eItemType::SHOTGUN) );
	}

	bool DoesNeedShotGun(Elite::Blackboard* pBlackboard)
	{
		ItemManager* pInventory;
		if (!pBlackboard->GetData("Inventory", pInventory))
			return false;

		return (!pInventory->HasItem(eItemType::SHOTGUN));
	}
	bool DoesNeedPistol(Elite::Blackboard* pBlackboard)
	{
		ItemManager* pInventory;
		if (!pBlackboard->GetData("Inventory", pInventory))
			return false;

		return (!pInventory->HasItem(eItemType::PISTOL));
	}

	bool RemembersFood(Elite::Blackboard* pBlackboard)
	{
		std::vector<ItemInfo> pMemory;
		if (!pBlackboard->GetData("ItemsInMemory", pMemory) || pMemory.empty())
			return false;

		for (const auto& item : pMemory)
		{
			if (item.Type == eItemType::FOOD)
				return true;
		}
		return false;
	}

	bool RemembersGun(Elite::Blackboard* pBlackboard)
	{
		std::vector<ItemInfo> pMemory;
		if (!pBlackboard->GetData("ItemsInMemory", pMemory) || pMemory.empty())
			return false;

		for (const auto& item : pMemory)
		{
			if (item.Type == eItemType::PISTOL || item.Type == eItemType::SHOTGUN)
				return true;
		}
		return false;
	}

	bool RemembersMedKit(Elite::Blackboard* pBlackboard)
	{
		std::vector<ItemInfo> pMemory;
		if (!pBlackboard->GetData("ItemsInMemory", pMemory) || pMemory.empty())
			return false;

		for (const auto& item : pMemory)
		{
			if (item.Type == eItemType::MEDKIT)
				return true;
		}
		return false;
	}

	bool RemembersPistol(Elite::Blackboard* pBlackboard)
	{
		std::vector<ItemInfo> pMemory;
		if (!pBlackboard->GetData("ItemsInMemory", pMemory) || pMemory.empty())
			return false;

		for (const auto& item : pMemory)
		{
			if (item.Type == eItemType::PISTOL)
				return true;
		}
		return false;
	}
	bool RemembersShotGun(Elite::Blackboard* pBlackboard)
	{
		std::vector<ItemInfo> pMemory;
		if (!pBlackboard->GetData("ItemsInMemory", pMemory) || pMemory.empty())
			return false;

		for (const auto& item : pMemory)
		{
			if (item.Type == eItemType::SHOTGUN)
				return true;
		}
		return false;
	}
	bool HouseCheckDone(Elite::Blackboard* pBlackboard)
	{
		std::vector<HouseSearch> pHousesMemory;
		if (pBlackboard->GetData("HousesInMemory", pHousesMemory) == false)
			return false;

		IExamInterface* pInterface{ nullptr };
		if (!pBlackboard->GetData("Interface", pInterface))
			return false;
		auto agentInfo = pInterface->Agent_GetInfo();

	
		HouseSearch closestHouse{};
		closestHouse.Center = { FLT_MAX ,FLT_MAX };
		for (const auto& house : pHousesMemory)
		{

			if (agentInfo.Position.Distance(house.Center) < agentInfo.Position.Distance(closestHouse.Center))
			{
				
					closestHouse = house;
			}

		}
		return closestHouse.FinishedSearch;
	}

	bool WasInside(Elite::Blackboard* pBlackboard)
	{
		bool wasInside;
		if (!pBlackboard->GetData("WasInside", wasInside))
			return false;

		return wasInside;
	}
	bool AllCellsExplored(Elite::Blackboard* pBlackboard)
	{
		Explorer* pExplorer{ nullptr };
		if (!pBlackboard->GetData("Explorer", pExplorer))
			return false;

		if (pExplorer->AllCellsVisited())
		{
			std::cout << "You explored the world! " << std::endl;
			return true;
		}
		else
			return false;
	}

	bool Desperate(Elite::Blackboard* pBlackboard)
	{
		std::vector<ItemInfo> pMemory;
		if (!pBlackboard->GetData("ItemsInMemory", pMemory))
			return false;

		ItemManager* pInventory;
		if (!pBlackboard->GetData("Inventory", pInventory))
			return false;

		IExamInterface* pInterface{ nullptr };
		if (!pBlackboard->GetData("Interface", pInterface))
			return false;
		auto agentInfo = pInterface->Agent_GetInfo();


		bool doesNotRemember{ true };
		for (const auto& item : pMemory)
		{
			if (item.Type == eItemType::MEDKIT)
				doesNotRemember = false;
		}

		if (agentInfo.Energy < 4 && !pInventory->HasItem(eItemType::FOOD))
			return true;

		if (agentInfo.Health < 4 && !pInventory->HasItem(eItemType::MEDKIT) && doesNotRemember)
			return true;

		return false;
	}
}