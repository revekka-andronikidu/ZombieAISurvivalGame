#pragma once
#include "Exam_HelperStructs.h"
#include "IExamInterface.h"
#include <vector>

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

private:

	struct Cell
	{
		Elite::Vector2 Center{};
		bool visited{false};
		bool insideHouse{ false };

	};

	IExamInterface* m_pInterface = nullptr;
	std::vector<Cell> m_Cells;
	float m_WorldWidth;
	float m_WorldHeight;
	float m_CellSize{};
	int m_GridDivisions{};
	size_t m_NumberOfCells{};
	bool IsPointInRect(const Elite::Vector2& topLeft, float width, float height, const Elite::Vector2& point) const;

};