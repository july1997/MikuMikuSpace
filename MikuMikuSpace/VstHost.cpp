#include "VstHost.h"



VstHost::VstHost()
{
}


VstHost::~VstHost()
{
}


void VstHost::loadLibrary(std::string libraryName)
{
    boost::mutex process_mutex;
    auto get_process_lock = [&]() -> boost::unique_lock<boost::mutex> { return boost::make_unique_lock(process_mutex); };
    //! VST�v���O�C���ƁA���[�h���Ă���VST�z�X�g�̊ԂŃf�[�^�����Ƃ肷��N���X
    hwm::HostApplication	hostapp(SAMPLING_RATE, BLOCK_SIZE);

    try
    {
        //! VstPlugin�N���X
        //! VST�v���O�C����C�C���^�[�t�F�[�X�ł���AEffect��ێ����āA���b�v���Ă���
        hwm::VstPlugin vsti(libraryName, SAMPLING_RATE, BLOCK_SIZE, &hostapp);

        if (!vsti.IsSynth())
        {
            MessageBox(GetMainWindowHandle(), "This plugin is an Audio Effect. VST Instrument is expected.", "error", MB_OK);
        }

        //! Wave�o�̓N���X
        //! Windows��Wave�I�[�f�B�I�f�o�C�X���I�[�v�����āA�I�[�f�B�I�̍Đ����s���B
        hwm::WaveOutProcessor	wave_out_;
        //! �f�o�C�X�I�[�v��
        bool const open_device =
            wave_out_.OpenDevice(
                SAMPLING_RATE,
                2,	//2ch
                BLOCK_SIZE,				// �o�b�t�@�T�C�Y�B�Đ����r�؂�鎞�͂��̒l�𑝂₷�B���������C�e���V�͑傫���Ȃ�B
                BUFFER_MULTIPLICITY,	// �o�b�t�@���d�x�B�Đ����r�؂�鎞�͂��̒l�𑝂₷�B���������C�e���V�͑傫���Ȃ�B
                //! �f�o�C�X�o�b�t�@�ɋ󂫂�����Ƃ��ɌĂ΂��R�[���o�b�N�֐��B
                //! ���̃A�v���P�[�V�����ł́A���VstPlugin�ɑ΂��č����������s���A���������I�[�f�B�I�f�[�^��WaveOutProcessor�̍Đ��o�b�t�@�֏�������ł���B
                [&](short * data, size_t device_channel, size_t sample)
        {
            auto lock = get_process_lock();
            //! VstPlugin�ɒǉ������m�[�g�C�x���g��
            //! �Đ��p�f�[�^�Ƃ��Ď��ۂ̃v���O�C�������ɓn��
            vsti.ProcessEvents();
            //! sample���̎��Ԃ̃I�[�f�B�I�f�[�^����
            float **syntheized = vsti.ProcessAudio(sample);
            size_t const channels_to_be_played =
                std::min<size_t>(device_channel, vsti.GetEffect()->numOutputs);

            //! ���������f�[�^���I�[�f�B�I�f�o�C�X�̃`�����l�����ȓ��̃f�[�^�̈�ɏ����o���B
            //! �f�o�C�X�̃T���v���^�C�v��16bit�����ŊJ���Ă���̂ŁA
            //! VST����-1.0 .. 1.0�̃I�[�f�B�I�f�[�^��-32768 .. 32767�ɕϊ����Ă���B
            //! �܂��AVST���ō��������f�[�^�̓`�����l�����Ƃɗ񂪕�����Ă���̂ŁA
            //! Waveform�I�[�f�B�I�f�o�C�X�ɗ����O�ɃC���^�[���[�u����B
            for (size_t ch = 0; ch < channels_to_be_played; ++ch)
            {
                for (size_t fr = 0; fr < sample; ++fr)
                {
                    double const sample = syntheized[ch][fr] * 32768.0;
                    data[fr * device_channel + ch] =
                        static_cast<short>(
                            std::max<double>(-32768.0, std::min<double>(sample, 32767.0))
                        );
                }
            }
        }
            );

        if (open_device == false)
        {
            return;
        }

        vsti.AddNoteOn(60);
        WaitKey();
        vsti.AddNoteOff(60);
        WaitKey();
        //! �I������
        vsti.CloseEditor();
        wave_out_.CloseDevice();
    }
    catch (std::exception &e)
    {
        MessageBox(GetMainWindowHandle(), e.what(), "error", MB_OK);
    }
}