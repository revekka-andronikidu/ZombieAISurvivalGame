#pragma once
#include <Exam_HelperStructs.h>
 #include <chrono>

struct HouseSearch : public HouseInfo
{
	int CurrentCorner{0};
	bool FinishedSearch{false};
	std::chrono::time_point<std::chrono::steady_clock> lastVisited;
};

struct Cell
{
	Elite::Vector2 Center{};
	bool visited{ false };

};