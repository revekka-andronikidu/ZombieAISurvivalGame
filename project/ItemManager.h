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

	ItemManager(const ItemManager& itemManager) = delete;
	ItemManager(ItemManager&& itemManager) = delete;
	ItemManager& operator=(const ItemManager& itemManager) = delete;
	ItemManager& operator=(ItemManager&& itemManager) = delete;

	UINT GetFreeSlot() const;
	bool IsFull( ) const;

	void UseMedKit();
	void UseShotGun();
	void UsePistol();
	void UseGun(); 
	void UseFood();
	int HowManyIHave(eItemType itemType) const;
		

	void AddItem(ItemInfo itemInfo);
	bool HasItem(eItemType itemType) const;
	UINT GetSlotWithItem(eItemType itemType) const;

private:
	IExamInterface* m_pInterface;
	std::vector< eItemType> m_Inventory{};
	UINT m_Full;
};

