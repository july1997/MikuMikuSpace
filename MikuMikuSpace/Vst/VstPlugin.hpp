#pragma once

#include <cstdlib>
#include <vector>
#include <stdexcept>
#include <array>
#include <string>

#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_factories.hpp>

#include <windows.h>

#pragma warning(push)
#pragma warning(disable: 4996)
#include "./vstsdk2.4/pluginterfaces/vst2.x/aeffectx.h"
#pragma warning(pop)

#include "./HostApplication.hpp"

namespace hwm {

//! VST�v���O�C����\���N���X
struct VstPlugin
{
    //! VST�v���O�C���̃G���g���|�C���g��\���֐��̌^
    //! audioMasterCallback�Ƃ����z�X�g���̃R�[���o�b�N�֐���
    //! �n���Ă��̊֐����Ăяo����AEffect *�Ƃ���VST�v���O�C����
    //! C�C���^�[�t�F�[�X�̃I�u�W�F�N�g���Ԃ�B
	typedef AEffect* (*VstPluginEntryProc)(audioMasterCallback callback);

	VstPlugin(
		std::string module_path,
		size_t sampling_rate,
		size_t block_size,
		HostApplication *hostapp )
		//:	module_(module_path.c_str())
		:	hostapp_(hostapp)
		,	is_editor_opened_(false)
		,	events_(0)
	{
		//if(!module_) { throw std::runtime_error("module not found"); }
		directory_ = module_path;
		initialize(sampling_rate, block_size);
	}

	~VstPlugin()
	{
		terminate();
	}

public:
	AEffect *GetEffect() { return effect_; }
	AEffect *GetEffect() const { return effect_; }

	bool	IsSynth() const { return (effect_->flags & effFlagsIsSynth) != 0; }
	bool	HasEditor() const { return (effect_->flags & effFlagsHasEditor) != 0; }

	void	OpenEditor(/*balor::gui::Control &parent*/)
	{
		//parent_ = &parent;
		dispatcher(effEditOpen, 0, 0, /*parent_->handle()*/NULL, 0);

        //! �v���O�C���̃G�f�B�^�[�E�B���h�E���J���ۂɎw�肵���e�E�B���h�E�̃T�C�Y��
        //! �G�f�B�^�[�E�B���h�E�ɍ��킹�邽�߂ɁA�G�f�B�^�[�E�B���h�E�̃T�C�Y���擾����B
		ERect *rect;
		dispatcher(effEditGetRect, 0, 0, &rect, 0);

		SetWindowSize(rect->right - rect->left, rect->bottom - rect->top);

		//parent.visible(true);
		is_editor_opened_ = true;
	}

	void	CloseEditor()
	{
		dispatcher(effEditClose, 0, 0, 0, 0);
		is_editor_opened_ = false;
		//parent_ = nullptr;
	}

	bool	IsEditorOpened() const { return is_editor_opened_; }

    //! �z�X�g�̃R�[���o�b�N�֐��Ƀ��T�C�Y�v���������ۂ�
    //! HostApplication�N���X�̃n���h���ɂ���Ă��̊֐����Ă΂��
	void	SetWindowSize(size_t width, size_t height)
	{
		//parent_->size(
		//	parent_->sizeFromClientSize(balor::Size(width, height))
		//	);
	}

    //! AEffect *�Ƃ����v���O�C����C�C���^�[�t�F�[�X�I�u�W�F�N�g���o�R���āA
    //! ���ۂ�VST�v���O�C���{�̂ɖ��߂𓊂���ɂ́A
    //! opcode�Ƃ���ɕt������p�����[�^��n���āA
    //! dispatcher�Ƃ����֐����Ăяo���B
	VstIntPtr dispatcher(VstInt32 opcode, VstInt32 index, VstIntPtr value, void *ptr, float opt) {
		return effect_->dispatcher(effect_, opcode, index, value, ptr, opt);
	}

	VstIntPtr dispatcher(VstInt32 opcode, VstInt32 index, VstIntPtr value, void *ptr, float opt) const {
		return effect_->dispatcher(effect_, opcode, index, value, ptr, opt);
	}

	HostApplication & GetHost() { return *hostapp_; }
	HostApplication const & GetHost() const { return *hostapp_; }
	std::string	GetEffectName() const { return effect_name_; }
	char const * GetDirectory() const { return directory_.c_str(); }

	size_t GetProgram() const { dispatcher(effGetProgram, 0, 0, 0, 0); }
	void SetProgram(size_t index) { dispatcher(effSetProgram, 0, index, 0, 0); }
	size_t GetNumPrograms() const { return effect_->numPrograms; }
	std::string GetProgramName(size_t index) { return program_names_[index]; }

    //! �m�[�g�I�����󂯎��
    //! ���ۂ̃��A���^�C�����y�A�v���P�[�V�����ł́A
    //! �����Ńm�[�g��񂾂��͂Ȃ����܂��܂�MIDI����
    //! ���m�ȃ^�C�~���O�ŋL�^����悤�ɂ���B
    //! �ȒP�̂��߁A���̃A�v���P�[�V�����ł́A
    //! �m�[�g���𐏎��R���e�i�ɒǉ����A
    //! ���̍��������̃^�C�~���O�œ���VST�v���O�C��
    //! �Ƀf�[�^�������邱�Ƃ����҂�������ɂȂ��Ă���B
	void AddNoteOn(size_t note_number)
	{
		VstMidiEvent event;
		event.type = kVstMidiType;
		event.byteSize = sizeof(VstMidiEvent);
		event.flags = kVstMidiEventIsRealtime;
		event.midiData[0] = static_cast<char>(0x90u);		// note on for 1st channel
		event.midiData[1] = static_cast<char>(note_number);	// note number
		event.midiData[2] = static_cast<char>(0x64u);		// velocity
		event.midiData[3] = 0;				// unused
		event.noteLength = 0;
		event.noteOffset = 0;
		event.detune = 0;
		event.deltaFrames = 0;
		event.noteOffVelocity = 100;
		event.reserved1 = 0;
		event.reserved2 = 0;

		auto lock = get_event_buffer_lock();
		midi_events_.push_back(event);
	}

    //! �m�[�g�I���Ɠ����B
	void AddNoteOff(size_t note_number)
	{
		VstMidiEvent event;
		event.type = kVstMidiType;
		event.byteSize = sizeof(VstMidiEvent);
		event.flags = kVstMidiEventIsRealtime;
		event.midiData[0] = static_cast<char>(0x80u);		// note on for 1st channel
		event.midiData[1] = static_cast<char>(note_number);	// note number
		event.midiData[2] = static_cast<char>(0x64u);		// velocity
		event.midiData[3] = 0;				// unused
		event.noteLength = 0;
		event.noteOffset = 0;
		event.detune = 0;
		event.deltaFrames = 0;
		event.noteOffVelocity = 100;
		event.reserved1 = 0;
		event.reserved2 = 0;

		auto lock = get_event_buffer_lock();
		midi_events_.push_back(event);
	}

    //! �I�[�f�B�I�̍��������ɐ旧���A
    //! MIDI����VST�v���O�C���{�̂ɑ���B
    //! ���̏����͉���ProcesAudio�Ɠ����I�ɍs����ׂ��B
    //! �܂�A���M����ׂ��C�x���g������ꍇ�́A
    //! ProcessAudio�̒��O�Ɉ�x�������̊֐����Ă΂��悤�ɂ���B
	void ProcessEvents()
	{
		{
			auto lock = get_event_buffer_lock();
            //! ���M�p�f�[�^��VstPlugin�����̃o�b�t�@�Ɉڂ��ւ��B
			std::swap(tmp_, midi_events_);
		}

        //! ���M�f�[�^���Ȃɂ�������ΕԂ�B
		if(tmp_.empty()) { return; }

        //! VstEvents�^�́A�����̔z����ϒ��z��Ƃ��Ĉ����̂ŁA
        //! ���M������MIDI�C�x���g�̐��ɍ��킹�ă��������m��
        //! VstEvents�^�ɁA���Ƃ���VstEvent�̃|�C���^����̗̈悪�܂܂�Ă���̂ŁA
        //! ���ۂɊm�ۂ��郁�����ʂ͑��M����C�x���g���������������̂Ōv�Z���Ă���B
        //!
		//! �����Ŋm�ۂ�����������
        //! processReplacing���Ăяo���ꂽ��ŉ������B
		size_t const bytes = sizeof(VstEvents) + sizeof(VstEvent *) * std::max<size_t>(tmp_.size(), 2) - 2;
		events_ = (VstEvents *)malloc(bytes);
		for(size_t i = 0; i < tmp_.size(); ++i) {
			events_->events[i] = reinterpret_cast<VstEvent *>(&tmp_[i]);
		}
		events_->numEvents = tmp_.size();
		events_->reserved = 0;

        //! �C�x���g�𑗐M�B
		dispatcher(effProcessEvents, 0, 0, events_, 0);
	}
	
    //! �I�[�f�B�I��������
	float ** ProcessAudio(size_t frame)
	{
		BOOST_ASSERT(frame <= output_buffers_[0].size());

        //! ���̓o�b�t�@�A�o�̓o�b�t�@�A��������ׂ��T���v�����Ԃ�n����
        //! processReplacing���Ăяo���B
        //! �����v���O�C����double���x�����ɑΉ����Ă���Ȃ�΁A
        //! �������̒i�K��effProcessPrecision��kVstProcessPrecision64���w�肵�A
        //! �����f�[�^�^��float�ł͂Ȃ�double�Ƃ��A
        //! ������processReplacing�̑����processReplacingDouble���g�p����B
		effect_->processReplacing(effect_, input_buffer_heads_.data(), output_buffer_heads_.data(), frame);

        //! �����I���Ȃ̂�
        //! effProcessEvents�ő��M�����f�[�^���������B
		tmp_.clear();
		free(events_);
		events_ = nullptr;

		return output_buffer_heads_.data();
	}

private:
    //! �v���O�C���̏���������
	void initialize(size_t sampling_rate, size_t block_size)
	{
		HMODULE hModule = LoadLibrary((LPCSTR)directory_.c_str());
		if (NULL == hModule)
		{
			throw std::runtime_error("can not open the library");
		}

		//! �G���g���|�C���g�擾
		VstPluginEntryProc proc = (VstPluginEntryProc)GetProcAddress(hModule, "VSTPluginMain");
		if (NULL == proc)
		{		
			//! �Â��^�C�v��VST�v���O�C���ł́A
			//! �G���g���|�C���g����"main"�̏ꍇ������B
			proc = (VstPluginEntryProc)GetProcAddress(hModule, "main");
			if(!proc) { throw std::runtime_error("entry point not found"); }
		}

		AEffect *test = proc(&hwm::VstHostCallback);
		if(!test || test->magic != kEffectMagic) { throw std::runtime_error("not a vst plugin"); }

		effect_ = test;
        //! ���̃A�v���P�[�V������AEffect *�������₷�����邽��
        //! AEffect�̃��[�U�[�f�[�^�̈�ɂ��̃N���X�̃I�u�W�F�N�g�̃A�h���X��
        //! �i�[���Ă����B
		effect_->user = this;

        //! �v���O�C���I�[�v��
		dispatcher(effOpen, 0, 0, 0, 0);
        //! �ݒ�n
		dispatcher(effSetSampleRate, 0, 0, 0, static_cast<float>(sampling_rate));
		dispatcher(effSetBlockSize, 0, block_size, 0, 0.0);
		dispatcher(effSetProcessPrecision, 0, kVstProcessPrecision32, 0, 0);
        //! �v���O�C���̓d���I��
		dispatcher(effMainsChanged, 0, true, 0, 0);
        //! processReplacing���Ăяo�����Ԃ�
		dispatcher(effStartProcess, 0, 0, 0, 0);

        //! �v���O�C���̓��̓o�b�t�@����
		input_buffers_.resize(effect_->numInputs);
		input_buffer_heads_.resize(effect_->numInputs);
		for(int i = 0; i < effect_->numInputs; ++i) {
			input_buffers_[i].resize(block_size);
			input_buffer_heads_[i] = input_buffers_[i].data();
		}

        //! �v���O�C���̏o�̓o�b�t�@����
		output_buffers_.resize(effect_->numOutputs);
		output_buffer_heads_.resize(effect_->numOutputs);
		for(int i = 0; i < effect_->numOutputs; ++i) {
			output_buffers_[i].resize(block_size);
			output_buffer_heads_[i] = output_buffers_[i].data();
		}

        //! �v���O�C�����̎擾
		std::array<char, kVstMaxEffectNameLen+1> namebuf = {};
		dispatcher(effGetEffectName, 0, 0, namebuf.data(), 0);
		namebuf[namebuf.size()-1] = '\0';
		effect_name_ = namebuf.data();

        //! �v���O����(�v���O�C���̃p�����[�^�̃v���Z�b�g)���X�g�쐬
		program_names_.resize(effect_->numPrograms);
		std::array<char, kVstMaxProgNameLen+1> prognamebuf = {};
		for(int i = 0; i < effect_->numPrograms; ++i) {
			VstIntPtr result = 
				dispatcher(effGetProgramNameIndexed, i, 0, prognamebuf.data(), 0);
			if(result) {
				prognamebuf[prognamebuf.size()-1] = '\0';
				program_names_[i] = std::string(prognamebuf.data());
			} else {
				program_names_[i] = "unknown";
			}
		}
	}

    //! �I������
	void terminate()
	{
		if(IsEditorOpened()) {
			CloseEditor();
		}
	
		dispatcher(effStopProcess, 0, 0, 0, 0);
		dispatcher(effMainsChanged, 0, false, 0, 0);
		dispatcher(effClose, 0, 0, 0, 0);
	}

private:
	HostApplication *hostapp_;
	AEffect *effect_;

	std::vector<std::vector<float>>	output_buffers_;
	std::vector<std::vector<float>> input_buffers_;
	std::vector<float *>			output_buffer_heads_;
	std::vector<float *>			input_buffer_heads_;
	boost::mutex mutable			event_buffer_mutex_;
	std::vector<VstMidiEvent>		midi_events_;
	bool							is_editor_opened_;
	std::string						effect_name_;
	std::string						directory_;
	std::vector<std::string>		program_names_;
	std::vector<VstMidiEvent> tmp_;
	VstEvents *events_;

	boost::unique_lock<boost::mutex>
			get_event_buffer_lock() const { return boost::make_unique_lock(event_buffer_mutex_); }
};

}

