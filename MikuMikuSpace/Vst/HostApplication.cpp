#pragma warning(push)
#pragma warning(disable: 4996)

#include <algorithm>
#include <boost/range/size.hpp>
#include <boost/static_assert.hpp>
#include <boost/chrono.hpp>

#include <tchar.h>

#include "./HostApplication.hpp"
#include "./VstPlugin.hpp"

namespace hwm {

HostApplication::HostApplication(size_t sampling_rate, size_t block_size)
	:	sampling_rate_(sampling_rate)
	,	block_size_(block_size)
{}

VstIntPtr VSTCALLBACK VstHostCallback(AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void *ptr, float opt)
{
	//! VstPlugin�̏���������������܂ł͂�����
	if( !effect || !effect->user) {
		switch(opcode) {
			case audioMasterVersion:
				return kVstVersion;
			default:
				return 0;
		}
	} else {
		//! VstPlugin�̏���������������ƁAeffect->user�ɁAeffect��ێ����Ă���VstPlugin�̃A�h���X�������Ă���
		VstPlugin *vst = static_cast<VstPlugin *>(effect->user);
		return vst->GetHost().Callback(vst, opcode, index, value, ptr, opt);
	}
}

VstIntPtr HostApplication::Callback(VstPlugin* vst, VstInt32 opcode, VstInt32 index, VstIntPtr value, void *ptr, float opt)
{
	int result = false;
	opt; //���g�p�̕ϐ��̌x����}��

	switch( opcode )
	{
	case audioMasterAutomate:
		//! Plugin����̑���̒ʒm
		//! �I�[�g���[�V��������̋L�^�ɑΉ�����VST�z�X�g��
		//! �����œn���Ă����f�[�^���I�[�g���[�V�����G���x���[�v�ɋL�^����
		break;

	case audioMasterVersion:
		result = kVstVersion;
		break;

	case audioMasterCurrentId:
		//! kPlugCategShell�^�C�v�̃v���O�C���ɑ΂��āA�S�ẴT�u�v���O�C������W�J����Ƃ��ɂ̂݌Ă΂��
		return vst->GetEffect()->uniqueID;

	case audioMasterIdle:
		//! VST�v���O�C���̃A�C�h�����Ԃ�VST�z�X�g�n��
		//! VST�z�X�g�́A�S�Ă̊J���Ă���v���O�C���G�f�B�^�ɑ΂��āAeffEditIdle���Ăяo�����肷��
		{
			if(vst->IsEditorOpened()) {
				vst->dispatcher(effEditIdle, 0, 0, 0, 0);
			}
		}
		break;

//Deprecated
	//case audioMasterPinConnected:
		//break;
//Deprecated
	//case audioMasterWantMidi:
		//break;

	case audioMasterGetTime:
		//! VST�z�X�g�̌��݂̎�������Ԃ�
		timeinfo_.samplePos = 0;
		timeinfo_.sampleRate = sampling_rate_;
		timeinfo_.nanoSeconds = GetTickCount() * 1000.0 * 1000.0;
		timeinfo_.ppqPos = 0;
		timeinfo_.tempo = 120.0;
		timeinfo_.barStartPos = 0;
		timeinfo_.cycleStartPos = 0;
		timeinfo_.cycleEndPos = 0;
		timeinfo_.timeSigNumerator = 4;
		timeinfo_.timeSigDenominator = 4;
		timeinfo_.smpteOffset = 0;
		timeinfo_.smpteFrameRate = kVstSmpte24fps ;
		timeinfo_.samplesToNextClock = 0;
		timeinfo_.flags = (kVstNanosValid | kVstPpqPosValid | kVstTempoValid | kVstTimeSigValid);

		return reinterpret_cast<VstIntPtr>(&timeinfo_);

	case audioMasterProcessEvents:
		//! �v���O�C�����瑗���Ă����C�x���g����������
		//! processReplacing�̌Ăяo���̒�����Ă΂��B
		break;

//Deprecated
	//case audioMasterSetTime:
		//break;
//Deprecated
	//case audioMasterTempoAt:
		//break;
//Deprecated
	//case audiomasterGetNumAutomatableParameters:
		//break;
//Deprecated
	//case audioMasterGetParameterQuantization:
		//break;

	case audioMasterIOChanged:
		break;

//Deprecated
	//case audioMasterNeedIdle:
		//break;

	case audioMasterSizeWindow:
		//! �v���O�C������̃T�C�Y�ύX�v��
		{
			size_t const width = index;
			size_t const height = value;
			vst->SetWindowSize(width, height);
			return 1; //return 1 if supported to set window size
		}

	case audioMasterGetSampleRate:
		//! �T���v�����O���[�g�₢���킹
		return sampling_rate_;

	case audioMasterGetBlockSize:
		// �u���b�N�T�C�Y�₢���킹
		return block_size_;

	case audioMasterGetInputLatency:
		//! ���̓��C�e���V�₢���킹
		return 0;

	case audioMasterGetOutputLatency:
		//! �o�̓��C�e���V�₢���킹
		return 0;

//Deprecated
	//case audioMasterGetPreviousPlug:
		//break;
//Deprecated
	//case audioMasterGetNextPlug:
	//break;
//Deprecated
	//case AudioMasterWillReplaceOrAccumulate:
		//break;

	case audioMasterGetCurrentProcessLevel:
		return kVstProcessLevelUnknown;

	case audioMasterGetAutomationState:
		return kVstAutomationOff;

	case audioMasterOfflineStart:
		break;

	case audioMasterOfflineRead:
		break;

	case audioMasterOfflineWrite:
		break;

	case audioMasterOfflineGetCurrentPass:
		break;

	case audioMasterOfflineGetCurrentMetaPass:
		break;

//Deprecated
	//case audiomasterSetOutputSampleRate:
		//break;
//Deprecated
	//case audioMasterGetputSpeakerArrangement:
		//break;

	case audioMasterGetVendorString:
		{
			static char const vendor_string[] = "hotwatermorning";
			static size_t const vendor_string_len = sizeof(vendor_string)/sizeof(char) - 1;
			BOOST_STATIC_ASSERT(vendor_string_len <= kVstMaxVendorStrLen);
			std::copy(vendor_string, vendor_string + vendor_string_len, static_cast<char *>(ptr));
		}
		return 1;

	case audioMasterGetProductString:
		{
			static char const product_string[] = "Vst Host Test";
			static size_t const product_string_len = sizeof(product_string)/sizeof(char) - 1;
			BOOST_STATIC_ASSERT(product_string_len <= kVstMaxProductStrLen);
			std::copy(product_string, product_string + product_string_len, static_cast<char *>(ptr));
		}
		return 1;

	case audioMasterGetVendorVersion:
		{
			return 1;
		}

	case audioMasterVendorSpecific:
		break;

//Deprecated
	//case audioMasterSetIcon:
		//break;

	case audioMasterCanDo:
		//! �z�X�g���T�|�[�g���Ă���@�\��VST�v���O�C���ɒʒm
		{
			char const *do_list[] = {
				{ "sendVstEvents" },
				{ "sendVstMidiEvents" },
				{ "sizeWindow" },
				{ "startStopProcess" },
				{ "sendVstMidiEventFlagIsRealtime" } };

			for(auto elem: do_list) {
				if(strcmp(elem, static_cast<char const *>(ptr)) == 0) { 
					return 1;
				}
			}
			//don't know
			return 0;
		}

	case audioMasterGetLanguage:
		{
			return kVstLangJapanese;
		}

//Deprecated
	//case audioMasterOpenWindow:
		//break;

//Deprecated
	//case audioMasterCloseWindow:
		//break;

	case audioMasterGetDirectory:
		//! �v���O�C����DLL���܂�ł���f�B���N�g���̖₢���킹
		return reinterpret_cast<VstIntPtr>(vst->GetDirectory());

	case audioMasterUpdateDisplay:
		break;

	case audioMasterBeginEdit:
		break;

	case audioMasterEndEdit:
		break;

	case audioMasterOpenFileSelector:
		break;

	case audioMasterCloseFileSelector:
		break;

//Deprecated
	//case audioMasterEditFile:
		//break;
//Deprecated
	//case audioMasterGetChunkFile:
		//break;
//Deprecated
	//case audioMasterGetInputSpeakerArrangement:
		//break;

	default:
		//unsupported opcodes
		;
	}
	return result;
}

}	//::hwm

#pragma warning(pop)
