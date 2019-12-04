#pragma once
#pragma comment(lib, "BakkesMod.lib")
#include "bakkesmod/plugin/bakkesmodplugin.h"

struct Color
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
};
struct Popup
{
	std::string text = "";
	Color color = { 255, 255, 255 };
	Vector2 startLocation = { -1, -1 };
};

struct AerialRecord
{
	int distance;
	float personalRecord = -1;
};

class FastAerialPractice : public BakkesMod::Plugin::BakkesModPlugin
{
private:
	chrono::system_clock::time_point lastMsg;
	float timeStart;
	float timeHit;
	bool hitted = false;
	std::vector<Popup*> popups;
	Popup pDefaultTime;
	Popup pBallHitted;
	Popup pBest;
	AerialRecord* currentAerial;
public:
	virtual void onLoad();
	virtual void onUnload();

	virtual void onHitBall(std::string eventName);
	virtual void onStartedDriving(std::string eventName);
	virtual void onReset(std::string eventName);
	Vector GetCarForwardVector(CarWrapper& car);
	virtual void Render(CanvasWrapper canvas);

	virtual void save();
	virtual void load();
};
