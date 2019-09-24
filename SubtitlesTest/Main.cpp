#pragma warning(disable: 4996) //usage of deprecated function in sphelper.h
#include <sphelper.h>
//#include <ffmpegcpp.h>
#include <memory>
#include <vector>
#include <sstream>
extern "C" {
	#include <libavutil/opt.h>
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libswresample/swresample.h>
}
/*using namespace ffmpegcpp;
class BufferFrameSink : public AudioFrameSink, public FrameWriter {
private:
	std::vector<uint8_t> memoryBuffer;
	FrameSinkStream * fSinkStream;
	int sampleSize, channels, sampleRate, samples, frames;
	uint64_t channelLayout;
	AVSampleFormat fmt;
	FILE * outFile;
public:
	BufferFrameSink() : fSinkStream(nullptr), frames(0) {
//		outFile = fopen("rawData.pcm", "wb");
	};
	FrameSinkStream * CreateStream() override {
//		if (fSinkStream == nullptr)
			fSinkStream = new FrameSinkStream(this, 0);
		return fSinkStream;
	}
	virtual void WriteFrame(int streamIndex, AVFrame* frame, StreamData* streamData) override {
		++frames;
		fmt = static_cast<AVSampleFormat>(frame->format);
		sampleSize = av_get_bytes_per_sample(fmt);
		channels = frame->channels;
		sampleRate = frame->sample_rate;
		samples = frame->nb_samples;
		channelLayout = frame->channel_layout;
		std::vector<uint8_t> buffer(sampleSize * frame->nb_samples * frame->channels);
		for (int i = 0; i < frame->nb_samples; ++i) {
			for (int ch = 0; ch < channels; ++ch) {
				memcpy(&buffer[sampleSize * i * channels + ch * sampleSize], frame->data[ch] + sampleSize * i, sampleSize);
//				fwrite(frame->data[ch] + sampleSize * i, sampleSize, 1, outFile);
			}
		}
		memoryBuffer.insert(memoryBuffer.end(), buffer.begin(), buffer.end());
	}
	virtual void Close(int streamIndex) override {
		if(fSinkStream) delete fSinkStream;
//		fclose(testFile);
	}
	bool IsPrimed() override {
		return true;
	}
	void saveRaw(const char * filename) {
		FILE * testFile = fopen(filename, "wb");
		if (testFile) {
			fwrite(&memoryBuffer[0], sampleSize, samples * frames * channels, testFile);
			fclose(testFile);
		}
	}
	std::vector<uint8_t> getBuffer() {
		return memoryBuffer;
	}
	void fmtAudioForRecognition() {
//		fclose(outFile);
		auto muxer = std::make_unique<Muxer>("temp.wav");
		auto audioCodec = std::make_unique<AudioCodec>(AV_CODEC_ID_PCM_S16LE);
		auto encoder = std::make_unique<AudioEncoder>(audioCodec.get(), muxer.get());
		auto src = std::make_unique<RawAudioFileSource>("rawData.pcm", "f32le", sampleRate, channels, encoder.get());
		src->PreparePipeline();
		while (!src->IsDone()) src->Step();
		muxer->Close();
	}
};*/
//void __stdcall recognizerCallback(WPARAM w, LPARAM l);
std::vector<int16_t> getAudioBuffer(const char * filename);
int main() {
/*	auto demuxer = std::make_unique<Demuxer>("C:\\Users\\stephen\\Downloads\\ffmpeg-20190913-0993327-win64-static\\ffmpeg-20190913-0993327-win64-static\\bin\\subTest.mp4");
	auto audioSink = std::make_unique<BufferFrameSink>();
	demuxer->DecodeBestAudioStream(audioSink.get());
	demuxer->PreparePipeline();
	while (!demuxer->IsDone()) demuxer->Step();
	printf("Decode complete!\n");
	audioSink->saveRaw("rawData.pcm");
	audioSink->fmtAudioForRecognition();*/
	av_register_all();
	if (FAILED(CoInitialize(NULL))) printf("Com failed!\n");
	HRESULT hr;
	CComPtr<ISpObjectToken> recognizerToken;
	hr = SpFindBestToken(SPCAT_RECOGNIZERS, L"language=409", NULL, &recognizerToken); //gets english recognizer
	if (FAILED(hr)) printf("Failed to find recognizer\n");

	CComPtr<ISpStream> inputStream;
	hr = inputStream.CoCreateInstance(CLSID_SpStream);
	if (FAILED(hr)) printf("Failed to create stream\n");
	CSpStreamFormat inputFormat;
	hr = inputFormat.AssignFormat(SPSF_44kHz16BitStereo); //will have to set format based on ffmpeg
	if (FAILED(hr)) printf("Failed to assign format\n");
	auto buffer = getAudioBuffer("VY 2018 trailer.mp4");
	FILE * test = fopen("rawData.pcm", "wb");
	fwrite(&buffer[0], 2, buffer.size(), test);
	fclose(test);
	IStream * stream = SHCreateMemStream(reinterpret_cast<BYTE*>(&buffer[0]), buffer.size() * 2);
	hr = inputStream->SetBaseStream(stream, inputFormat.FormatId(), inputFormat.WaveFormatExPtr());
//	hr = inputStream->BindToFile(L"temp.wav", 
//		SPFM_OPEN_READONLY, &inputFormat.FormatId(), inputFormat.WaveFormatExPtr(), SPFEI_ALL_EVENTS);
	if (FAILED(hr)) printf("Failed to set input stream\n");
	printf("Set stream!\n");

	CComPtr<ISpRecognizer> recognizer;
	CComPtr<ISpRecoContext> context;
	CComPtr<ISpRecoGrammar> grammar;
	hr = recognizer.CoCreateInstance(CLSID_SpInprocRecognizer);
	if (FAILED(hr)) printf("Failed to create recognizer\n");
	hr = recognizer->SetRecognizer(recognizerToken);
	if (FAILED(hr)) printf("Failed to set recognizer\n");
	hr = recognizer->SetInput(inputStream, TRUE);
	if (FAILED(hr)) printf("Failed to set input\n");
	hr = recognizer->CreateRecoContext(&context);
	if (FAILED(hr)) printf("Failed to create context\n");
//	hr = context->SetNotifyCallbackFunction(recognizerCallback, reinterpret_cast<WPARAM>(context.p), 0);
//	if (FAILED(hr)) printf("Failed to set callback\n");
//	printf("Set callback!\n");

	const ULONGLONG ullInterest =
		SPFEI(SPEI_SOUND_START) | SPFEI(SPEI_SOUND_END) |
		SPFEI(SPEI_PHRASE_START) | SPFEI(SPEI_RECOGNITION) |
		SPFEI(SPEI_FALSE_RECOGNITION) | SPFEI(SPEI_HYPOTHESIS) |
		SPFEI(SPEI_INTERFERENCE) | SPFEI(SPEI_RECO_OTHER_CONTEXT) |
		SPFEI(SPEI_REQUEST_UI) | SPFEI(SPEI_RECO_STATE_CHANGE) |
		SPFEI(SPEI_PROPERTY_NUM_CHANGE) | SPFEI(SPEI_PROPERTY_STRING_CHANGE |
		SPFEI(SPEI_START_SR_STREAM) | SPFEI(SPEI_END_SR_STREAM));

	hr = context->SetInterest(ullInterest, ullInterest); //bind callback to event
	if (FAILED(hr)) printf("Failed to set interests\n");

	hr = context->CreateGrammar(0, &grammar);
	if (FAILED(hr)) printf("Failed to create grammar\n");
	hr = grammar->LoadDictation(NULL, SPLO_STATIC);
	if (FAILED(hr)) printf("Failed to load dictation\n");
	hr = grammar->SetDictationState(SPRS_ACTIVE);
	if (FAILED(hr)) printf("Failed to start dictation\n");
	hr = recognizer->SetRecoState(SPRST_ACTIVE_ALWAYS);
	if (FAILED(hr)) printf("Failed to start recognizer\n");
	printf("Begin recognition\n");

	HANDLE hSpeechNotify = INVALID_HANDLE_VALUE;
	context->SetNotifyWin32Event();
	hSpeechNotify = context->GetNotifyEventHandle();

	bool exit = false;
	FILE * subtitlesFile = fopen("subtitles.srt", "w");
	int phrases = 1;
	while (!exit) {
		DWORD msg = WaitForMultipleObjects(1, &hSpeechNotify, TRUE, INFINITE);
		CSpEvent ev;
		while (ev.GetFrom(context) == S_OK) {
			switch (ev.eEventId) {
			case SPEI_RECOGNITION:
			{
				SPPHRASE * phrase;
				SPRECORESULTTIMES time;
				WCHAR * txt = nullptr;
				CComPtr<ISpRecoResult> recoRes = ev.RecoResult();
				if (SUCCEEDED(recoRes->GetPhrase(&phrase))) {
					hr = recoRes->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, TRUE, &txt, NULL);
					if (SUCCEEDED(hr)) {
						wprintf(txt);
						wprintf(L"\n");

/*						const ULONG altNum = 5;
						ULONG alternatives;
						CComPtr<ISpPhraseAlt> alts[altNum];
						hr = recoRes->GetAlternates(phrase->Rule.ulFirstElement, phrase->Rule.ulCountOfElements, altNum, &alts[0], &alternatives);
						if (SUCCEEDED(hr)) {
							for (ULONG i = 0; i < alternatives; ++i) {
								WCHAR * alt = nullptr;
								hr = alts[i]->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, TRUE, &alt, NULL);
								if (SUCCEEDED(hr)) {
									wprintf(L"Alternative: %s\n", alt);
									CoTaskMemFree(alt);
								}
							}
						}*/
					}

				}
				if (SUCCEEDED(recoRes->GetResultTimes(&time))) {
					ULARGE_INTEGER startTime, endTime;
					startTime.HighPart = time.ftStreamTime.dwHighDateTime;
					startTime.LowPart = time.ftStreamTime.dwLowDateTime;
					startTime.QuadPart += time.ullStart;
					endTime = startTime;
					endTime.QuadPart += time.ullLength;
					FILETIME stime, etime;
					stime.dwHighDateTime = startTime.HighPart;
					stime.dwLowDateTime = startTime.LowPart;
					etime.dwHighDateTime = endTime.HighPart;
					etime.dwLowDateTime = endTime.LowPart;
					SYSTEMTIME start, end;
					FileTimeToSystemTime(&stime, &start);
					FileTimeToSystemTime(&etime, &end);
					int len = lstrlenW(txt);
					if (len > 0) {
						auto mb = std::make_unique<char[]>(len + 1);
						wcstombs(mb.get(), txt, len);
						mb[len] = '\0';
						fprintf(subtitlesFile, "%d\n", phrases);
						fprintf(subtitlesFile, "%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\n",
							start.wHour, start.wMinute, start.wSecond, start.wMilliseconds, end.wHour, end.wMinute, end.wSecond, end.wMilliseconds);
						fprintf(subtitlesFile, "%s\r\n", mb.get());
					}
					
				}
				CoTaskMemFree(phrase);
				if(txt != nullptr) CoTaskMemFree(txt);
				++phrases;
				break;
			}
			case SPEI_START_SR_STREAM:
				printf("Begin recognition\n");
				break;
			case SPEI_END_SR_STREAM:
				printf("Recognition finished!\n");
				exit = true;
				break;
			}
		}
	}
	fclose(subtitlesFile);
	getchar();
	stream->Release();
	CoUninitialize();
}

std::vector<int16_t> getAudioBuffer(const char * filename)
{
	std::vector<int16_t> data;

	AVFormatContext * fmtCtx = avformat_alloc_context();
	if (avformat_open_input(&fmtCtx, filename, NULL, NULL) != 0) printf("Failed to open file\n");
	if(avformat_find_stream_info(fmtCtx, NULL) < 0) printf("Could not get stream info!\n");
//	av_dump_format(fmtCtx, 0, filename, 0);
	AVStream * audioStream = nullptr;
/*	for (int i = 0; i < fmtCtx->nb_streams; ++i) {
		if (fmtCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
			audioStream = fmtCtx->streams[i];
			break;
		}
	}*/
	AVCodec * decoder;
	int r = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, &decoder, 0);
	if (r != AVERROR_STREAM_NOT_FOUND) audioStream = fmtCtx->streams[r];
	else printf("Could not find stream!\n");
//	decoder = avcodec_find_decoder(audioStream->codec->codec_id);
	AVCodecContext * codec = avcodec_alloc_context3(decoder);
	avcodec_copy_context(codec, audioStream->codec);
	if (avcodec_open2(codec, decoder, NULL) < 0) printf("Could not open or find decoder!\n");

	SwrContext * refmt = swr_alloc();
	av_opt_set_int(refmt, "in_channel_count", codec->channels, 0);
	av_opt_set_int(refmt, "out_channel_count", 2, 0);
	av_opt_set_int(refmt, "in_channel_layout", codec->channel_layout, 0);
	av_opt_set_int(refmt, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
	av_opt_set_int(refmt, "in_sample_rate", codec->sample_rate, 0);
	av_opt_set_int(refmt, "out_sample_rate", 44100, 0);
	av_opt_set_sample_fmt(refmt, "in_sample_fmt", codec->sample_fmt, 0);
	av_opt_set_sample_fmt(refmt, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
	swr_init(refmt);
	if (!swr_is_initialized(refmt)) printf("Resampler not initalized!\n");
//	printf("Swr init\n");

	AVPacket pack;
	av_init_packet(&pack);
	while (av_read_frame(fmtCtx, &pack) >= 0) {
		if (pack.stream_index != r) continue; //if not an audio packet
		AVFrame * frame = av_frame_alloc();
/*		int success;
		if (avcodec_decode_audio4(codec, frame, &success, &pack) < 0) {
			printf("Error decoding\n");
			break;
		}
		if (!success) continue;*/
		int ret = avcodec_send_packet(codec, &pack);
		if (ret < 0) {
			printf("Error sending packet!\n");
			continue;
		}
		if (!frame) printf("Error alloc frame!\n");
		while (ret >= 0) {
			ret = avcodec_receive_frame(codec, frame);
			if (ret >= 0) {
				int16_t * buffer;
				if (av_samples_alloc(reinterpret_cast<uint8_t**>(&buffer), NULL, 2, frame->nb_samples, AV_SAMPLE_FMT_S16, 0) < 0) printf("Error alloc!\n"); //allocates mem for 2 channels
				int sampleNum = swr_convert(refmt, reinterpret_cast<uint8_t**>(&buffer), frame->nb_samples, const_cast<const uint8_t**>(reinterpret_cast<uint8_t**>(frame->data)), frame->nb_samples);
				if (sampleNum < 0) printf("Convert error!\n");
				data.insert(data.end(), buffer, buffer + sampleNum * 2); //2 channels
//				av_freep(buffer);
			}
		}
		av_frame_free(&frame);
	}

	swr_free(&refmt);
	avcodec_free_context(&codec);
	avformat_close_input(&fmtCtx);
	avformat_free_context(fmtCtx);

	printf("Decoded!\n");
	return data;
}
