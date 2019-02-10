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
    //! VSTプラグインと、ロードしているVSTホストの間でデータをやりとりするクラス
    hwm::HostApplication	hostapp(SAMPLING_RATE, BLOCK_SIZE);

    try
    {
        //! VstPluginクラス
        //! VSTプラグインのCインターフェースであるAEffectを保持して、ラップしている
        hwm::VstPlugin vsti(libraryName, SAMPLING_RATE, BLOCK_SIZE, &hostapp);

        if (!vsti.IsSynth())
        {
            MessageBox(GetMainWindowHandle(), "This plugin is an Audio Effect. VST Instrument is expected.", "error", MB_OK);
        }

        //! Wave出力クラス
        //! WindowsのWaveオーディオデバイスをオープンして、オーディオの再生を行う。
        hwm::WaveOutProcessor	wave_out_;
        //! デバイスオープン
        bool const open_device =
            wave_out_.OpenDevice(
                SAMPLING_RATE,
                2,	//2ch
                BLOCK_SIZE,				// バッファサイズ。再生が途切れる時はこの値を増やす。ただしレイテンシは大きくなる。
                BUFFER_MULTIPLICITY,	// バッファ多重度。再生が途切れる時はこの値を増やす。ただしレイテンシは大きくなる。
                //! デバイスバッファに空きがあるときに呼ばれるコールバック関数。
                //! このアプリケーションでは、一つのVstPluginに対して合成処理を行い、合成したオーディオデータをWaveOutProcessorの再生バッファへ書き込んでいる。
                [&](short * data, size_t device_channel, size_t sample)
        {
            auto lock = get_process_lock();
            //! VstPluginに追加したノートイベントを
            //! 再生用データとして実際のプラグイン内部に渡す
            vsti.ProcessEvents();
            //! sample分の時間のオーディオデータ合成
            float **syntheized = vsti.ProcessAudio(sample);
            size_t const channels_to_be_played =
                std::min<size_t>(device_channel, vsti.GetEffect()->numOutputs);

            //! 合成したデータをオーディオデバイスのチャンネル数以内のデータ領域に書き出し。
            //! デバイスのサンプルタイプを16bit整数で開いているので、
            //! VST側の-1.0 .. 1.0のオーディオデータを-32768 .. 32767に変換している。
            //! また、VST側で合成したデータはチャンネルごとに列が分かれているので、
            //! Waveformオーディオデバイスに流す前にインターリーブする。
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
        //! 終了処理
        vsti.CloseEditor();
        wave_out_.CloseDevice();
    }
    catch (std::exception &e)
    {
        MessageBox(GetMainWindowHandle(), e.what(), "error", MB_OK);
    }
}