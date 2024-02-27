# AudioDataLib

When working with audio libraries (e.g., DirectSound, Oboe, SDL, etc.) that sit between you and the hardware, what they *do*
for you is provide a way to send or receive an audio stream to or from the hardware.  What they typically do *not do*
is provide a way to load or save such audio data, convert it, mix it, buffer it, or even synthesize it for real-time consumption.
*That*, however, is what this *this* library *does*.

Current features are as follows.

 * Load/save WAV files.
 * Load MIDI files.
 * Load SF2 (sound-font) files.
 * Mix/convert PCM/float audio for real-time playback of SFX.  (You provide the audio callback.)
 * Playback MIDI files.  (You provide access to the MIDI port where messages are sent.)
 * Synthesize MIDI messages into real-time, SF-based audio.  (You provide access to the MIDI port where message are received.)

## Documentation

A doxygen-based documentation page has been setup <a href="https://spencerparkin.github.io/AudioDataLib/Documentation/html/index.html">here</a> for the project.  As of this writing, it is a work in progress.

## Setup

This project is designed to be cross-platform, but I have only tested it on Windows so far.  In any case, you would do the following from a command shell.

```
git clone https://github.com/spencerparkin/AudioDataLib
cd AudioDataLib
mkdir Build
cd Build
cmake ..
cmake --build
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
    const char* filePath = "path/to/my_file.wav";
    FileData* fileData = nullptr;
    FileInputStream inputStream(filePath);
    WaveFileFormat fileFormat;
    Error error;

    if(fileFormat.ReadFromStream(inputStream, fileData, error))
        fileData->DumpInfo(stdout);
    else
    {
        fprintf(stderr, "Failed read file: %s\n", filePath);
        fprintf(stderr, "Error: %s\n", error.GetErrorMessage());
    }

    delete fileData;
    return 0;
}
```

There you have it.  Don't be too hard on me.  I worked really hard on this stuff.