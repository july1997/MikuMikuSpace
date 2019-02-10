#pragma once
#include "DxLib.h"

#include "Model.h"
#include "Bullet_physics.h"

#include <string>

class ModelManager : public Model
{
    public:
        ModelManager();
        ~ModelManager();

        virtual void loadModel(const char *FileName);
};

