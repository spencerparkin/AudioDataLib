# AudioDataLib

When working with audio libraries (e.g., DirectSound, Oboe, SDL, etc.) that sit between you and the hardware, what they *do*
for you is provide a way to send or receive an audio stream to or from the hardware.  What they typically do *not do*
is provide a way to load or save such audio data, convert it, mix it, buffer it, or even synthesize it for real-time consumption.
*That*, however, is what this *this* library *does*.

Current features (done or in the works) are as follows.

 * Load/save WAV files.
 * Load/save MIDI files.
 * Load/save SF2 (sound-font) files.
 * Mix/convert PCM/float audio for real-time playback of SFX.
 * Playback MIDI files to an output MIDI port.
 * Synthesize MIDI messages into real-time audio from an input MIDI port.

## Setup

This project is designed to be cross-platform, but I have only tested it on Windows so far.  In any case, you would do the following from a command shell.

```
git clone https://github.com/spencerparkin/AudioDataLib
cd AudioDataLib
mkdir Build
cd Build
cmake ..
```

The `CMakeLists.txt` files will need to be updated to point to your `wxWidgets` and `SDL` installations if building the tools.  If all you care about
is the library, then it *should* build just fine as it only depends on the C++ standard library.

## Usage

You can do a lot of things with the library, but here's a quick example of just loading WAV file and then printing some info about it.

```C++
#include <WaveFileFormat.h>
#include <AudioData.h>
#include <ByteStream.h>
#include <Error.h>
#include <stdio.h>

using namespace AudioDataLib;

int main(int argc, char** argv)
{
    FileData* fileData = nullptr;
    Error error;

    FileInputStream inputStream("path/to/my_file.wav");

    WaveFileFormat fileFormat;
    fileFormat.ReadFromStream(inputStream, fileData, error);

    auto audioData = dynamic_cast<AudioData*>(fileData);
    if (audioData)
        audioData->DumpInfo(stdout);

    delete fileData;

    return 0;
}
```

My guess is that many will be put off by my like of "modern style", and by my verbose API.  I also don't apologize for heavy use of *OOP*, which has come
under considerable attack in recent years.  And yes, I use explicit `this` everywhere in my code, because implicit `this` was just a bad idea.