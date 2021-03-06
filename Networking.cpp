#include "Networking.h"

#include <sstream>
#include <thread>
#include <mutex>
#include <boost/asio.hpp>
#include <chrono>

#include "Sound.h"

using namespace std;
using namespace glm;

namespace Networking {

	// these two need to be identified during init
	static Scene *scene;
	static Level *level;
	static string ip = "";
	static string player;
	static int levelNum;

	// game over condition
	static bool gameOver = false;

	// constant port number
	const static char *port = "3789";

	// pointer to active network stream and associated lock
	typedef boost::asio::ip::tcp::iostream nstream;
	static boost::asio::ip::tcp::iostream *nsp;
	static mutex nspMutex;

	// various commands
	const static string TERRAIN = "terrain";
	const static string PATH = "path";
	const static string DONE = "done";
	const static string READY = "ready";
	const static string START = "start";
	const static string END = "end";
	const static string KEY = "key";
	const static string BULLET = "bullet";
	const static string ENEMY_SHIP = "enemy_ship";
	const static string TURRET = "turret";
	const static string LIGHTNING = "lightning";
	const static string HEALTH = "health";
	const static string SCORE = "score";
	const static string MISSILE = "missile";

	// function template
	static void listenFunc();

	extern void Init(Scene *currScene, Level *currentLevel, std::string ip_addr, const char *p, int currLevelNum) {
		scene = currScene;
		level = currentLevel;
		ip = ip_addr;
		player = p;
		levelNum = currLevelNum;

		gameOver = false;

		// starts listening thread
		thread listenThread(listenFunc);
		listenThread.detach();
	}

	/** Parses terrain data from the server.

	The format is as follows:
	terrain
	[size] [x offset of chunk] [y offset of chunk]
	[float data...]
	*/
	static void parseTerrain(boost::asio::ip::tcp::iostream& ns, string& line) {
		//cout << "Receiving terrain" << endl;

		// Read size and offset
		getline(ns, line);
		stringstream ss(line);
		size_t size;
		int x, y;
		ss >> size >> x >> y;

		// Read data
		float *terrainMap = new float[size * size];
		ns.read((char *)terrainMap, size * size * sizeof(float));
		level->LoadMap(terrainMap, size, x, y);
	}

	/** Parses path data from the server.

	The format is as follows:
	path
	[size]
	[float data...]
	*/
	static void parsePath(boost::asio::ip::tcp::iostream& ns, string& line) {
		//cout << "Receiving path" << endl;

		// Read size
		getline(ns, line);
		stringstream ss(line);
		size_t num;
		ss >> num;

		// Read data
		vec3 *points = new vec3[num];
		ns.read((char *)points, num * sizeof(float) * 3);
		level->LoadControlPoints(points, num);
	}

	/** Parses enemy ship data from server.

	Format:
	enemy_ship
	[timeOffset] [offset (vec2)]
	*/
	static void parseEnemyShip(boost::asio::ip::tcp::iostream& ns, string& line) {
		getline(ns, line);
		stringstream ss(line);
		float timeOffset;
		vec2 offset;
		ss >> timeOffset >> offset.x >> offset.y;
		level->LoadEnemyShip(timeOffset, offset);
	}

	/** Parses power-up/power-down sphere data from server.

	Format:
	turret
	[location (vec3)]
	*/
	static void parseTurret(boost::asio::ip::tcp::iostream& ns, string& line) {
		getline(ns, line);
		stringstream ss(line);
		vec3 loc;
		ss >> loc.x >> loc.y >> loc.z;
		Turret *turret = new Turret("Models/turret.obj");
		turret->SetColor(vec3(0.9, 0.7, 0.5));
		swap(loc.y, loc.z);
		loc *= 20.f;
		float height = level->GetHeightAt(loc.x / 320.f, loc.z / 320.f);
		height = (height - .5f) * 60.f;
		loc.y = height;
        turret->SetPosition(loc);
		level->objects.push_back(turret);
	}

	static void listenFunc() {
		boost::asio::ip::tcp::iostream ns(ip.c_str(), port);
		nsp = &ns;
		if (ns.fail()) {
			cerr << "failed to open tcp connection" << endl;
			nsp = NULL;
			return;
		}

		// write the player
		ns << player << endl;
		if (levelNum == 0)
			ns << "easy" << endl;
		else if (levelNum == 1)
			ns << "medium" << endl;
		else
			ns << "hard" << endl;

		// Will loop until stream closes
		string line;
		while (getline(ns, line)) {
			if (line == TERRAIN) {
				parseTerrain(ns, line);
			} else if (line == PATH) {
				parsePath(ns, line);
			} else if (line == ENEMY_SHIP) {
				parseEnemyShip(ns, line);
			} else if (line == TURRET) {
				parseTurret(ns, line);
			} else if (line == DONE) {
				cout << "Ready!" << endl;
				break;
			}
		}
		ns << READY << endl;
		getline(ns, line);
		//cout << "received command: " << line << endl;
		// next line should be start
		level->SetReady();

		// main loop for reading stuff
		for (;;) {

			// hacky solution:
			// every 5 milliseconds, poll the underlying buffer of the network
			// iostream to see if it has received more data
			while(ns.rdbuf()->available() == 0 && !gameOver)
				std::this_thread::sleep_for(std::chrono::milliseconds(5));

			if (gameOver) break;

			getline(ns, line);
			stringstream ss(line); string header;
			ss >> header;
			if (header == KEY) {
				int key, action; vec2 shipOffset;
				ss >> key >> action >> shipOffset.x >> shipOffset.y;
				switch(key) {
				case GLFW_KEY_LEFT:
					scene->keyLeft = (action == GLFW_PRESS);
					break;
				case GLFW_KEY_RIGHT:
					scene->keyRight = (action == GLFW_PRESS);
					break;
				case GLFW_KEY_UP:
					scene->keyUp = (action == GLFW_PRESS);
					break;
				case GLFW_KEY_DOWN:
					scene->keyDown = (action == GLFW_PRESS);
					break;
				}
				scene->SetShipOffset(shipOffset);
			} else if (header == BULLET) {
				vec3 position, velocity;
				ss >> position.x >> position.y >> position.z;
				ss >> velocity.x >> velocity.y >> velocity.z;
				Sound::PlayLaser();
				level->ship->AddBullet(position, velocity);
			} else if (header == MISSILE) {
				vec2 offset; float timeOffset;
				ss >> offset.x >> offset.y >> timeOffset;
				Missile *missile = new Missile("Models/missile.obj");
				missile->SetColor(vec3(1.0, 1.0, 1.0));
				missile->SetTimeOffset(timeOffset);
				missile->SetOffset(offset);
				lock_guard<mutex> lock(level->mutex);
				level->objects.push_back(missile);
			} else if (header == LIGHTNING) {
				scene->AddLightning(true);
			} else if (header == HEALTH) {
				float health; ss >> health;
				scene->health = health;
			} else if (header == SCORE) {
				int score; ss >> score;
				scene->SetScore(score);
			} else if (header == END) {
				break;
			}
		}
		ns << HEALTH << " " << -1 << endl;
		ns << END << endl;
	}

	extern bool PingServer(string ip_addr) {
		nstream ns(ip_addr.c_str(), port);
		ns << "ping" << endl;
		string line;
		getline(ns, line);
		return line == "pong";
	}

	extern void KeyAction(int key, int action, vec2 shipOffset) {
		lock_guard<mutex> lock(nspMutex);
		(*nsp) << KEY << " " << key << " " << action << " ";
		(*nsp) << shipOffset.x << " " << shipOffset.y << endl;
	}

	extern void AddBullet(glm::vec3 position, glm::vec3 velocity) {
		lock_guard<mutex> lock(nspMutex);
		(*nsp) << BULLET << " ";
		(*nsp) << position.x << " " << position.y << " " << position.z << " ";
		(*nsp) << velocity.x << " " << velocity.y << " " << velocity.z << endl;
	}

	extern void AddLightning() {
		lock_guard<mutex> lock(nspMutex);
		(*nsp) << LIGHTNING << endl;
	}

	extern void SetHealth(float health) {
		lock_guard<mutex> lock(nspMutex);
		(*nsp) << HEALTH << " " << health << endl;
	}

	extern void SetScore(int score) {
		lock_guard<mutex> lock(nspMutex);
		(*nsp) << SCORE << " " << score << endl;
	}

	extern void AddMissile(vec2 offset, float timeOffset) {
		lock_guard<mutex> lock(nspMutex);
		(*nsp) << MISSILE << " " << offset.x << " " << offset.y
			<< " " << timeOffset << endl;
	}

	extern void GameOver() {
		gameOver = true;
	}

};