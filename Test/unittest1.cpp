#include "stdafx.h"
#include "CppUnitTest.h"
#include "../MikuMikuSpace/Network.h"
#include "../MikuMikuSpace/Network.cpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Test
{
    TEST_CLASS(UnitTest1)
    {
public:
        TEST_METHOD(TestMethod1)
        {
            // TODO: �e�X�g �R�[�h�������ɑ}�����܂�
            Network network;
            network.setid(1);
            Assert::AreEqual(1, network.getid());
        }
    };
}