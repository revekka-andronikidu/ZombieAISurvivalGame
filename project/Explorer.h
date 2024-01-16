#pragma once
#include "Exam_HelperStructs.h"
#include "IExamInterface.h"
#include <vector>
#include "ExtendedStructs.h"

class Explorer final
{
public:
	Explorer(IExamInterface* pInterface);
	~Explorer() = default;

	Explorer(const Explorer& explorer) = delete;
	Explorer(Explorer&& explorer) = delete;
	Explorer& operator=(const Explorer& explorer) = delete;
	Explorer& operator=(Explorer&& explorer) = delete;

	void DrawGrid() const;
	void Update();
	int PositionToIdx(const Elite::Vector2& position) const;
	Elite::Vector2 NextClosestCell();
	bool AllCellsVisited();
	

private:

	

	IExamInterface* m_pInterface = nullptr;
	std::vector<Cell> m_Cells;
	std::vector<int> m_CurrentCellsIdx;
	int m_CurrentRadius;
	int m_Steps;
	int m_StartIndex{};
	int m_CurrentStep{ 1 };
	float m_WorldWidth;
	float m_WorldHeight;
	float m_CellSize{};
	int m_GridDivisions{};
	size_t m_NumberOfCells{};
	bool IsPointInRect(const Elite::Vector2& topLeft, float width, float height, const Elite::Vector2& point) const;
	void DiscoverEdges();
	bool AllCurrentCellsVisited();
	std::vector<int> GetCellsInRadius(int cellIndex, int radius);
};