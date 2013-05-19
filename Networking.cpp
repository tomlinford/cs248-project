#include "Networking.h"

#include <sstream>
#include <boost/thread.hpp>
#include <boost/asio.hpp>

using namespace std;

namespace Networking {

// these two need to be identified during init
static Level *level;
static string ip = ""; // boss, you probably will want to set this to be "localhost"

// constant port number
const static char *port = "1338";

// various commands
const static string TERRAIN = "terrain";

// function template
static void listenFunc();

extern void Init(Level *currentLevel, std::string ip_addr) {
	level = currentLevel;
	ip = ip_addr;

	// starts listening thread
	boost::thread listenThread(listenFunc);
}

static void listenFunc() {
	boost::asio::ip::tcp::iostream ns(ip.c_str(), port);
	string line;

	// will loop until stream closes
	while (getline(ns, line)) {
		if (line == TERRAIN) {
			// protocol: read next line to get the size of terrain.
			// a size of 512 means will be reading in 512 * 512 floats
			getline(ns, line);
			stringstream ss(line);
			size_t size;
			ss >> size;
			// now we read in the appropriate bytes into an array
			float *terrainMap = new float[size * size];
			ns.read((char *)terrainMap, size * size * sizeof(float));
			level->SetLevel(terrainMap, size);
		}
	}
}

};