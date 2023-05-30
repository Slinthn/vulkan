/**
 * @brief Initialise XAudio2
 * 
 */
void audio_init(
    void
){
    IXAudio2 *xaudio2;
    XAudio2Create(&xaudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);

    IXAudio2MasteringVoice *master;
    xaudio2->lpVtbl->CreateMasteringVoice(xaudio2, &master,
        XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE,
        0, 0, 0, 0);

    struct sln_file file = sln_read_file("heheheha.wav", 1);

    WAVEFORMATEX format = {0};  // TODO: all these values are just 'fuck it'
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = 2;
    format.nSamplesPerSec = 48000;
    format.nAvgBytesPerSec = 48000 * 2 * 2;
    format.nBlockAlign = 4;
    format.wBitsPerSample = 16;

    IXAudio2SourceVoice *voice;
    xaudio2->lpVtbl->CreateSourceVoice(xaudio2, &voice, &format, 0, 1, 0, 0, 0);


    XAUDIO2_BUFFER buffer = {0}; // TODO:
    buffer.AudioBytes = 400000;
    buffer.pAudioData = file.data;

    voice->lpVtbl->SubmitSourceBuffer(voice, &buffer, 0);

    voice->lpVtbl->Start(voice, 0, 0);

    // TODO: not a good idea to close the file. It frees up memory and I'm
    // certain that XAudio2 does not clone the buffer
    //sln_close_file(file);
}
