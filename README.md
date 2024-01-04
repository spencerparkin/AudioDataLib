# AudioDataLib

This is a C++ library that helps you work with audio data and audio files.  It does not provide access to
audio hardware, but it can help you feed audio to and consume audio from an audio device using some other
audio API, such as SDL, for example.

## Dependencies

There are none.  You just need to be able to compile it.  I've only tried to compile it using Visual Studio,
but there's no reason it couldn't be ported to some other tool-chain or non-Windows platform.

## Maturity

This library is quite new, and at the time of this writing, needs to be battle tested before I recommend anyone
actually trying to use it.  I have one application in mind, and will report back here once I've encorporated
the library into that project.  If it works great and reliably, then I'll cite that here as a sign of at least
some maturity or reliability in the software.

## Usage

For all use-cases, we assume we're in the `ParseDataLib` namespace, and have the following headers.

```
#include <WaveFormat.h>
#include <AudioData.h>
#include <AudioSink.h>
#include <ByteStream.h>
#include <Mutex.h>

using namespace ParseDataLib;
```

### Opening a WAV File

Opening a WAV can be done as follows.  Don't forget to free the returned audio data.  Error-handling is omitted.

```
FileInputStream inputStream("path/to/myaudio.wav");

std::string error;
AudioData* audioData = nullptr;

WaveFormat waveFormat;
waveFormat.ReadFromStream(inputStream, audioData, error);

double clipLengthSeconds = audioData->GetTimeSeconds();
printf("Clip length is %f seconds.\n", clipLengthSeconds);

delete audioData;
```

### Saving a WAV File

Saving a WAV file is very similar.

```
FileOutputStream outputStream("path/to/myaudio.wav");

std::string error;
AudioData* audioData = nullptr;     // Pretend this was initialized to something non-null.

WaveFormat waveFormat;
waveFormat.WriteToStream(outputStream, audioData, error);
```

### Converting Audio

To convert some audio data from one format to another, you can use the `AudioSink` class as follows.

```
// Again, pretend this is non-null, and that you got it from somewhere cool.
AudioData* audioDataIn = nullptr;

// Choose an audio format for our generated output.
AudioData::Format outputFormat;
outputFormat.bitsPerSample = 32;
outputFormat.numChannels = 1;
outputFormat.framesPerSecond = 96000;

// Configure our audio sink.
AudioSink audioSink(true);
audioSink.AddAudioInput(new AudioStream(audioDataIn));
audioSink.SetAudioOutput(new AudioStream(outputFormat));

// Pump converted audio into the generated output stream.
double clipLengthSeconds = audioDataIn->GetTimeSeconds();
audioSink.GenerateAudio(clipLengthSeconds, 0.0);

// Now go grab the generated audio.
AudioData* audioDataOut = new AudioData();
audioDataOut->SetFormat(outputFormat);
audioDataOut->SetAudioBufferSize(outputFormat.BytesFromSeconds(clipLengthSeconds));
audioSink.GetAudioOutput()->ReadBytesFromStream(audioDataOut->GetAudioBuffer(), audioDataOut->GetAudioBufferSize());

// Do something snazzy with the converted audio data here.

// Don't forget to clean-up the memory.
delete audioDataOut;
```

### Mixing Audio

The same class (`AudioSink`) used to convert audio can also be used to mix it.

```
// Yet again, pretend you got some way-cool audio from somewhere dope, yo.  (Pretend these are non-null.)
AudioData* audioDataInA = nullptr;
AudioData* audioDataInB = nullptr;

// Configure our audio sink.
AudioSink audioSink(true);
audioSink.AddAudioInput(new AudioStream(audioDataInA));
audioSink.AddAudioInput(new AudioStream(audioDataInB));
audioSink.SetAudioOutput(new AudioStream(audioDataInA->GetFormat()));

// Pump mixed audio into the generated output stream.
double clipLengthSeconds = ADL_MAX(audioDataInA->GetTimeSeconds(), audioDataInB->GetTimeSeconds());
audioSink.GenerateAudio(clipLengthSeconds, 0.0);

// Grab the generated audio just as we did in the last example.
```

### Streaming Audio to an Output Device

As you can see, the `AudioSink` class is a mechanism for converting and/or mixing audio, but it is
also designed to pump audio to an audio output device in real-time.  The typical use-case is that
you've opened such a device (using some other library like SDL or DirectSound), you've setup your
instances of the `AudioSink` class, and now you occationally want to play sound effects.  Your
function to play a sound effect might look as follows.

```
void MyAudioSubSystem::PlaySoundFX(const AudioData* soundFXData)
{
    this->audioSink.AddAudioInput(new AudioStream(soundFXData));
}
```

And that's it.  Once the stream is depleted, the given audio stream here is deleted by the sink.
But what about feeding the audio to the device?  Typically, audio device libraries have you
implement a callback function.  This function is called by the underlying audio API when the
audio device wants more audio.  It is up to you to give it something to render as actual sound.
It might look like something as follows.

```
/*static*/ void MyAudioSubSystem::AudioCallback(char* audioBuffer, int audioBufferSize, void* userData)
{
    auto audioSystem = static_cast<MyAudioSubSystem*>(userData);
    audioSystem->GrabAudio((uint8_t*)audioBuffer, (uint64_t)audioBufferSize);
}

void MyAudioSubSystem::GrabAudio(uint8_t* audioBuffer, uint64_t audioBufferSize)
{
    // We're being called because the audio driver needs more audio.  Give it what we have.
    uint64_t numBytesRead = this->audioSink.GetAudioOutput()->ReadBytesFromStream(audioBuffer, audioBufferSize);

    // If this loop happens, it means we failed keep up with the demand from the audio device.
    for(uint64_t i = numBytesRead; i < audioBufferSize; i++)
        audioBuffer[i] = 0;     // Fill the remainder of the buffer with silence.
}
```

In order to keep up with demand, you would need to pump the audio somewhere in the main
loop of your program.  That would be something like the following.

```
void MyAudioSubSystem::PumpAudio()
{
    // Make sure we have 4 future seconds of audio ready to be consumed by the audio device.
    // If our future reserve dips below 4 seconds, add at least 2 seconds more of audio to
    // the generated output audio stream.
    this->audioSink.GenerateAudio(4.0, 2.0);
}
```

It's important to note that pumping audio and feeding audio are almost certainly going to
be happening on *different* threads!  To account for that, you can provide a thread-safe
audio stream when configuring your `AudioSink` class instance.

```
void MyAudioSubSystem::Initialize()
{
    // Among other initialization needs, we do this.
    this->mutex = new MyAudioMutex();
    this->audioSink.SetAudioOutput(new ThreadSafeAudioStream(this->desiredAudioFormat, this->mutex, true));
}
```

The class `MyAudioMutex` is a derivative of the `AudioDataLib::Mutex` class, because I didn't
want the audio data library to depend on any platform-specific thing, like a mutex.
(I think standard C++ has some mutex stuff in it, so I might try defaulting to that at some point.)
In any case, you might do the following.

```
class MyAudioMutex : public AudioDataLib::Mutex
{
public:
    MyAudioMutex() {}
    virtual ~MyAudioMutex() {}

    virtual void Lock() override { /* Write your mutex lock code here. */ }
    virtual void Unlock() override { /* Write your mutex unlock code here. */ }
};
```

## Plans

I'd like to add support for more file formats and stream formats other than just PCM at some point.
Some synthasis support would be good to add.  Being able to add an echo, for example, might be
interesting.  Support for some signal analysis using FFTs would be interesting too.
This is all pie-in-the-ski right now as I'm sure the library in its current form is,
admittedly, mediocre at best.