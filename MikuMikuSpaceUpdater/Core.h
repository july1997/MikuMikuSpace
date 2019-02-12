#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include "../Common/Zip/unzipper.h"
#include "../Common/UI/UI.h"
#include "../Common/Fps.h"
#include "../Common/WebDownloader.h"
#include "../Common/Utility/json11.hpp"
#include "../Common/Server.h"
#include "../Common/Logger.h"

using namespace std;
using namespace ziputils;

class Core
{
public:
	Core();
	~Core();

	size_t setup();

	void load();

	bool donwloadUpdater();

	bool update();

	bool lunchExe();

private:

	Logger log;
	FpsManager fps;
	UI ui;
	int font, font2, font3;
	int inbox, box;
	int e = 0;
	bool majorupdate = 0;
	std::string ver;

};

