# Arduino Audio Quilt
A sample player for Arduino and Sparkfun MP3 Shield with seperate banks and track rotation ability

## How To Use

Copy your samples in the format TRACK_A1.MP3 where A is the Sample Bank and 1 is the Sample Number.

A Typical Setup might look like :

TRACK_A1.MP3

TRACK_A2.MP3

TRACK_A3.MP3

TRACK_A4.MP3

TRACK_A6.MP3

TRACK_B1.MP3

TRACK_B2.MP3

TRACK_B3.MP3

TRACK_B4.MP3

TRACK_C1.MP3

TRACK_C2.MP3

TRACK_C3.MP3


_note:_ you will also need the other files that live in the SD Card folder as these are used by the MP3Shield.

IF YOU PLAN ON HAVING MORE SAMPLES THAN 9, PLEASE CHANGE THE BANKlIMITS VARIABLE IN THE SOURCE CODE



##  Limitations.

Understanding that every byte streamed to the VS10xx needs also to be read from the SdCard over the same shared SPI bus, resulting in the SPI bus being less than half as efficient. Along with overhead. Depending upon the Bitrate of the file being streamed to the VSdsp, there is only so much Real Time available. This may impact the performance of high bit-rate audio files being streamed. Additionally the Play Speed Multiplier feature can be exhausted quickly. Where on a typical UNO there is plenty of real-time to transfer good quality files, with CPU to spare for other tasks, assuming they do not consume too much time either.

The available CPU can be increased by either or both increasing the speed of the SPI and or the Arduino F_CPU. Where the Speed of the SPI is individually maintained by both this driver and SdFatLib. As not to or be interfered with each other and or other libraries using the same SPI bus. The SdCard can be increased from SPI_HALF_SPEED to SPI_FULL_SPEED argument in the SD.begin. Where this library will set the Read and Write speeds to the VSdsp correspondingly, based on F_CPU of the Arduino.

The actual consumed CPU utilization can be measured by defining the [PERF_MON_PIN][21] to a valid pin, which generates a low signal on configured pin while servicing the VSdsp. This is inclusive of the SdCard reads.

The below table show's typical average CPU utilizations of the same MP3 file that has been resampled to various bit rates and using different configurations. Where a significant difference is observed in performance.

| BitRate   | SdCard    | Refilling | IDLE |
| 128K      | Half      | 12%       | 88%  |
| 128K      | Full      | 10%       | 90%  |
| 96K       | Full      | 7%        | 93%  |
| 56K       | Full      | 4%        | 96%  |

_Note_
: Only F_CPU of 8MgHz and 16Hz are suppored. Others will default to SPI_CLOCK_DIV2, assuming 4MgHz.

The VS10xx chips are DSP's that run firmware out of ROM, that is internal to the VS10xx chip itself. Where the VSdsp's RAM can additionally be loaded with externally provided firmware and executed, also known as patches or plug-ins, over the SPI port and executed. This allows the VSdsp to have a method for both fixing problems that may exist in the factory ROM's firmware and or add new features provided by [VLSI's website][22]. It is even possible to write your own custom VSdsp code, using there Integrated Development Tools (VSIDE).

By storing them on the SdCard these plug-ins do not consume the Arduino's limited Flash spaces

Below are pre-compiled binary's of corresponding provided VSLI patches/plugins. The filenames are kept short as SdCard only support 8.3.

    .pcm.053       .vs1053-pcm110vs1053pcm.plg
    .admxleft.053  .vs1053b-admix130admix-left.plg
    .admxmono.053  .vs1053b-admix130admix-mono.plg
    .admxrght.053  .vs1053b-admix130admix-right.plg
    .admxster.053  .vs1053b-admix130admix-stereo.plg
    .admxswap.053  .vs1053b-admix130admix-swap.plg
    .patchesf.053  .vs1053b-patches195vs1053b-patches-flac.plg
    .patches.053   .vs1053b-patches195vs1053b-patches.plg
    .rtmidi.053    .vs1053b-rtmidistartrtmidistart.plg
    .eq5.053       .vs1053b-eq5-090vs1053b-eq5.plg

Note
: All plugins should be placed in the root of the SdCard.


## Trouble Shooting.

The below is a list of basic questions to ask when attempting to determine the problem.

* Did it initially **PRINT** the available RAM and Full Help Menu?
    * On initialisation dod it provide a opening print indicating the amount of available SRAM and full menu help. If you don't see this the problem is between your Target and IDE. And likely not this library
    * Is Serial Monitor set to the correct tty or com port and 115200 baud rate? Did you change the baud rate?
    * Reset the Arduino after Serial Monitor is open or send any key. It may have printed these prior to the Serial Monitor being started.
* **WHAT** is the Error reported?
    * Is the Error Code is indicating a file problem.
    * Are the filenames 8.3 format? See below warning.
* Did the SdCard **LOAD**?
    * Try reseating your SdCard.
* Is **MP3player.begin()** locking up, in [setup()][19]?
* Why does my Serial Monitor display: "`...do not have a sd.begin in the main sketch, See Trouble Shooting Guide.`"
* Compiler Error: "`...undefined reference to `sd'`"
* Is the last thing printed to the Serial Monitor: "`Free RAM = 1097 Should be a base line of 1095, on ATmega328 when using INTx`" then nothing...
    * Versions after 1.01.00 require the `SdFat::begin()` to be initialized in the main sketch.ino, as shown in the below example. This provides more immediate access to the SdCard's files by the main sketch. However, if not done there is no immediate compiler error and the sketch will lock up after as it attempts [SFEMP3Shield::begin][28].

...

if(![sd][29].begin([SD_SEL][30], SPI_HALF_SPEED)) [sd][29].initErrorHalt();

...

* Is it **FAT** (FAT16 or FAT32)?
    * If the Error Code is indicating problems with the INIT, VOLUME or Track not being successful. It is recommend to use SdFat Example Library's QuickStart.ino as to see if it can access the card. Additionaly, SdInfo.ino may indicate if it can mount the card. Which may then need to formatted in FAT16 or FAT32. Where SdFormatter.ino can do this for you.
* Are the needed files on the **root**?
    * Remember to put patch and audio track files on the SdCard after formatting.
    * Are the filenames 8.3 format? See below warning.
* `"Error code: 1 when b trying to play track"`
    * See the above [Limitations.][26] about Non-Blocking.
    * Remember to check your audio cables and volume.
* `"Warning: patch file not found, skipping."`
* Why do I only **hear** 1 second of music, or less?
    * This symptom is typical of the interrupt not triggering the [SFEMP3Shield::refill()][31]. I bet repeatidly sendnig a track number will advance the play about one second at a time, then stop.
    * What board is it? Check Hardware [Limitations.][26] about Interrupts.
    * Are you trying the SFE provided [test files][32] ? Or some homemade mp3 files? The SFE test files are nice as they are Immediately LOUD.
    * Interrupt problems may cause mp3 files that have a quiet lead in (or ramp up of volume) to be falsely diagnosed as not playing at all. Where the first 1 second may not be loud enough to be heard.
* `Free RAM = 1090 Should be a base line of 1094`
    * As a courtesy and good practice it prints out the available remaining RAM, not statically allocated. And the actual available amount may depend on specific processor, IDE version, libraries and or other factors. 

Note
: This library makes extensive use of SdFat Library as to retrieve the stream of audio data from the SdCard. Notably this is where most failures occur. Where some SdCard types and manufacturers are not supported by SdFat. Though SdFat Lib is at this time, supporting most known cards.

Warning
: SdFatLib only supports 8.3 filenames. Long file names will not work. Use the `'d'` menu command to display directory contents of the SdCard. `"longfilename.mp3"` will be converted to `"longfi~1.mp3"` . Where one can not predict the value of the 1. The DOS command of `"dir c /x"` will list a cross reference, so that you know exactly, what is what.

Error Codes typically are returned from this Library's object's in place of Serial.print messages. As to both save Flash space and Serial devices may not always be present. Where it becomes the responsibility of the calling sketch of the library's object to appropiately react or display corresponding messages.

##  begin function:

The following error codes return from the [SFEMP3Shield::begin()][28] member function.

    0 OK
    1 *Failure of SdFat to initialize physical contact with the SdCard
    2 *Failure of SdFat to start the SdCard's volume
    3 *Failure of SdFat to mount the root directory on the volume of the SdCard
    4 Other than default values were found in the SCI_MODE register.
    5 SCI_CLOCKF did not read back and verify the configured value.
    6 Patch was not loaded successfully. This may result in playTrack errors


##  Playing functions:

The following error codes return from the [SFEMP3Shield::playTrack()][34] or [SFEMP3Shield::playMP3()][35] member functions.

    0 OK
    1 Already playing track
    2 File not found
    3 indicates that the VSdsp is in reset.

The code has been written with plenty of appropiate comments, describing key components, features and reasonings.