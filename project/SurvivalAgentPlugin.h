#pragma once
#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"
#include "Steering.h"
#include "EBlackboard.h"
#include "BehaviourTree/EBehaviorTree.h"
#include "Explorer.h"
#include "ExtendedStructs.h"

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

	std::vector<ItemInfo> m_ItemsMemory{};
	

	bool m_WasInside{false};
	float m_WasInsideTimer{ 0.f };

	float m_DangerTimer{ 0.f };
	bool m_RecentlyInDanger{false};

	std::vector<HouseSearch> m_HousesMemory{};
	

	SteeringPlugin_Output* m_pSteeringOutput{nullptr};
	Steering* m_pSteering{nullptr};

	Explorer* m_pExplorer{nullptr};

	std::vector<EnemyInfo> m_EnemiesInFov;
	std::vector<PurgeZoneInfo> m_PurgeZonesInFov;
	std::vector<HouseInfo> m_HousesInFov;
	std::vector<ItemInfo> m_ItemsInFov;

	Elite::BehaviorTree* m_pBehaviorTree{nullptr};



	//MY FUNCTIONS
	//initialization
	void InitializeBlackboard();
	void InitializeBT();

	//
	void GetEntitiesInFov();
	void UseResourcesIfNeeded();
	
	void InsideTimer(float dt);
	void DangerTimer(float dt);
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