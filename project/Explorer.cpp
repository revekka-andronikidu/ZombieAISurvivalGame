#include "stdafx.h"
#include "Explorer.h"

Explorer::Explorer(IExamInterface* pInterface)
	:m_pInterface{ pInterface }
{
	m_WorldWidth = pInterface->World_GetInfo().Dimensions.x;
	m_WorldHeight = pInterface->World_GetInfo().Dimensions.y;
	
	Elite::Vector2 worldCenter = pInterface->World_GetInfo().Center;

	//number of divisions
	m_GridDivisions = 16;

	//tile size
	m_CellSize = m_WorldWidth/m_GridDivisions;

	m_NumberOfCells  = m_GridDivisions * m_GridDivisions ;


	m_Cells.reserve(m_NumberOfCells);

	const Elite::Vector2 topLeftCellCenter{ Elite::Vector2{worldCenter.x - m_WorldWidth / 2.f, worldCenter.y - m_WorldHeight / 2.f} - Elite::Vector2{m_CellSize / 2.f, m_CellSize / 2.f} };


	Cell cell;
	for (size_t i{}; i < m_NumberOfCells; ++i)
	{
		cell.Center.x = topLeftCellCenter.x + (i % m_GridDivisions) * m_CellSize;
		cell.Center.y = topLeftCellCenter.y + (i / m_GridDivisions) * m_CellSize;
		cell.visited = false;
		m_Cells.emplace_back(cell);
	}

	
	m_Steps = 4;
	m_CurrentRadius = m_GridDivisions / 2 / m_Steps;
	auto agentInfo = m_pInterface->Agent_GetInfo();
	//m_StartIndex = PositionToIdx(agentInfo.Position + Elite::Vector2{ 0,m_CellSize });
	m_StartIndex = PositionToIdx(pInterface->World_GetInfo().Center);
	m_CurrentCellsIdx = GetCellsInRadius(m_StartIndex, m_CurrentRadius);

	DiscoverEdges();
}

void Explorer::DrawGrid() const
{
	const Elite::Vector3 defaultColor{ 0.5f, 0.5f, 0.5f };
	const Elite::Vector3 visitedColor{ 0.f, 0.f, 0.8f };
	const Elite::Vector3 hasHosue{ 0.8f, 0.f, 0.0f };
	Elite::Vector3 color{};
	//for each cell
	/*for (const auto& cell : m_CurrentCellsIdx)
	{
		const std::vector<Elite::Vector2> rect
		{
			{ m_Cells[cell].Center + Elite::Vector2{-m_CellSize / 2,m_CellSize / 2}},
			{ m_Cells[cell].Center + Elite::Vector2{m_CellSize / 2,m_CellSize / 2} },
			{ m_Cells[cell].Center + Elite::Vector2{m_CellSize / 2,-m_CellSize / 2} },
			{ m_Cells[cell].Center + Elite::Vector2{-m_CellSize / 2,-m_CellSize / 2} },
		};

		
		if (m_Cells[cell].visited)
		{
			color = visitedColor;

			m_pInterface->Draw_Polygon(rect.data(), static_cast<int>(rect.size()), color);
		}
		else
			color = defaultColor;

		m_pInterface->Draw_Polygon(rect.data(), static_cast<int>(rect.size()), color);
	}*/

	for (const auto& cell : m_Cells)
	{
		const std::vector<Elite::Vector2> rect
		{
			{cell.Center + Elite::Vector2{-m_CellSize / 2,m_CellSize / 2}},
			{ cell.Center + Elite::Vector2{m_CellSize / 2,m_CellSize / 2} },
			{ cell.Center + Elite::Vector2{m_CellSize / 2,-m_CellSize / 2} },
			{cell.Center + Elite::Vector2{-m_CellSize / 2,-m_CellSize / 2} },
		};


		if (cell.visited)
		{
			color = visitedColor;

			m_pInterface->Draw_Polygon(rect.data(), static_cast<int>(rect.size()), color);
		}
		else
			color = defaultColor;

		//m_pInterface->Draw_Polygon(rect.data(), static_cast<int>(rect.size()), color);
	}

}

void Explorer::Update()
{
	auto agentInfo = m_pInterface->Agent_GetInfo();

	int idx = PositionToIdx(agentInfo.Position + Elite::Vector2{ 0,m_CellSize });
	

	if (idx != -1 && m_Cells.at(idx).visited == false)
	{
		float fovRange{ agentInfo.FOV_Range };
		if ((m_Cells.at(idx).Center - agentInfo.Position).MagnitudeSquared() <= (fovRange * fovRange))
		{
			m_Cells.at(idx).visited = true;
		}
	}

	if (AllCurrentCellsVisited())
	{
		m_CurrentStep++;

		m_CurrentRadius = (m_GridDivisions / 2 / m_Steps) * m_CurrentStep;

		m_CurrentCellsIdx = GetCellsInRadius(m_StartIndex, m_CurrentRadius);
	}
}

int Explorer::PositionToIdx(const Elite::Vector2& position) const
{
	for (size_t i{ 0 }; i < m_NumberOfCells; ++i)
	{
		Elite::Vector2 topLeft{ m_Cells.at(i).Center + Elite::Vector2{-m_CellSize / 2.f, m_CellSize / 2.f} };

		if (IsPointInRect(topLeft, m_CellSize, m_CellSize, position))
		{
			
			return i;
		}
	}
	return -1;
}


bool Explorer::IsPointInRect(const Elite::Vector2& topLeft, float width, float height, const Elite::Vector2& point) const
{
	return point.x >= topLeft.x && point.x < topLeft.x + width && point.y >= topLeft.y && point.y < topLeft.y + height;
}

Elite::Vector2 Explorer::NextClosestCell()
{
	float closestDistance{ FLT_MAX };
	Elite::Vector2 agentPosition = m_pInterface->Agent_GetInfo().Position;
	Elite::Vector2 target{ 0.f, 0.f };

	
	for (size_t idx{0}; idx < m_CurrentCellsIdx.size(); idx++)
	{
		if (m_Cells[m_CurrentCellsIdx[idx]].visited)
			continue;

		auto distance = Elite::Distance(agentPosition, m_Cells[m_CurrentCellsIdx[idx]].Center);
		if (distance < closestDistance)
		{
			target = m_Cells[m_CurrentCellsIdx[idx]].Center;
			closestDistance = distance;
		}
	}

	return target;
	
}

bool Explorer::AllCellsVisited()
{
	for (size_t i{}; i < m_NumberOfCells; ++i)
	{	
		if(!m_Cells[i].visited)
		return false;
	}
	return true;
}

bool Explorer::AllCurrentCellsVisited()
{
	for (size_t i{}; i < m_CurrentCellsIdx.size(); ++i)
	{
		if (!m_Cells[m_CurrentCellsIdx[i]].visited)
		return false;
	}
	return true;
}

void Explorer::DiscoverEdges()
{
	
	// Set top and bottom edges to doscovered
	for (int col = 0; col < m_GridDivisions; ++col) {
		m_Cells[col].visited = true;                  // Top edge
		m_Cells[(m_GridDivisions - 1) * m_GridDivisions + col].visited = true; // Bottom edge
	}

	// Set left and right edges to discovered
	for (int row = 0; row < m_GridDivisions; ++row) {
		m_Cells[row * m_GridDivisions].visited = true;            // Left edge
		m_Cells[(row + 1) * m_GridDivisions - 1].visited = true;   // Right edge
	}
}
std::vector<int> Explorer::GetCellsInRadius(int cellIndex, int radius)
{
	std::vector<int> result;

	// Extract row and column from the 1D index
	int row = cellIndex / m_GridDivisions;
	int col = cellIndex % m_GridDivisions;

	// Loop within a 3x3 square centered at the given cell
	for (int i = -radius; i <= radius; ++i) {
		for (int j = -radius; j <= radius; ++j) {
			int newRow = row + i;
			int newCol = col + j;

			// Check if the new coordinates are within the grid boundaries
			if (newRow >= 0 && newRow < m_GridDivisions && newCol >= 0 && newCol < m_GridDivisions) {
				// Convert 2D coordinates to 1D index
				int newIndex = newRow * m_GridDivisions + newCol;
				result.push_back(newIndex);
			}
		}
	}

	return result;
}
	

