#pragma once

#include "Model.h"
#include "Bullet_physics.h"

#include <vector>
#include <memory>

//�������Z���Ȃ����߂ɂ�Model.h�����g����������
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

        //���f���̃��b�V�����獄�̂��쐬
        size_t loadModel(const char *FileName);

		// Form�F���̍��̂̑傫��
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

        //�I�u�W�F�N�g�̈ʒu���L�[�{�[�h�ňړ��ł���悤�ɂ��܂�
        void testingPositioning(int object);

        //�S���f���̈ʒu���X�V
        void update();

        //���f����S�ĕ`��
        void draw();
};