#pragma once

#include "DxLib.h"

#include <string>

class Youtube
{
    public:
        Youtube();
        ~Youtube();

        //URL���瓮����_�E�����[�h���čĐ�����@�߂�l:0�@�I��
        size_t downloadmovie(std::string VideoURL);

        //�Đ����J�n
        void playMovie();

        //��ʂ̍X�V
        void update();

        //�Đ����ꎞ��~�A�ĊJ����
        void pause();

        //�Đ��ʒu��ύX����
        void seek(int time);

        //�Đ��ʒu�𓾂�
        int getSeek();

        //yotude-dl�̍X�V
        bool upgrade();

		//ModelHandel��ScreenTexture(�e�N�X�`���ԍ�)�ɓ�����Đ����邽�߂̏���
		void setScreen(const int ModelHandel, const int ScreenTexture);

		void stop();

		void setVolume(int Volume);

    private:

        int screen_handel,movie_handle;

        size_t downloadmovieflag = 3;
        size_t timer = 0;
        bool deleteCacheflag = 0, upgradeflag = 0;
        struct stat buf;

        void deleteProcess();
        bool deleteCache();

        std::string exe;
        std::string path;

        void yotube_dl(const char *exe, const char *word);

        DWORD dwD;
        HANDLE hHandle;

		int mode_handle = -1, screen_texture;
};

