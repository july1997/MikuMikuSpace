#pragma once

#include <memory>
#include <utility>
#include <vector>

#include <boost/assert.hpp>
#include <boost/atomic.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/thread.hpp>

#include <windows.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

namespace hwm {

//! WAVEHDR�̃��b�p�N���X
//! WAVEHDR���Ǘ����A�o�C�g�P�ʂŎw�肵�������̃o�b�t�@�����蓖�Ă�B
struct WaveHeader {
	enum { UNUSED, USING, DONE };

	WaveHeader(size_t byte_length)
	{
		header_.lpData = new char[byte_length];
		header_.dwBufferLength = byte_length;
		header_.dwBytesRecorded = 0;
		header_.dwUser = UNUSED;
		header_.dwLoops = 0;
		header_.dwFlags = 0;
		header_.lpNext = nullptr;
		header_.reserved = 0;
	}

	~WaveHeader()
	{
		delete [] header_.lpData;
	}

	WaveHeader(WaveHeader &&rhs)
		:	header_(rhs.header_)
	{
		WAVEHDR empty = {};
		rhs.header_ = empty;
	}

	WaveHeader & operator=(WaveHeader &&rhs)
	{
		WaveHeader(std::move(rhs)).swap(rhs);
		return *this;
	}

	void swap(WaveHeader &rhs)
	{
		WAVEHDR tmp = header_;
		header_ = rhs.header_;
		rhs.header_ = tmp;
	}

	WAVEHDR * get()			{ return &header_; }

private:
	WAVEHDR	header_;
};

//! Wave�I�[�f�B�I�f�o�C�X���I�[�v�����A
//! �f�o�C�X�ւ̏����o�����s���N���X
struct WaveOutProcessor
{
	WaveOutProcessor()
		:	hwo_		(NULL)
		,	terminated_	(false)
		,	block_size_	(0)
		,	channel_	(0)
		,	multiplicity_(0)
	{
		InitializeCriticalSection(&cs_);
	}

	~WaveOutProcessor() {
		BOOST_ASSERT(!hwo_);
		DeleteCriticalSection(&cs_);
	}
	
	CRITICAL_SECTION cs_;
	HWAVEOUT hwo_;
	size_t block_size_;
	size_t multiplicity_;
	size_t channel_;
	std::vector<std::unique_ptr<WaveHeader>>	headers_;
	boost::thread					process_thread_;
	boost::atomic<bool>				terminated_;


	typedef
		std::function<void(short *data, size_t channel, size_t sample)>
	callback_function_t;

    //! �f�o�C�X�̃o�b�t�@���󂢂Ă���ꍇ�ɒǉ��Ńf�[�^��v������
    //! �R�[���o�b�N�֐�
    //! �f�o�C�X�̃f�[�^�`���͊ȒP�̂��߁A16bit�����t�������Œ�
	callback_function_t				callback_;
	boost::mutex					initial_lock_mutex_;

    //! �f�o�C�X���J��
    //! �J���f�o�C�X�̎w��́A����WAVE_MAPPER�Œ�B
    //! �ȒP�̂��ߗ�O���S���Ȃǂ͂��܂�l������Ă��Ȃ��_�ɒ��ӁB
	bool OpenDevice(size_t sampling_rate, size_t channel, size_t block_size, size_t multiplicity, callback_function_t callback)
	{
		BOOST_ASSERT(0 < block_size);
		BOOST_ASSERT(0 < multiplicity);
		BOOST_ASSERT(0 < channel && channel <= 2);
		BOOST_ASSERT(callback);
		BOOST_ASSERT(!process_thread_.joinable());

		block_size_ = block_size;
		channel_ = channel;
		callback_ = callback;
		multiplicity_ = multiplicity;

        //! �f�o�C�X���I�[�v����������܂�callback���Ă΂�Ȃ��悤�ɂ��邽�߂̃��b�N
		boost::unique_lock<boost::mutex> lock(initial_lock_mutex_);

		terminated_ = false;
		process_thread_ = boost::thread([this] { ProcessThread(); });

		WAVEFORMATEX wf;
		wf.wFormatTag = WAVE_FORMAT_PCM;
		wf.nChannels = 2;
		wf.wBitsPerSample = 16;
		wf.nBlockAlign = wf.nChannels * (wf.wBitsPerSample / 8);
		wf.nSamplesPerSec = sampling_rate;
		wf.nAvgBytesPerSec = wf.nBlockAlign * wf.nSamplesPerSec;
		wf.cbSize = sizeof(WAVEFORMATEX);

		headers_.resize(multiplicity_);
		for(auto &header: headers_) {
			header.reset(new WaveHeader(block_size * channel * sizeof(short)));
		}

        //! WAVEHDR�g�p�ςݒʒm���󂯎������Ƃ���
        //! CALLBACK_FUNCTION���w��B
        //! ����Ƀf�o�C�X���I�[�v���ł���ƁA
        //! waveOutWrite���Ăяo�������WaveOutProcessor::waveOutProc�ɒʒm������悤�ɂȂ�B
		MMRESULT const result = 
			waveOutOpen(
				&hwo_, 
				WAVE_MAPPER,
				&wf, 
				(DWORD_PTR)waveOutProc,
				(DWORD_PTR)this,
				CALLBACK_FUNCTION
				);

		if(result != MMSYSERR_NOERROR) {
			terminated_ = true;
			process_thread_.join();
			terminated_ = false;
            hwo_ = NULL;

			return false;
		}

		return true;
	}

    //! �f�o�C�X�����
	void CloseDevice() {
		terminated_.store(true);
		process_thread_.join();
		waveOutReset(hwo_);
		waveOutClose(hwo_);

        //! waveOutReset���Ăяo�������Ƃ�
        //! WAVEHDR�̎g�p�ςݒʒm�����邱�Ƃ�����B
        //! �܂�AwaveOutReset���Ăяo���������
        //! ������WAVEHDR������ł��Ȃ�(�f�o�C�X�ɂ���Ďg�p����������Ȃ�����)
        //! ���̂��߁A�m���ɂ��ׂĂ�WAVEHDR�̉�����m�F����B
		for( ; ; ) {
			int left = 0;
			for(auto &header : headers_ | boost::adaptors::indirected) {
				if(header.get()->dwUser == WaveHeader::DONE) {
					waveOutUnprepareHeader(hwo_, header.get(), sizeof(WAVEHDR));
					header.get()->dwUser = WaveHeader::UNUSED;
				} else if(header.get()->dwUser == WaveHeader::USING) {
					left++;
				}
			}

			if(!left) { break; }
			Sleep(10);
		}
		hwo_ = NULL;
	}

    //! �f�o�C�X�I�[�v�����Ɏw�肵���R�[���o�b�N�֐����Ăяo���āA
    //! �f�o�C�X�ɏo�͂���I�[�f�B�I�f�[�^����������B
	void PrepareData(WAVEHDR *header)
	{
		callback_(reinterpret_cast<short *>(header->lpData), channel_, block_size_);
	}

    //! �Đ��p�f�[�^�̏�����
    //! WAVEHDR�̓���ւ������X�ƍs�����[�J�[�X���b�h
	void ProcessThread()
	{
		{
			boost::unique_lock<boost::mutex> lock(initial_lock_mutex_);
		}
		for( ; ; ) {
			if(terminated_.load()) { break; }

            //! �g�p�ς�WAVEHDR�̊m�F
			for(auto &header: headers_ | boost::adaptors::indirected) {
				DWORD_PTR status = NULL;

				EnterCriticalSection(&cs_);
				status = header.get()->dwUser;
				LeaveCriticalSection(&cs_);

                //! �g�p�ς�WAVEHDR��Unprepare���āA���g�p�t���O�𗧂Ă�B
				if(status == WaveHeader::DONE) {
					waveOutUnprepareHeader(hwo_, header.get(), sizeof(WAVEHDR));
					header.get()->dwUser = WaveHeader::UNUSED;
				}
			}

            //! ���g�pWAVEHDR���m�F
			for(auto &header: headers_ | boost::adaptors::indirected) {
				DWORD_PTR status = NULL;

				EnterCriticalSection(&cs_);
				status = header.get()->dwUser;
				LeaveCriticalSection(&cs_);

				if(status == WaveHeader::UNUSED) {
                    //! �Đ��p�f�[�^������
					PrepareData(header.get());
					header.get()->dwUser = WaveHeader::USING;
                    //! waveOutPrepareHeader���Ăяo���O�ɁAdwFlags�͕K��0�ɂ���B
					header.get()->dwFlags = 0;

                    //! WAVEHDR��Prepare
					waveOutPrepareHeader(hwo_, header.get(), sizeof(WAVEHDR));
                    //! �f�o�C�X�֏����o��(�����悤�ɓo�^)
					waveOutWrite(hwo_, header.get(), sizeof(WAVEHDR));
				}
			}

			Sleep(1);
		}
	}

    //! �f�o�C�X����̒ʒm���󂯎��֐�
	static
	void CALLBACK waveOutProc(HWAVEOUT hwo, UINT msg, DWORD_PTR instance, DWORD_PTR p1, DWORD_PTR /*p2*/)
	{
		reinterpret_cast<WaveOutProcessor *>(instance)->WaveCallback(hwo, msg, reinterpret_cast<WAVEHDR*>(p1));
	}

    //! �}���`���f�B�A�n�̃R�[���o�b�N�֐���
    //! �g����֐��Ɍ��肪����ȂǗv����������
    //! ���ɁA���̊֐�����waveOutUnprepareHeader�Ȃǂ��Ăяo����WAVEHDR��
    //! ���Z�b�g�������s������͂ł��Ȃ��B�����Ȃ��΃V�X�e�����f�b�h���b�N����B
	void WaveCallback(HWAVEOUT /*hwo*/, UINT msg, WAVEHDR *header)
	{
		switch(msg) {
			case WOM_OPEN:
				break;

			case WOM_CLOSE: 
				break;

			case WOM_DONE:
				EnterCriticalSection(&cs_);
                //! �g�p�ς݃t���O�𗧂Ă�̂�
				header->dwUser = WaveHeader::DONE;
				LeaveCriticalSection(&cs_);
				break;
		}
	}
};

}	//::hwm
