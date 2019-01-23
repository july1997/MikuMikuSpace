#pragma once
#include <array>
#include <memory>

#include <windows.h>
#include <tchar.h>
#include <mmsystem.h>

#include <boost/atomic.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/optional.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>

#pragma warning(push)
#pragma warning(disable: 4996)
#include "Vst/vstsdk2.4/pluginterfaces/vst2.x/aeffectx.h"
#pragma warning(pop)

#include "Vst/HostApplication.hpp"
#include "Vst/VstPlugin.hpp"
#include "Vst/WaveOutProcessor.hpp"

#include "DxLib.h"

//64bit��VST�̂ݎg�p�\�Ȗ͗l(64bit�r���h��)
class VstHost
{
	//! �I�[�f�B�I�n�萔
	static size_t const SAMPLING_RATE = 44100;
	static size_t const BLOCK_SIZE = 1024;
	static size_t const BUFFER_MULTIPLICITY = 4;

public:
	VstHost();
	~VstHost();

	void loadLibrary(std::string libraryName);
};

