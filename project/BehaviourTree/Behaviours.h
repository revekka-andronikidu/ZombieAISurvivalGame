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
		
	
	
		pSteering->Wander();
		pSteering->Run(false);	
		pSteering->SpinAround();
		
	
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

		//pSteering->SpinAround();
		pSteering->Flee(closestEnemy.Location);
		//pSteering->Face(closestEnemy.Location);

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

		std::vector<ItemInfo*> pMemory;
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
		pSteering->AutoOrient(true);

		if ((agentInfo.Position.Distance(closestItem.Location) < agentInfo.GrabRange ))
		{
			if (closestItem.Type == eItemType::GARBAGE)
			{
				pInterface->DestroyItem(closestItem);
				return Elite::BehaviorState::Success;
			}
			else 
			{
				for (int i{}; i < pMemory.size(); i++) //if item in memory erase
				{
					if (closestItem.ItemHash == pMemory[i]->ItemHash)
					{
						pMemory.erase(pMemory.begin() + i);
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

		std::vector<ItemInfo*> pMemory;
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
			if (item->Type == eItemType::FOOD)
			{
				if (agentInfo.Position.Distance(item->Location) < agentInfo.Position.Distance(closestFood.Location))
				{
					closestFood.Location = item->Location;
				}
			}
		}

		pSteering->Seek(closestFood.Location);
		pSteering->SpinAround();
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState SeekClosestMedKit(Elite::Blackboard* pBlackboard)
	{

		std::vector<ItemInfo*> pMemory;
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
			if (item->Type == eItemType::MEDKIT)
			{
				if (agentInfo.Position.Distance(item->Location) < agentInfo.Position.Distance(closestMedkit.Location))
				{
					closestMedkit.Location = item->Location;
				}
			}
		}

		pSteering->Seek(closestMedkit.Location);
		pSteering->SpinAround();
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState SeekClosestPistol(Elite::Blackboard* pBlackboard)
	{

		std::vector<ItemInfo*> pMemory;
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

		for (auto item : pMemory)
		{
			if (item->Type == eItemType::PISTOL)
			{
				if (agentInfo.Position.Distance(item->Location) < agentInfo.Position.Distance(closestPistol.Location))
				{
					closestPistol.Location = item->Location;
				}
			}
		}

		pSteering->Seek(closestPistol.Location);
		pSteering->SpinAround();
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState SeekClosestShotgun(Elite::Blackboard* pBlackboard)
	{

		std::vector<ItemInfo*> pMemory;
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

		for (auto item : pMemory)
		{
			if (item->Type == eItemType::SHOTGUN)
			{
				if (agentInfo.Position.Distance(item->Location) < agentInfo.Position.Distance(closestShotgun.Location))
				{
					closestShotgun.Location = item->Location;
				}
			}
		}

		pSteering->Seek(closestShotgun.Location);
		pSteering->SpinAround();
		return Elite::BehaviorState::Success;
	}
	Elite::BehaviorState SeekClosestGun(Elite::Blackboard* pBlackboard)
	{

		std::vector<ItemInfo*> pMemory;
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

		for (auto item : pMemory)
		{
			if (item->Type == eItemType::PISTOL || item->Type == eItemType::SHOTGUN)
			{
				if (agentInfo.Position.Distance(item->Location) < agentInfo.Position.Distance(closestGun.Location))
				{
					closestGun.Location = item->Location;
				}
			}
		}

		pSteering->Seek(closestGun.Location);
		pSteering->SpinAround();
		return Elite::BehaviorState::Success;
	}



	Elite::BehaviorState RememberItem(Elite::Blackboard* pBlackboard)
	{
		std::vector<ItemInfo> pItemsInFOV;
		if (pBlackboard->GetData("ItemsInFOV", pItemsInFOV) == false || pItemsInFOV.empty())
			return Elite::BehaviorState::Failure;

		std::vector<ItemInfo*> pMemory;
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
		
		for (auto item : pMemory)
		{
			if (item == &closestItem) //if item already in memory return
				return Elite::BehaviorState::Success;
		}

		pMemory.push_back(&closestItem);
		pBlackboard->ChangeData("ItemsInMemory", pMemory);
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
		std::cout << "Enemy in FOV" << std::endl;
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

	bool WasBitten(Elite::Blackboard* pBlackboard) //was bitten or enemy in fov
	{
		IExamInterface* pInterface{ nullptr };
		if (!pBlackboard->GetData("Interface", pInterface))
			return false;


		auto agentInfo = pInterface->Agent_GetInfo();
		if (agentInfo.WasBitten)
			return true;
	}
	
	bool IsInDanger(Elite::Blackboard* pBlackboard) //was bitten or enemy in fov
	{
		return (WasBitten(pBlackboard) || IsEnemyInFOV(pBlackboard));
		std::cout << "In Danger" << std::endl;
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
			
	
		return need;
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

		
		return (!pInventory->HasItem(eItemType::PISTOL) && !pInventory->HasItem(eItemType::SHOTGUN)); 
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
		std::vector<ItemInfo*> pMemory;
		if (!pBlackboard->GetData("ItemsInMemory", pMemory) || pMemory.empty())
			return false;

		for (auto item : pMemory)
		{
			if (item->Type == eItemType::FOOD)
				return true;
		}
		return false;
	}

	bool RemembersGun(Elite::Blackboard* pBlackboard)
	{
		std::vector<ItemInfo*> pMemory;
		if (!pBlackboard->GetData("ItemsInMemory", pMemory) || pMemory.empty())
			return false;

		for (auto item : pMemory)
		{
			if (item->Type == eItemType::PISTOL || item->Type == eItemType::SHOTGUN)
				return true;
		}
		return false;
	}

	bool RemembersMedKit(Elite::Blackboard* pBlackboard)
	{
		std::vector<ItemInfo*> pMemory;
		if (!pBlackboard->GetData("ItemsInMemory", pMemory) || pMemory.empty())
			return false;

		for (auto item : pMemory)
		{
			if (item->Type == eItemType::MEDKIT)
				return true;
		}
		return false;
	}

	bool RemembersPistol(Elite::Blackboard* pBlackboard)
	{
		std::vector<ItemInfo*> pMemory;
		if (!pBlackboard->GetData("ItemsInMemory", pMemory) || pMemory.empty())
			return false;

		for (auto item : pMemory)
		{
			if (item->Type == eItemType::PISTOL)
				return true;
		}
		return false;
	}
	bool RemembersShotGun(Elite::Blackboard* pBlackboard)
	{
		std::vector<ItemInfo*> pMemory;
		if (!pBlackboard->GetData("ItemsInMemory", pMemory) || pMemory.empty())
			return false;

		for (auto item : pMemory)
		{
			if (item->Type == eItemType::SHOTGUN)
				return true;
		}
		return false;
	}
}