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
static string player;

// constant port number
const static char *port = "1338";

// various commands
const static string TERRAIN = "terrain";
const static string PATH = "path";
const static string DONE = "done";
const static string READY = "ready";
const static string START = "start";

// function template
static void listenFunc();

extern void Init(Level *currentLevel, std::string ip_addr, char *p) {
	level = currentLevel;
	ip = ip_addr;
	player = p;

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
    cout << "Receiving terrain" << endl;
    
    // Read size and offset
    getline(ns, line);
    stringstream ss(line);
    size_t size;
    int x, y;
    ss >> size >> x >> y;
    
    // Read data
    float *terrainMap = new float[size * size];
    ns.read((char *)terrainMap, size * size * sizeof(float));
    level->SetLevel(terrainMap, size, x, y);
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
    level->SetControlPoints(points, num);
}

static void listenFunc() {
	boost::asio::ip::tcp::iostream ns(ip.c_str(), port);
	if (ns.fail()) {
		cerr << "failed to open tcp connection" << endl;
		return;
	}

	// write the player
	ns << player << endl;
	
	// Will loop until stream closes
	string line;
	while (getline(ns, line)) {
		if (line == TERRAIN) {
			parseTerrain(ns, line);
		} else if (line == PATH) {
            parsePath(ns, line);
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
}

};