#include "fast_aerial_practice.h"
#include "HelperFunctions.h"
#include "bakkesmod\wrappers\includes.h"
#include "utils/parser.h"
#include <vector>

using namespace std;
using namespace std::placeholders;

BAKKESMOD_PLUGIN(FastAerialPractice, "Times your fast aerial and keeps track of your best time", "1.0", PLUGINTYPE_FREEPLAY)

static const string savefile = "bakkesmod/data/fastaerialplugin.data";

static std::vector<AerialRecord> aerialRecords{ {-200, -1}, {1, -1},  {200, -1}, {800, -1}, {1600, -1}, {3200, -1} };

void FastAerialPractice::onLoad() {
	srand(time(nullptr));
	load();

	cvarManager->registerNotifier("aerialtimer_reset", [this](std::vector<string> params) {
		for (int i = 0; i < aerialRecords.size(); i++) {
			aerialRecords[i].personalRecord = -1;
		}
	}, "Reset personal bests", PERMISSION_ALL);

	pDefaultTime = { "" };
	pBest = { "" };
	pBallHitted = { "" };
	popups.push_back(&pDefaultTime);
	popups.push_back(&pBest);
	popups.push_back(&pBallHitted);

	gameWrapper->HookEventPost("Function TAGame.Car_TA.EventHitBall", bind(&FastAerialPractice::onHitBall, this, _1));
	gameWrapper->HookEventPost("Function Engine.Controller.Restart", bind(&FastAerialPractice::onReset, this, _1));
	gameWrapper->HookEventPost("Function GameEvent_Soccar_TA.Active.StartRound", bind(&FastAerialPractice::onStartedDriving, this, _1));
	gameWrapper->RegisterDrawable(bind(&FastAerialPractice::Render, this, _1));
}
void FastAerialPractice::onUnload() {
	save();
}

void FastAerialPractice::onHitBall(string eventName)
{
	auto server = gameWrapper->GetGameEventAsServer();
	if (server.IsNull())
		return;
	auto ball = server.GetBall();
	if (ball.IsNull())
		return;

	//Reset ball gravity
	ball.SetBallGravityScale(1);

	timeHit = gameWrapper->GetGameEventAsServer().GetSecondsElapsed() - timeStart;
	hitted = true;
	//If too fast
	if (timeHit < 1.5)
		return;
	//If the record is the default one.
	if (currentAerial->personalRecord < 0)
	{
		currentAerial->personalRecord = timeHit;
		pBallHitted.color = { 255,255,255 };
		save();
	}
	else {
		if (timeHit < currentAerial->personalRecord)
		{
			pBallHitted.color = { 255,200,0 };
			currentAerial->personalRecord = timeHit;
			save();
		}
		else {
			pBallHitted.color = { 255,0,0 };
		}
	}

	pBallHitted.text = string("Ball hitted after ") + to_string_with_precision(timeHit, 2) + string(" seconds.");
	lastMsg = chrono::system_clock::now();
}

void FastAerialPractice::onStartedDriving(string eventName)
{
	if (!gameWrapper->IsInGame())
		return;

	timeStart = gameWrapper->GetGameEventAsServer().GetSecondsElapsed();
}

void FastAerialPractice::onReset(string eventName)
{
	auto server = gameWrapper->GetGameEventAsServer();
	if (server.IsNull())
		return;
	auto ball = server.GetBall();
	if (ball.IsNull())
		return;
	auto car = gameWrapper->GetLocalCar();
	if (car.IsNull())
		return;


	int i  = rand() % aerialRecords.size();
	currentAerial = &aerialRecords[i];
	auto newBallLocation = car.GetLocation() + GetCarForwardVector(car) * currentAerial->distance;
	newBallLocation.Z = 1500;

	ball.SetBallGravityScale(0);
	ball.SetLocation(newBallLocation);

	hitted = false;
	pBallHitted.text = "";

	if (currentAerial->personalRecord > 0) {
		pBest.text = string("Personal Best: ") + to_string_with_precision(currentAerial->personalRecord, 2) + string(" seconds");
	}
	else {
		pBest.text = "";
	}
	lastMsg = chrono::system_clock::now();
}

Vector FastAerialPractice::GetCarForwardVector(CarWrapper& car)
{
	auto rbstate = car.GetRBState();
	Quat quat = rbstate.Quaternion;
	return sp::quatToFwd(quat);
	
}

void FastAerialPractice::Render(CanvasWrapper canvas)
{
	if (!gameWrapper->IsInGame() || popups.empty() || currentAerial == nullptr || chrono::duration_cast<std::chrono::seconds> (chrono::system_clock::now() - lastMsg).count() > 10)
		return;

	auto screenSize = canvas.GetSize();
	for (int i = 0; i < popups.size(); i++)
	{
		auto pop = popups.at(i);
		if (pop->startLocation.X < 0)
		{
			pop->startLocation = { (int)(screenSize.X * 0.35), (int)(screenSize.Y * 0.1 + i * 0.035 * screenSize.Y) };
		}

		Vector2 drawLoc = { pop->startLocation.X, pop->startLocation.Y };
		canvas.SetPosition(drawLoc);
		canvas.SetColor(pop->color.R, pop->color.G, pop->color.B, 255);
		canvas.DrawString(pop->text, 3, 3);
	}
}

void FastAerialPractice::save()
{
	ofstream myfile;
	myfile.open(savefile);
	if (myfile.is_open())
	{
		for (auto record : aerialRecords)
		{
			myfile << record.distance << "," << record.personalRecord << "\n";
		}
	}
	else
	{
		cvarManager->log("Can't write savefile.");
	}
	myfile.close();
}

void FastAerialPractice::load()
{
	ifstream myfile;
	myfile.open(savefile);
	if (myfile.good())
	{
		aerialRecords.clear();
		string line, word;
		vector<string> row;
		while (getline(myfile, line)) {
			row.clear();

			stringstream s(line);

			while (getline(s, word, ',')) {
				row.push_back(word);
			}
			if (row.size() != 2)
			{
				cvarManager->log("Invalid savefile");
			}
			aerialRecords.push_back({ stoi(row[0]), stof(row[1]) });
		}
		myfile.close();
	}
	else {
		cvarManager->log("Unable to open savefile");
	}
}
