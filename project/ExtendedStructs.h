#pragma once
#include <Exam_HelperStructs.h>

struct HouseSearch : public HouseInfo
{
	int CurrentCorner{0};
	bool FinishedSearch{false};
};