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
	m_pBlackboard = new Elite::Blackboard();
	m_pSteeringOutput = new SteeringPlugin_Output{};
	m_pSteering = new Steering(m_pInterface, m_pSteeringOutput);

	
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

	//delete m_pBlackboard; //deleted by behaviour tree
	
	

	//delete 
	/*for (auto pistol : m_pPistolsLoot)
	{
		delete pistol;
		pistol = nullptr;
	}
	m_pPistolsLoot.clear();

	for (auto shotGun : m_pShotGunsLoot)
	{
		delete shotGun;
		shotGun = nullptr;
	}
	for (auto medKit : m_pMedKitsLoot)
	{
		delete medKit;
		medKit = nullptr;
	}
	for (auto food : m_pFoodLoot)
	{
		delete food;
		food = nullptr;
	}
	for (auto house : m_pHousesMemory)
	{
		delete house;
		house = nullptr;
	}*/
	
}

//Called only once, during initialization. Only works in DEBUG Mode
void SurvivalAgentPlugin::InitGameDebugParams(GameDebugParams& params)
{
	params.AutoFollowCam = true; //Automatically follow the AI? (Default = true)
	params.RenderUI = true; //Render the IMGUI Panel? (Default = true)
	params.SpawnEnemies = true; //Do you want to spawn enemies? (Default = true)
	params.EnemyCount = 20; //How many enemies? (Default = 20)
	params.GodMode = true; //GodMode > You can't die, can be useful to inspect certain behaviors (Default = false)
	params.LevelFile = "GameLevel.gppl";
	params.AutoGrabClosestItem = true; //A call to Item_Grab(...) returns the closest item that can be grabbed. (EntityInfo argument is ignored)
	params.StartingDifficultyStage = 1;
	params.InfiniteStamina = false;
	params.SpawnDebugPistol = true;
	params.SpawnDebugShotgun = true;
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
	if (m_pInterface->Input_IsMouseButtonUp(Elite::InputMouseButton::eLeft))
	{
		//Update_Debug target based on input
		Elite::MouseData mouseData = m_pInterface->Input_GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eLeft);
		const Elite::Vector2 pos = Elite::Vector2(static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y));
		m_Target = m_pInterface->Debug_ConvertScreenToWorld(pos);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Space))
	{
		m_CanRun = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Left))
	{
		m_AngSpeed -= Elite::ToRadians(10);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Right))
	{
		m_AngSpeed += Elite::ToRadians(10);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_G))
	{
		m_GrabItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_U))
	{
		m_UseItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_R))
	{
		m_RemoveItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_X))
	{
		m_DestroyItemsInFOV = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyUp(Elite::eScancode_Space))
	{
		m_CanRun = false;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Delete))
	{
		m_pInterface->RequestShutdown();
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_KP_Minus))
	{
		if (m_InventorySlot > 0)
			--m_InventorySlot;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_KP_Plus))
	{
		if (m_InventorySlot < 4)
			++m_InventorySlot;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Q))
	{
		ItemInfo info = {};
		m_pInterface->Inventory_GetItem(m_InventorySlot, info);
		std::cout << (int)info.Type << std::endl;
	}
}

//This function calculates the new SteeringOutput, called once per frame
SteeringPlugin_Output SurvivalAgentPlugin::UpdateSteering(float dt)
{
	
	GetEntitiesInFov();
	UseResourcesIfNeeded(); //use food or med kit if needs too
	
	
	m_pBlackboard->ChangeData("Interface", m_pInterface); //update interface
	//m_pBlackboard->ChangeData("")

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
	//items
	m_pBlackboard->AddData("Inventory", m_pItemManager);
	/*m_pBlackboard->AddData("Medkits", m_pMedKitsLoot);
	m_pBlackboard->AddData("Pistols", m_pPistolsLoot);
	m_pBlackboard->AddData("ShotGuns", m_pShotGunsLoot);
	m_pBlackboard->AddData("Food", m_pFoodLoot);

	m_pBlackboard->AddData("Houses", m_pHousesMemory);*/

	m_pBlackboard->AddData("Interface", m_pInterface);
	m_pBlackboard->AddData("SteeringOutput", m_pSteeringOutput);
	m_pBlackboard->AddData("Steering", m_pSteering);
	m_pBlackboard->AddData("EnemiesInFOV", m_EnemiesInFov);
	m_pBlackboard->AddData("PurgeZonesInFOV", m_PurgeZonesInFov);
	m_pBlackboard->AddData("HousesInFOV", m_HousesInFov);

	

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
							new Elite::BehaviorConditional{BT_Conditions::IsPurgeZoneInFOV},
							new Elite::BehaviorAction{ BT_Actions::FleePurgeZone}
						}
					},

					// Try to shoot enemies
					//new Elite::BehaviorSequence
					//{
					//	{
					//		new Elite::BehaviorConditional{ BT_Conditions::IsEnemyInFOV },
					//		new Elite::BehaviorConditional{ BT_Conditions::HasAGun },
					//		//new Elite::BehaviorAction{ BT_Actions::AimAndShoot }
					//	}
					//},
		//Explore if nothing else to do
		new Elite::BehaviorAction{ BT_Actions::Explore }
		//new Elite::BehaviorAction{ BT_Actions::RevisitHouses }


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
}

void SurvivalAgentPlugin::UseResourcesIfNeeded()
{
	if (m_pInterface->Agent_GetInfo().Health);
}

