#pragma once
#include "Exam_HelperStructs.h"
#include "IExamInterface.h"
#include <vector>

//class IExamInterface;

class ItemManager final
{
public:

	ItemManager(IExamInterface* pInterface);
	~ItemManager() = default;

	ItemManager(const ItemManager& inventoryManager) = delete;
	ItemManager(ItemManager&& inventoryManager) = delete;
	ItemManager& operator=(const ItemManager& inventoryManager) = delete;
	ItemManager& operator=(ItemManager&& inventoryManager) = delete;

	UINT GetFreeSlot() const;
	bool IsFull( ) const;

	void UseMedKit();
	void UseShotGun();
	void UsePistol();
	void UseGun(); 
	void UseFood();
	

	void AddItem(ItemInfo itemInfo);
	bool HasItem(eItemType itemType) const;
	UINT GetSlotWithItem(eItemType itemType) const;

private:
	IExamInterface* m_pInterface;
	std::vector< eItemType> m_Inventory;
	UINT m_Full;
};

