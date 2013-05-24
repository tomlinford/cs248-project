#include "Networking.h"

#include <sstream>
#include <thread>
#include <boost/asio.hpp>

using namespace std;
using namespace glm;

namespace Networking {

// these two need to be identified during init
static Level *level;
static string ip = "";

// constant port number
const static char *port = "1338";

// various commands
const static string TERRAIN = "terrain";
const static string PATH = "path";
const static string READY = "ready";

// function template
static void listenFunc();

extern void Init(Level *currentLevel, std::string ip_addr) {
	level = currentLevel;
	ip = ip_addr;

	// starts listening thread
	thread listenThread(listenFunc);
	listenThread.detach();
}

static void listenFunc() {
	boost::asio::ip::tcp::iostream ns(ip.c_str(), port);
	string line;
	
	/*while (getline(ns, line)) {
		line = "";
	}*/

	// will loop until stream closes
	while (getline(ns, line)) {
		if (line == TERRAIN) {
			cout << "reading in terrain" << endl;
			// protocol: read next line to get the size of terrain.
			// a size of 512 means will be reading in 512 * 512 floats
			getline(ns, line);
			stringstream ss(line);
			size_t size;
			int x, y;
			ss >> size >> x >> y;
			// now we read in the appropriate bytes into an array
			float *terrainMap = new float[size * size];
			ns.read((char *)terrainMap, size * size * sizeof(float));
			level->SetLevel(terrainMap, size, x, y);
		} else if (line == PATH) {
			getline(ns, line);
			stringstream ss(line);
			size_t num;
			ss >> num;
			vec2 *points = new vec2[num];
			ns.read((char *)points, num * sizeof(float) * 2);
			level->SetControlPoints(points, num);
		} else if (line == READY) {
			level->SetReady();
		}
	}
	getline(ns, line);
	line += "what";
}

};