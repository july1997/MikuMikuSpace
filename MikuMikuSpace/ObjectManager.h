#pragma once

#include "Model.h"
#include "Bullet_physics.h"

#include <vector>
#include <memory>

//物理演算しないためにはModel.hをお使いください
class Object
{
    protected:

		std::shared_ptr<Bullet_physics> bullet;
        int world;
        std::vector<VECTOR> forms;
        std::vector<int> bodyhandel;

        size_t objects = 0;

        std::vector<std::shared_ptr<Model>> models;

        VECTOR testingpos = VGet(0, 0, 0), testingrot = VGet(0, 0, 0);

    public:
        Object(std::shared_ptr<Bullet_physics>_bullet, int _world);

        //モデルのメッシュから剛体を作成
        size_t loadModel(const char *FileName);

		// Form：箱の剛体の大きさ
        size_t loadModel(const char *FileName, VECTOR Form);

        size_t deleteModel(int object);

        size_t copyModel(int object, float scale = 1.0f);

		int setModelScale(int object, float scale);

        int setPos(int object, VECTOR pos);

        int setRot(int object, VECTOR rot);

        int setScale(int object, float scale, float mass = 1);

        int getModelHandle(int object);

        VECTOR getPos(int object);

        VECTOR getRot(int object);

        //オブジェクトの位置をキーボードで移動できるようにします
        void testingPositioning(int object);

        //全モデルの位置を更新
        void update();

        //モデルを全て描画
        void draw();
};