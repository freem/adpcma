< adpcma - a command line YM2610 adpcm-a sample encoder for Neo-Geo games >
a little old, but still tastes good! Release 3 (+readme update)
================================================================================
Notice: While not as user-friendly as a GUI program, the addition of .wav file
support should make this a bit easier to use than the last two versions (if you
happened to be one of the lucky few who got the binaries).

"I used your ADPCM-A-encoder for the sample in my twister-intro... works great,
no problems so far! :-)" - blastar, http://www.yaronet.com/board/topic/170722#post-3

So apparently this works on hardware. Now I don't feel so bad releasing it :p
freem says hello.

The author recommends using smkdan's demo, found at the following address:
https://wiki.neogeodev.org/index.php?title=Playing_sound_samples
for quickly testing ADPCM-A playback.

================================================================================
[Usage]
================================================================================
adpcma (infile) [outfile]

infile is required.
outfile is optional. If omitted, the output file is named "out.pcma".

================================================================================
[Input Files]
================================================================================
Your audio must match the following parameters:
* 18500Hz sampling rate (18.5kHz)
* 16 bit signed PCM
* Mono (1 channel; 2 channels/Stereo playback is warped/slower.)

Input files can be one of two types:
* Raw data (.raw)
* Microsoft PCM Wave files (.wav)

<Raw Data>
Audacity is able to produce the raw data format via "Export Audio".
Save as "other uncompressed files", then hit "Options" and use these settings:
Header   - RAW (header-less)
Encoding - Signed 16 bit PCM

Other sound editing programs are probably able to export this format as well.

<Microsoft PCM Wave>
New in version 3 is the ability to be lazy and just throw a .wav file right at
the program. However, the .wav file must follow the parameters listed above. In
the case of any errors, the program will let you know.

--------------------------------------------------------------------------------
(Digressionary Note)
The "adpcm_yamaha" codec in ffmpeg is more like ADPCM-B, so don't think you can
take any shortcuts by using that. The author has not tested it, and is therefore
not sure if it's YM2610 ADPCM-B compatible.

================================================================================
[Output Files]
================================================================================
This program does not manage a sound library (multiple sounds in one file, e.g.
a V ROM). That would be a separate tool entirely.

If you used the first version of this encoder, you might have noticed the last
line of output has changed. Unlike the first release, this version _will_ pad
the output files to a multiple of 256 using 0x80 (decimal 128) as the pad byte.
This value was chosen by attempting to encode silence and getting a large number
of 0x80 and 0x08 values. 0x80 seems more reasonable for signed values than 8.

================================================================================
[What about ADPCM-B?]
================================================================================
There's been a bunch of ADPCM-B tools already, namely "ADPCM_Encode"/adpcmb.c by
Fred/FRONT and Valley Bell. If you need them, perhaps try Google or DuckDuckGo?

================================================================================
[Sampling Rate Discussion]
================================================================================
There's been some discussion about sample rate.

Yamaha's YM2610 application manual says "18.5KHz", which implies 18500Hz.

There is a post on 2ch that says "18.518kHz", which feels wrong.
http://nozomi.2ch.sc/test/read.cgi/gamemusic/1411480635/

It should be noted that 18500/55500 is equal to 1/3...

================================================================================
[Comparisons Between Encoders]
================================================================================
Ideally, this encoder would be able to match the output of the one SNK provided
to third parties, but that would take a lot of work. Furthermore, nobody has
come forward with said encoder, and it's highly unlikely that it will surface
any time soon.

That being said, the author knows about the following ADPCM-A encoders:
* "adpcma", this program.
* whatever smkdan was using to produce the files for his demo.
* MVSTracker, which creates quiet and/or broken ADPCM-A files.
* the official SNK encoder, only available to SNK and third party devs.
* Jeff Kurtz's Neo Sound Builder, not currently available to the public.
It is likely that more exist and have not been publicized.

The source code for MVSTracker is public, and is likely the first port of call
for anyone developing an ADPCM-A encoder. Unfortunately, the algorithms used in
it provide less than optimal results. HPMAN ended up creating some example
encoder code in C# based on the MVSTracker and MAME codebases, which is what
this encoder is based on.

================================================================================
[EOF]
