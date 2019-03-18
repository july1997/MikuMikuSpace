#pragma once
#include "DxLib.h"


#include <Ntsecapi.h>	// for PUNICODE_STRING
#include "sexyhook.h"

#include "../Common/Logger.h"
#include "../Common/Version.h"

class Core
{

    public:
        Core();
        ~Core();

        size_t setup();
};

