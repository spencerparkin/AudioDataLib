# AudioDataLib

When working with audio libraries (e.g., DirectSound, Oboe, SDL, etc.) that sit between you and the hardware, what they *do*
for you is provide a way to send or receive an audio stream to or from the hardware.  What they typically do *not do*
is provide a way to load or save such audio data, convert it, mix it, or buffer it for real-time consumption.  *That*,
however, is what this *this* library *does*.

Written in C++, this library presently supports WAV and MIDI files.  For real-time playback, it can stream encoded
wave-form data to an audio device, and it can send appropriately timed MIDI messages to a MIDI device on a port.

## Dependencies

There are none, unless you count the standard C++ library.  You just need to be able to compile it.  I've compiled it on
Windows using Visual Studio, and it's also been compiled using Android Studio's tool-chain for mobile.

## Maturity

This library is quite new, and at the time of this writing, needs to be battle-tested before I would recommend anyone
actually trying to use it.  I currently have one application for it in an Android mobile app, and it seems to be working
quite well for me there.

## Usage

For all use-cases, we assume we're in the `AudioDataLib` namespace, and have the following headers.

```C++
#include <WaveFormat.h>
#include <AudioData.h>
#include <AudioSink.h>
#include <ByteStream.h>
#include <Mutex.h>
#include <MidiData.h>
#include <MidiPlayer.h>

using namespace AudioDataLib;
```

### Opening a WAV File

Opening a WAV can be done as follows.  Don't forget to free the returned audio data.  Error-handling is omitted.

```C++
FileInputStream inputStream("path/to/myaudio.wav");

std::string error;
FileData* fileData = nullptr;
WaveFormat waveFormat;
waveFormat.ReadFromStream(inputStream, fileData, error);

auto audioData = dynamic_cast<AudioData*>(fileData);

double clipLengthSeconds = audioData->GetTimeSeconds();
printf("Clip length is %f seconds.\n", clipLengthSeconds);

AudioData::Destroy(audioData);
```

### Saving a WAV File

Saving a WAV file is very similar.

```C++
FileOutputStream outputStream("path/to/myaudio.wav");

std::string error;
AudioData* audioData = nullptr;     // Pretend this was initialized to something non-null.

WaveFormat waveFormat;
waveFormat.WriteToStream(outputStream, audioData, error);
```

### Converting Audio

To convert some audio data from one format to another, you can use the `AudioSink` class as follows.

```C++
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
AudioData::Destroy(audioDataOut);
```

### Mixing Audio

The same class (`AudioSink`) used to convert audio can also be used to mix it.

```C++
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
instance of the `AudioSink` class, and now you occationally want to play sound effects.  Your
function to play a sound effect might look as follows.

```C++
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

```C++
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

```C++
void MyAudioSubSystem::PumpAudio()
{
    // Make sure we have 0.01 future seconds of audio ready to be consumed by the audio device.
    // If our future reserve dips below 0.01 seconds, add at least 0.005 seconds more of audio to
    // the generated output audio stream.  I should probably add an overload that works with bytes
    // instead of seconds.
    this->audioSink.GenerateAudio(0.01, 0.005);
}
```

It's important to note that pumping audio and feeding audio are almost certainly going to
be happening on *different* threads!  To account for that, you can provide a thread-safe
audio stream when configuring your `AudioSink` class instance.

```C++
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
In any case, you would need the following.

```C++
class MyAudioMutex : public AudioDataLib::Mutex
{
public:
    MyAudioMutex() {}
    virtual ~MyAudioMutex() {}

    virtual void Lock() override { /* Write your mutex lock code here. */ }
    virtual void Unlock() override { /* Write your mutex unlock code here. */ }
};
```

Note that if a mutex has to block, it could pre-empt the thread for an unbounded amount
of time.  You might consider using a lock-free spin-wait or something like that.  What I've
found in practice is that a regular mutex seems to work just fine.  The mutex provided to
the library is only ever locked long enough to copy a buffer, and that's it.  That's as
tight as I can make the lock/unlock pair.

### Streaming MIDI message to a MIDI Device

Keeping the same philosophy here, the library doesn't deal with MIDI devices directly (one of the
goals is zero dependencies, not counting the standard C++ library), but it does
provide a convenient way to feed a MIDI device (that you open yourself) for real-time playback purposes.
The connected device can then do whatever it wants with the music; typically, it would synthesize it for you
as audible sound, but it could be anything.

Loading some MIDI data into RAM can be done as follows.

```C++
Error error;
FileData* fileData = nullptr;
FileFormat* fileFormat = FileFormat::CreateForFile("path/to/myfile.mid");
FileInputStream inputStream("path/to/myfile.mid");
fileFormat->ReadFromStream(inputSTream, fileData, error);
FileFormat::Destroy(fileFormat);
auto midiData = dynamic_cast<MidiData*>(fileData);

// Do something snazzy with the MIDI data here.

FileData::Destroy(midiData);
```

Sending the MIDI data to a device's input port can be done as follows.

```C++
class MyPlayer : public MidiPlayer
{
public:
    MyPlayer(() : MidiPlayer(nullptr) {}
    virtual ~MyPlayer() {}
    
    virtual bool SendMessage(const uint8_t* message, uint64_t messageSize, Error& error) override
    {
        // Send the given MIDI message to the connected MIDI device's input port here!
    }
};

Error error;
MyPlayer myPlayer;
MidiData* midiData = nullptr;   // Pretend this was initialized with some groovy MIDI data.
myPlayer.SetMidiData(midiData);
myPlayer.SetTimeSeconds(0.0);               // Start at the beginning.
myPlayer.BeginPlayback({1, 2, 3}, error);   // Begin playback of tracks 1, 2, and 3.

while(!myPlayer.NoMoreToPlay())
    myPlayer.ManagePlayback(error);
    
myPlayer.EndPlayback(error);
```

I'll be the first to admin that my API isn't perfect, but the code has worked pretty well
for me so far with the MIDI files I've used with it.  Note that I've found that for accurate
playback, the thread calling the `ManagePlayback` function needs to be a dedicated thread.

## Plans

I'd like to add support for more file formats and stream formats other than just PCM at some point.
Some synthesis support would be good to add, such as generating MIDI sounds from MIDI messages,
which could, in-turn, be pumped to a sound card or to disk as a WAV file.
Certain FX, such as being able to add an echo might be interesting.
Support for some signal analysis using FFTs would be interesting too.
This is all pie-in-the-ski right now as I'm sure the library in its current form is,
admittedly, mediocre at best.

I should at some point support the ability to capture MIDI data into a MIDI file.