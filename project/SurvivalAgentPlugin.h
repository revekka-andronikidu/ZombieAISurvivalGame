#pragma once
#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"
#include "Steering.h"
#include "EBlackboard.h"
#include "BehaviourTree/EBehaviorTree.h"
#include "Explorer.h"

class IBaseInterface;
class IExamInterface;
class ItemManager;


class SurvivalAgentPlugin :public IExamPlugin
{
public:
	SurvivalAgentPlugin() {};
	virtual ~SurvivalAgentPlugin() {};

	void Initialize(IBaseInterface* pInterface, PluginInfo& info) override;
	void DllInit() override;
	void DllShutdown() override;

	void InitGameDebugParams(GameDebugParams& params) override;
	void Update_Debug(float dt) override;

	SteeringPlugin_Output UpdateSteering(float dt) override;
	void Render(float dt) const override;

private:
	//Interface, used to request data from/perform actions with the AI Framework
	IExamInterface* m_pInterface = nullptr;

	Elite::Vector2 m_Target = {};
	bool m_CanRun = false; //Demo purpose
	bool m_GrabItem = false; //Demo purpose
	bool m_UseItem = false; //Demo purpose
	bool m_RemoveItem = false; //Demo purpose
	bool m_DestroyItemsInFOV = false;
	float m_AngSpeed = 0.f; //Demo purpose

	UINT m_InventorySlot = 0;

	//MY VARIABLES
	ItemManager* m_pItemManager {nullptr};
	Elite::Blackboard* m_pBlackboard{nullptr};

	//Items found == create exlorer for this
	/*std::vector<ItemInfo*> m_pPistolsLoot;
	std::vector<ItemInfo*> m_pShotGunsLoot;
	std::vector<ItemInfo*> m_pMedKitsLoot;
	std::vector<ItemInfo*> m_pFoodLoot;

	std::vector<HouseInfo*> m_pHousesMemory;*/

	SteeringPlugin_Output* m_pSteeringOutput;
	Steering* m_pSteering;

	Explorer* m_pExplorer;

	std::vector<EnemyInfo> m_EnemiesInFov;
	std::vector<PurgeZoneInfo> m_PurgeZonesInFov;
	std::vector<HouseInfo> m_HousesInFov;
	std::vector<ItemInfo> m_ItemsInFov;

	Elite::BehaviorTree* m_pBehaviorTree;



	//MY FUNCTIONS
	//initialization
	void InitializeBlackboard();
	void InitializeBT();

	//
	void GetEntitiesInFov();
	void UseResourcesIfNeeded();
	
};

//ENTRY
//This is the first function that is called by the host program
//The plugin returned by this function is also the plugin used by the host program
extern "C"
{
	__declspec (dllexport) IPluginBase* Register()
	{
		return new SurvivalAgentPlugin();
	}
}