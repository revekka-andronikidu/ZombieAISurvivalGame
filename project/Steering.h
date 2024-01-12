#ifndef STEERINGMANAGER
#define STEERINGMANAGER
#include "Exam_HelperStructs.h"
#include "IExamInterface.h"

class Steering final
{
public:
	Steering(IExamInterface* pInterface, SteeringPlugin_Output* pSteering);
	~Steering() = default;

	Steering(const Steering& steering) = delete;
	Steering(Steering&& steering) = delete;
	Steering& operator=(const Steering& steering) = delete;
	Steering& operator=(Steering&& steering) = delete;


	void AutoOrient(bool state);
	void Seek(const Elite::Vector2& target);
	void Flee(const Elite::Vector2& target);
	void Wander();
	void SpinAround();
	void Face(const Elite::Vector2& target);
	void Run(bool state);
	// StopMovement();
private:
	IExamInterface* m_pInterface = nullptr;
	SteeringPlugin_Output* m_pSteering = nullptr;
};
#endif
