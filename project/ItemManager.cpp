#include "stdafx.h"
#include "ItemManager.h"

ItemManager::ItemManager(IExamInterface* pInterface)
	: m_pInterface(pInterface)
{
	m_Inventory.resize(m_pInterface->Inventory_GetCapacity());
	m_Full = m_pInterface->Inventory_GetCapacity();

	for (size_t idx{}; idx < m_Inventory.size(); idx++)
	{	
		m_Inventory[idx] = eItemType::RANDOM_DROP;
	}
}

UINT ItemManager::GetFreeSlot() const
{
	ItemInfo itemInfo;
	for (UINT i{ 0 }; i < m_pInterface->Inventory_GetCapacity(); ++i)
	{
		if (!m_pInterface->Inventory_GetItem(i, itemInfo))
			return i;
	}

	return m_Full;
}

UINT ItemManager::GetSlotWithItem(eItemType itemType) const
{
	UINT slot = std::distance(m_Inventory.begin(), std::find(m_Inventory.begin(), m_Inventory.end(), itemType));
	return slot;
}


bool ItemManager::IsFull() const
{
	if (GetFreeSlot() == m_Full)
	{
		std::wcout << L"Inventory is full!\n";
		return true;
	}
	else
		return false;
}

void ItemManager::UseMedKit() //will check if med Kit aviable and HP  > treshold in agent manager
{
	if (!HasItem(eItemType::MEDKIT))
	{
		std::wcout << L"No MedKit in inventory!\n";
		return;
	}

	UINT slot = GetSlotWithItem(eItemType::MEDKIT);

	ItemInfo itemInfo{};
	if (m_pInterface->Inventory_GetItem(slot, itemInfo))
	{
		m_pInterface->Inventory_UseItem(slot);
		m_pInterface->Inventory_RemoveItem(slot);
		m_Inventory.at(slot) = eItemType::RANDOM_DROP; //temp will be rewriten when new item is added
	}
}

void ItemManager::UseShotGun()
{
	if (!HasItem(eItemType::SHOTGUN))
	{
		std::wcout << L"No Gun in the inventory!\n";
		return;
	}

	UINT slot = GetSlotWithItem(eItemType::SHOTGUN);

	ItemInfo itemInfo{};
	if (m_pInterface->Inventory_GetItem(slot, itemInfo))
	{
		m_pInterface->Inventory_UseItem(slot);
		//std::wcout << L"Pew Pew!\n";

		if (itemInfo.Value <= 0)
		{
			m_pInterface->Inventory_RemoveItem(slot);
			m_Inventory.at(slot) = eItemType::RANDOM_DROP;
			std::wcout << L"Gun discarted, out of ammo!\n";
		}
	}
}
void ItemManager::UsePistol()
{
	if (!HasItem(eItemType::PISTOL))
	{
		std::wcout << L"No Gun in the inventory!\n";
		return;
	}

	UINT slot = GetSlotWithItem(eItemType::PISTOL);

	ItemInfo itemInfo{};
	if (m_pInterface->Inventory_GetItem(slot, itemInfo))
	{
		m_pInterface->Inventory_UseItem(slot);
		//std::wcout << L"Pew Pew!\n";

		if (itemInfo.Value <= 0.f)
		{
			m_pInterface->Inventory_RemoveItem(slot);
			m_Inventory.at(slot) = eItemType::RANDOM_DROP;
			std::wcout << L"Gun discarted, out of ammo!\n";
		}
	}

};
void ItemManager::UseGun() //in agent use a gun after aiming to the target
{
	if (!HasItem(eItemType::PISTOL) && !HasItem(eItemType::SHOTGUN))
	{
		std::wcout << L"No Gun in the inventory!\n";
		return;
	}
	eItemType gunType{};
	if (HasItem(eItemType::SHOTGUN)) //shotgun preference //maybe will change depending on zombie type
		gunType = eItemType::SHOTGUN;
	else
		gunType = eItemType::PISTOL;

	UINT slot = GetSlotWithItem(gunType);

	ItemInfo itemInfo{};
	if (m_pInterface->Inventory_GetItem(slot, itemInfo))
	{
		m_pInterface->Inventory_UseItem(slot);
		//std::wcout << L"Pew Pew!\n";

		if (itemInfo.Value <= 0.f)
		{
			m_pInterface->Inventory_RemoveItem(slot);
			m_Inventory.at(slot) = eItemType::RANDOM_DROP;
			std::wcout << L"Gun discarted, out of ammo!\n";
		}
	}
}
void ItemManager::UseFood() 
{
	if (!HasItem(eItemType::FOOD))
	{
		std::wcout << L"No food in inventory!\n";
		return;
	}

	UINT slot = GetSlotWithItem(eItemType::FOOD);
	ItemInfo itemInfo{};
	if (m_pInterface->Inventory_GetItem(slot, itemInfo))
	{
		m_pInterface->Inventory_UseItem(slot);
		std::wcout << L"Nom Nom!\n";

		m_pInterface->Inventory_RemoveItem(slot);
		m_Inventory.at(slot) = eItemType::RANDOM_DROP; //temp will be rewriten when new item is added
	}

}

void ItemManager::AddItem(ItemInfo ItemInfo) //needs to be grabbed by agent first
{
	//add item into empty slot
	UINT slot = GetFreeSlot();
	if (slot != m_Full)
	{
		if (ItemInfo.Type == eItemType::GARBAGE) //removes garbage when grabbed // make agent not to pick up garbage
		{
			m_pInterface->Inventory_AddItem(slot, ItemInfo);
			m_pInterface->Inventory_RemoveItem(slot);
			return;
		}

		m_pInterface->Inventory_AddItem(slot, ItemInfo);
		m_Inventory[slot] = ItemInfo.Type;

	}
}

bool ItemManager::HasItem(eItemType itemType) const
{
	for (eItemType item : m_Inventory)
	{
		if (item == itemType)
		{
			return true;
		}

	}
	return false;
}



int ItemManager::HowManyIHave(eItemType itemType) const
{
	int count{ 0 };
	for (eItemType item : m_Inventory)
	{
		if (item == itemType)
		{
			count++;
		}

	}
	return count;
}