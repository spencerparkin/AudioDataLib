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