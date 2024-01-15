#include "stdafx.h"
#include "SurvivalAgentPlugin.h"
#include "IExamInterface.h"
#include "ItemManager.h"
#include "BehaviourTree/Behaviours.h"


using namespace std;

//Called only once, during initialization
void SurvivalAgentPlugin::Initialize(IBaseInterface* pInterface, PluginInfo& info)
{
	//Retrieving the interface
	//This interface gives you access to certain actions the AI_Framework can perform for you
	m_pInterface = static_cast<IExamInterface*>(pInterface);

	//Information for the leaderboards!
	info.BotName = "MinionExam";
	info.Student_Name = "Revekka"; //No special characters allowed. Highscores won't work with special characters.
	info.Student_Class = "2DAE09";
	info.LB_Password = "Revekka!";//Don't use a real password! This is only to prevent other students from overwriting your highscore!

	//----------------------------
	m_pItemManager = new ItemManager(m_pInterface);
	m_pExplorer = new Explorer(m_pInterface);
	m_pBlackboard = new Elite::Blackboard();
	m_pSteeringOutput = new SteeringPlugin_Output{};
	m_pSteering = new Steering(m_pInterface, m_pSteeringOutput);

	m_ItemsMemory.clear();
	
	InitializeBlackboard();

	//create behaviour tree
	InitializeBT();
}

//Called only once
void SurvivalAgentPlugin::DllInit()
{
	//Called when the plugin is loaded
}

//Called only once
void SurvivalAgentPlugin::DllShutdown()
{
	//Called when the plugin gets unloaded
	SAFE_DELETE(m_pItemManager);
	SAFE_DELETE(m_pBehaviorTree);
	SAFE_DELETE(m_pSteeringOutput);
	SAFE_DELETE(m_pSteering);
	SAFE_DELETE(m_pExplorer);
	


	//delete m_pBlackboard; //deleted by behaviour tree
	
	
}

//Called only once, during initialization. Only works in DEBUG Mode
void SurvivalAgentPlugin::InitGameDebugParams(GameDebugParams& params)
{
	params.AutoFollowCam = true; //Automatically follow the AI? (Default = true)
	params.RenderUI = true; //Render the IMGUI Panel? (Default = true)
	params.SpawnEnemies = true; //Do you want to spawn enemies? (Default = true)
	params.EnemyCount = 20; //How many enemies? (Default = 20)
	params.GodMode = false; //GodMode > You can't die, can be useful to inspect certain behaviors (Default = false)
	params.LevelFile = "GameLevel.gppl";
	params.AutoGrabClosestItem = true; //A call to Item_Grab(...) returns the closest item that can be grabbed. (EntityInfo argument is ignored)
	params.StartingDifficultyStage = 1;
	params.InfiniteStamina = false;
	params.SpawnDebugPistol = false;
	params.SpawnDebugShotgun = false;
	params.SpawnPurgeZonesOnMiddleClick = true;
	params.PrintDebugMessages = true;
	params.ShowDebugItemNames = true;
	params.Seed = 0; //-1 = don't set seed. Any other number = fixed seed //TIP: use Seed = int(time(nullptr)) for pure randomness
}

//Only Active in DEBUG Mode
//(=Use only for Debug Purposes)
void SurvivalAgentPlugin::Update_Debug(float dt)
{
	//Demo Event Code
	//In the end your Agent should be able to walk around without external input
	//if (m_pInterface->Input_IsMouseButtonUp(Elite::InputMouseButton::eLeft))
	//{
	//	//Update_Debug target based on input
	//	Elite::MouseData mouseData = m_pInterface->Input_GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eLeft);
	//	const Elite::Vector2 pos = Elite::Vector2(static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y));
	//	m_Target = m_pInterface->Debug_ConvertScreenToWorld(pos);
	//}
	//else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Space))
	//{
	//	m_CanRun = true;
	//}
	//else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Left))
	//{
	//	m_AngSpeed -= Elite::ToRadians(10);
	//}
	//else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Right))
	//{
	//	m_AngSpeed += Elite::ToRadians(10);
	//}
	//else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_G))
	//{
	//	m_GrabItem = true;
	//}
	//else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_U))
	//{
	//	m_UseItem = true;
	//}
	//else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_R))
	//{
	//	m_RemoveItem = true;
	//}
	//else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_X))
	//{
	//	m_DestroyItemsInFOV = true;
	//}
	//else if (m_pInterface->Input_IsKeyboardKeyUp(Elite::eScancode_Space))
	//{
	//	m_CanRun = false;
	//}
	//else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Delete))
	//{
	//	m_pInterface->RequestShutdown();
	//}
	//else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_KP_Minus))
	//{
	//	if (m_InventorySlot > 0)
	//		--m_InventorySlot;
	//}
	//else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_KP_Plus))
	//{
	//	if (m_InventorySlot < 4)
	//		++m_InventorySlot;
	//}
	//else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Q))
	//{
	//	ItemInfo info = {};
	//	m_pInterface->Inventory_GetItem(m_InventorySlot, info);
	//	std::cout << (int)info.Type << std::endl;
	//}

	m_pExplorer->DrawGrid();
}

//This function calculates the new SteeringOutput, called once per frame
SteeringPlugin_Output SurvivalAgentPlugin::UpdateSteering(float dt)
{
	
	GetEntitiesInFov();
	UseResourcesIfNeeded(); //use food or med kit if needs too
	
	
	m_pBlackboard->ChangeData("Interface", m_pInterface); //update interface
	InsideTimer(dt);
	DangerTimer(dt);

	m_pExplorer->Update(); //update exploration grid

	//update decisions and select highrst priority task
	m_pBehaviorTree->Update(dt);
	
	//update steering
	
	m_pBlackboard->GetData("SteeringOutput", m_pSteeringOutput);



	return *m_pSteeringOutput;
}

//This function should only be used for rendering debug elements
void SurvivalAgentPlugin::Render(float dt) const
{
	//This Render function should only contain calls to Interface->Draw_... functions
	m_pInterface->Draw_SolidCircle(m_Target, .7f, { 0,0 }, { 1, 0, 0 });
}


void SurvivalAgentPlugin::InitializeBlackboard()
{
	m_pBlackboard->AddData("DeltaTime", 0.0f);
	
	m_pBlackboard->AddData("Inventory", m_pItemManager);
	m_pBlackboard->AddData("Interface", m_pInterface);
	m_pBlackboard->AddData("Explorer", m_pExplorer);
	
	m_pBlackboard->AddData("ItemsInMemory", m_ItemsMemory); 
	m_pBlackboard->AddData("HousesInMemory", m_HousesMemory);
	
	m_pBlackboard->AddData("WasInside", m_WasInside);
	m_pBlackboard->AddData("InDanger", m_RecentlyInDanger);

	m_pBlackboard->AddData("SteeringOutput", m_pSteeringOutput); 
	m_pBlackboard->AddData("Steering", m_pSteering); 

	m_pBlackboard->AddData("EnemiesInFOV", m_EnemiesInFov);
	m_pBlackboard->AddData("PurgeZonesInFOV", m_PurgeZonesInFov);
	m_pBlackboard->AddData("HousesInFOV", m_HousesInFov);
	m_pBlackboard->AddData("ItemsInFOV", m_ItemsInFov);
}

void SurvivalAgentPlugin::InitializeBT()
{
	Elite::BehaviorTree* pBehaviorTree
	{
		new Elite::BehaviorTree
		{
			m_pBlackboard,
			new Elite::BehaviorSelector
			{
				{
					//flee purge zone
					new Elite::BehaviorSequence
					{
						{
							new Elite::BehaviorConditional{&BT_Conditions::IsPurgeZoneInFOV},
							new Elite::BehaviorAction{&BT_Actions::FleePurgeZone}
						}
					},
					new Elite::BehaviorSequence //Enemies
					(
					{
						//Enemy in FOV or was Bitten 
						new Elite::BehaviorSelector
						(
							{
								new Elite::BehaviorSequence // Try to shoot enemies
								(
									{
										new Elite::BehaviorConditional{ &BT_Conditions::IsEnemyInFOV },
										new Elite::BehaviorConditional{ &BT_Conditions::HasAGun },
										new Elite::BehaviorAction{ &BT_Actions::AimAndShoot }
									}
								),
								new Elite::BehaviorSequence // Hide in a house
								(
									{
										new Elite::BehaviorInvertConditional{ &BT_Conditions::HasAGun},
										new Elite::BehaviorInvertConditional{ &BT_Conditions::WasInside},
										new Elite::BehaviorConditional(&BT_Conditions::IsInDanger),
										new Elite::BehaviorAction{ &BT_Actions::HideInHouse}
									}
								),
								new Elite::BehaviorSequence //flee
								(
									{
										new Elite::BehaviorInvertConditional{ &BT_Conditions::HasAGun },
										new Elite::BehaviorConditional(&BT_Conditions::IsInDanger),
										new Elite::BehaviorAction(&BT_Actions::FleeFromEnemy)
									}
								),
								
							}
						)
					}
					),
					//Items
					new Elite::BehaviorSequence
					(
					{
						new Elite::BehaviorConditional(&BT_Conditions::IsItemInFOV),
						new Elite::BehaviorSelector //take item if we need it
						(
							{
								new Elite::BehaviorSequence
								(
								{
									new Elite::BehaviorConditional{&BT_Conditions::NeedClosestItem},
									new Elite::BehaviorAction{&BT_Actions::TakeTheItem},//destroy garbage
								}
							),
							new Elite::BehaviorSequence //remember spot if we have item
							(
								{
									new Elite::BehaviorInvertConditional{BT_Conditions::NeedClosestItem},
									new Elite::BehaviorAction(BT_Actions::RememberItem)
								}
							),
							}
						)
					}
					),
					new Elite::BehaviorSequence //search for things from memory
					(
					{
					 new Elite::BehaviorSelector //take item if we need it
						(
							{
						new Elite::BehaviorSequence  // search food 
						(
							{
								new Elite::BehaviorConditional{&BT_Conditions::DoesNeedFood},
								new Elite::BehaviorConditional{&BT_Conditions::RemembersFood},
								new Elite::BehaviorAction{&BT_Actions::SeekClosestFood}
							}
						),
						new Elite::BehaviorSequence  //search closest gun when has neither
						(
							{
								new Elite::BehaviorConditional{&BT_Conditions::DoesNeedGun},
								new Elite::BehaviorConditional{&BT_Conditions::RemembersGun},
								new Elite::BehaviorAction{&BT_Actions::SeekClosestGun}
							}
						),
						new Elite::BehaviorSequence  //search medkit
						(
							{
								new Elite::BehaviorConditional{&BT_Conditions::DoesNeedMedKit},
								new Elite::BehaviorConditional{&BT_Conditions::RemembersMedKit},
								new Elite::BehaviorAction{&BT_Actions::SeekClosestMedKit}
							}
						),
						new Elite::BehaviorSequence  //search closest pistol 
						(
							{
								new Elite::BehaviorConditional{&BT_Conditions::DoesNeedPistol},
								new Elite::BehaviorConditional{&BT_Conditions::RemembersPistol},
								new Elite::BehaviorAction{&BT_Actions::SeekClosestPistol}
							}
						),
						new Elite::BehaviorSequence  //search closest shotgun 						
						(
							{
								new Elite::BehaviorConditional{&BT_Conditions::DoesNeedShotGun},
								new Elite::BehaviorConditional{&BT_Conditions::RemembersShotGun},
								new Elite::BehaviorAction{&BT_Actions::SeekClosestShotgun}
							}
						),
					 }
					)
						
					}
					),
					new Elite::BehaviorSelector //search houses in fov
					(
					{
						new Elite::BehaviorSequence
						(
							{
								new Elite::BehaviorConditional{&BT_Conditions::IsHouseInFOVNew},
								new Elite::BehaviorAction{&BT_Actions::RememberHouse}
							}
						),
											
					}
					),
					new Elite::BehaviorAction{ &BT_Actions::SearchClosestHouseInMemory },
					new Elite::BehaviorSelector //search houses in fov
					(
					{
						new Elite::BehaviorSequence
						(
							{
								//new Elite::BehaviorConditional{&BT_Conditions::CellsToExplore},
								new Elite::BehaviorAction{ &BT_Actions::Explore}
								
							}
						),

					}
					),
					
					new Elite::BehaviorAction{ &BT_Actions::RevisitHouses}
					
					//if dying (desperately needs something, revisit houses == maybe put it with behaviours above?

					//Explore if nothing else to do
					//if not all tiles explored
					

					
				}
			}
		}
	};

	m_pBehaviorTree = pBehaviorTree;
}

void SurvivalAgentPlugin::GetEntitiesInFov()
{
	//Get Enemies in FOV
	m_EnemiesInFov = m_pInterface->GetEnemiesInFOV();
	m_pBlackboard->ChangeData("EnemiesInFOV", m_EnemiesInFov);

	//Get Purge Zones in FOV
	m_PurgeZonesInFov = m_pInterface->GetPurgeZonesInFOV();
	m_pBlackboard->ChangeData("PurgeZonesInFOV", m_PurgeZonesInFov);

	//Get Houses in FOV
	m_HousesInFov = m_pInterface->GetHousesInFOV();
	m_pBlackboard->ChangeData("HousesInFOV", m_HousesInFov);

	m_ItemsInFov = m_pInterface->GetItemsInFOV();
	m_pBlackboard->ChangeData("ItemsInFOV", m_ItemsInFov);
}

void SurvivalAgentPlugin::UseResourcesIfNeeded()
{
	ItemInfo item{};
	if (m_pItemManager->HasItem(eItemType::MEDKIT))
	{
		m_pInterface->Inventory_GetItem(m_pItemManager->GetSlotWithItem(eItemType::MEDKIT), item);

		if (10.f - m_pInterface->Agent_GetInfo().Health >= item.Value)
			m_pItemManager->UseMedKit();
	}

	if (m_pItemManager->HasItem(eItemType::FOOD))
	{
		
		m_pInterface->Inventory_GetItem(m_pItemManager->GetSlotWithItem(eItemType::FOOD), item);
		
		if (10.f - m_pInterface->Agent_GetInfo().Energy >= item.Value )
		m_pItemManager->UseFood();
	}

}


void SurvivalAgentPlugin::InsideTimer(float dt)
{

	if (m_pInterface->Agent_GetInfo().IsInHouse)
	{
		m_WasInside = true;
		m_WasInsideTimer = 0.f;
	}
	else
	{
		m_WasInsideTimer += dt;
		if (m_WasInsideTimer >= 10.f)
		{

			m_WasInside = false;
		}
	}
	m_pBlackboard->ChangeData("WasInside", m_WasInside);

}	

void SurvivalAgentPlugin::DangerTimer(float dt)
{
	
	
	if (m_pInterface->GetEnemiesInFOV().size() > 0)
	{
		m_RecentlyInDanger = true;
		m_DangerTimer = 0.f;
	}
	else
	{
		m_DangerTimer += dt;
		if (m_DangerTimer >= 5.f)
		{

			m_RecentlyInDanger = false;
		}
	}
	m_pBlackboard->ChangeData("InDanger", m_RecentlyInDanger);

}