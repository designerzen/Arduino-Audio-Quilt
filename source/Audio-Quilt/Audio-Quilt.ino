
/*******************************************************************************

Audio Quilt - A Sample Bank Player

http://github.com/madsci1016/Sparkfun-MP3-Player-Shield-Arduino-Library/blob/master/SFEMP3Shield/Examples/FilePlayer/FilePlayer.ino
http://mpflaga.github.io/Sparkfun-MP3-Player-Shield-Arduino-Library/
http://mpflaga.github.io/Sparkfun-MP3-Player-Shield-Arduino-Library/class_s_f_e_m_p3_shield.html#aa0f78c569478259a1d8a7ed96a4c4167
https://github.com/madsci1016/Sparkfun-MP3-Player-Shield-Arduino-Library/blob/master/SFEMP3Shield/Examples/MP3Shield_Library_Demo/MP3Shield_Library_Demo.ino
 ------------------------------

 Bare Conductive code written by Stefan Dzisiewski-Smith and Peter Krige.

 This work is licensed under a Creative Commons Attribution-ShareAlike 3.0
 Unported License (CC BY-SA 3.0) http://creativecommons.org/licenses/by-sa/3.0/

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.

*******************************************************************************/

// compiler error handling
#include "Compiler_Errors.h"

// touch includes
#include <MPR121.h>
#include <Wire.h>
#define MPR121_ADDR 0x5C
#define MPR121_INT 4

// SD Card includes
#include <SPI.h>
#include <SdFat.h>
#include <SdFatUtil.h>

// mp3 includes
#include <SFEMP3Shield.h>

// touch behaviour definitions
#define PIN_FIRST 0
#define PIN_LAST 11

// define LED_BUILTIN for older versions of Arduino
#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif


// SETTINGS :
// 12 is the MAX length of a file name
// %d is the dynamic track number
//char baseFileName[16] = "TRACK_Z%d.MP3";  // TRACK_A1.MP3
char baseFileName[12] = "Z%d.MP3";          // A1.MP3
// char baseFileName[16] = "Z%03i.MP3";     // A001.MP3

uint8_t bankPosition = 0;   // 6

// We have 12 Banks a-l :
// EDIT : Names of the sample banks
// NB. These MUST be UPPERCASE and consist of a SINGLE character
char bankNames[12] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L' };

// EDIT : Quantity of samples in each bank
// NB. This process is handles automatically if a file cannot be played
// You can set the number higher if you plan on having many more samples
int bankLimits[12]  = { 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9 };

// EDIT : Do we want a repeated press of the electrode to stop the previous sound?
boolean stopOnRepeat = false;

// don't mess with these!
// These are the individual index of the sample currently selected for each bank
int bankCounter[12] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

// Global Variables :
// sd card instantiation
SdFat sd;
SdFile file;

// mp3 variable
SFEMP3Shield MP3Player;
// unsigned 8-bit integer (like a byte but just for numbers 0-9)
uint8_t result;

// Current playing sample index
int specifiedSample = -1;
// As we are monophonic, we can record the last key pressed
int lastElectrodePressed = 0;

////////////////////////////////////////////////////////////////////////////////////////
// Automatically gets called once at the beginning of the application
////////////////////////////////////////////////////////////////////////////////////////
void setup() 
{
    Serial.begin(57600);

    pinMode(12, OUTPUT);

    // uncomment when using the serial monitor
    // makes the arduino wait here until console is available
    //while (!Serial) ; {}
    
    // Check SD Card
    if (!sd.begin(SD_SEL, SPI_HALF_SPEED)) sd.initErrorHalt();
    
    // Initialise Touch Pads
    if (!MPR121.begin(MPR121_ADDR)) Serial.println("error setting up MPR121");
    MPR121.setInterruptPin(MPR121_INT);

    // Touch Sensitivity Settings
    MPR121.setTouchThreshold(40);
    MPR121.setReleaseThreshold(20);

    // Initialise MP3 Player
    // Check to see if MP3 Player correctly loaded
    result = MP3Player.begin();
    
    // This entire section can be removed to save memory!
    // ----------------------------- 8< ----------------------
    // Explain Result as Success / Errors in plain english
    switch( result  )
    {
        // Success!
        case 0:
            // Print out some nice information to the console
            Serial.println(F("Gawain & Zen Industries === ") );
            Serial.println(F("Audio Sample Quilt") );    
            Serial.print(F("F_CPU = "));
            Serial.println(F_CPU);
            Serial.print(F("Free RAM = ")); // available in Version 1.0 F() bases the string to into Flash, to use less SRAM.
            Serial.print(FreeRam(), DEC);  // FreeRam() is provided by SdFatUtil.h
            Serial.println(F(" Should be a base line of 1028, on ATmega328 when using INTx"));

            break;

        // Failures...
        // Specific Error Messgaes
        // 1 *Failure of SdFat to initialize physical contact with the SdCard
        case 1:
            Serial.println(F("Failure of SdFat to initialize physical contact with the SdCard"));              // can be removed for space, if needed.
            break;

        // 2 *Failure of SdFat to start the SdCard's volume
        case 2:
            Serial.println(F("Failure of SdFat to start the SdCard's volume"));    // can be removed for space, if needed.
            break;

        // 3 *Failure of SdFat to mount the root directory on the volume of the SdCard
        case 3:
            Serial.println(F("Failure of SdFat to mount the root directory on the volume of the SdCard"));    // can be removed for space, if needed.
            break;

        // 4 Other than default values were found in the SCI_MODE register.
        case 4:    
            Serial.println(F("Other than default values were found in the SCI_MODE register."));    // can be removed for space, if needed.
            break;

        // 5 SCI_CLOCKF did not read back and verify the configured value.
        case 5:    
            Serial.println(F("SCI_CLOCKF did not read back and verify the configured value."));    // can be removed for space, if needed.
            break;

        // 6 Patch was not loaded successfully. This may result in playTrack errors
        case 6:
            Serial.println(F("Patch was not loaded successfully. This may result in playTrack errors"));              // can be removed for space, if needed.
            Serial.println(F("Warning: patch file not found, skipping."));              // can be removed for space, if needed.
            Serial.println(F("Use the \"d\" command to verify SdCard can be read"));    // can be removed for space, if needed.
            break;

        // Generic Error Message
        default:
            Serial.print(F("Error code: "));
            Serial.print(result);
            Serial.println(F(" when trying to start MP3 player"));
    }
    // ----------------------------- >8 ----------------------
 
    // We want these to happen even if we have encountered errors
    MP3Player.setVolume(10, 10);
        
    // Check File Limits from filenames available
    determineLimits();

    // Console help
    // if (Serial) showHelp();
}

////////////////////////////////////////////////////////////////////////////////////////
// METHOD : Prints the contents of help.txt to the console to save memory on credits
////////////////////////////////////////////////////////////////////////////////////////
void showHelp()
{
    char line[25];
    int n;
    // open test file
    SdFile file("HELP.TXT", O_READ);

    // check for open error
    if (file.isOpen())
    {
        // File help.txt exists
        // read lines from the file
        while ((n = file.fgets(line, sizeof(line))) > 0)
        {
            if (line[n - 1] == '\n')
            {
                // remove '\n'
                line[n-1] = 0;
                // replace next line with LCD call to display line
                Serial.println(line);
            } else {
                // no '\n' - line too long or missing '\n' at EOF
                // handle error
            }
        }
    }else{
        // help file does not exist on the SD Card
    }
}

////////////////////////////////////////////////////////////////////////////////////////
// METHOD : Handy way of showing what files are on the sd card
////////////////////////////////////////////////////////////////////////////////////////
void listFiles()
{
     // read out all files in dir to console for debugging...
    Serial.println(F("List files in dir : "));
    sd.ls();
}

////////////////////////////////////////////////////////////////////////////////////////
// METHOD  : Count how many sequential samples there are on the SD Card in "bank"
// RETURNS : an Integer
// SUMMARY : Get the NUMBER of tracks mp3s for the specified Sample Bank
////////////////////////////////////////////////////////////////////////////////////////
int getSampleQuantityInBank( int bank )
{
    int bankLimit = bankLimits[ bank ];
    int track = 1;
    
    //Serial.print("bankLimit :");
    //Serial.print(bank);
    //Serial.print("/");
    //Serial.println(bankLimit);
    
    // returns the amount of wav files following the naming convention
    // eg. if you want to see how many samples there are in bank 2...
    // int bank2Limit = getSampleQuantityInBank( 2 );
    // Now loop through our file names...
    for (track; track < bankLimit; track++)
    {
        // Create the expected filenames 
        char sampleFileName[13];
        // overwrite the 6th character
        baseFileName[bankPosition] = bankNames[bank];
        sprintf( sampleFileName, baseFileName, track);

        // check to see if this file exists on the drive...
        if ( sd.exists( sampleFileName ) )
        {
            /*
            Serial.print("Found ");
            Serial.print(track);
            Serial.print("/");
            Serial.print(bankLimit);
            Serial.print(" called ");
            Serial.print(sampleFileName);
            Serial.println( " exists." );
            
            */
        }else{

            /*
            // Limit Reached!
            Serial.print("Looking for ");
            Serial.print(track);
            Serial.print("/");
            Serial.print(bankLimit);
            Serial.print(" called ");
            Serial.print(sampleFileName);
            Serial.println( " FAIL" );
             */
            // minus one for the offset (no track 0) and one for the failure
            return track-1;
        }
    }
    // minus one for the offset (no track 0)
    return track-1;
}

////////////////////////////////////////////////////////////////////////////////////////
// METHOD  :
// Here we loop through each bank
// And each file on the SD card
// Attempting to determine how many sequentially numbered tracks there are 
////////////////////////////////////////////////////////////////////////////////////////
void determineLimits()
{
     // read out all files in dir to console for debugging...
    // Serial.println(F("Determining Quantity of files in each bank"));
    
    // Loop through Banks...
    for (int bank = 0; bank <= PIN_LAST; bank++)
    {
        // now we cam determine the quantity,
        // we can overwrite the built in limit
        bankLimits[ bank ] = getSampleQuantityInBank( bank );

        Serial.print(F("Bank ")); 
        Serial.print( bankNames[bank] ); 
        Serial.print(F(" contains ")); 
        Serial.print( bankLimits[ bank ] );
        Serial.println(F(" samples"));
    }
    
    Serial.println(F("----------------------"));
}


////////////////////////////////////////////////////////////////////////////////////////
// METHOD  : this method takes an electrode number (0 to 11)
// and returns a number based on an incremented index
// from 1 to sample limit for that bank
//
// RETURNS : an Integer
// SUMMARY : Get the NUMBER of the subsequent track from the Sample Bank
////////////////////////////////////////////////////////////////////////////////////////
int getSampleIndexFromBank( int bank )
{
    //Serial.print("Fetching index from electrode " );
    //Serial.println( electrode );
    
    // minus 1 as we are using 1+ rather than 0
    if ( bankCounter[bank] + 1 >= bankLimits[bank] )
    {
        // it has! so reset it to zero
        //Serial.print( "Resetting bank " );
        //Serial.print( bank );
        //Serial.print( " to " );
        //Serial.println( bankCounter[bank] );
        bankCounter[bank] = 0;
    } else {
        bankCounter[bank]++;
    }
    
    // now let us offset by the quantity in each limit...
    //return bankCounter[bank] + (bank*10);
    return bankCounter[bank];
}


////////////////////////////////////////////////////////////////////////////////////////
// METHOD  : Play the next sample track from the specified bank of samples
// NB. playTrack( int ) only works for 0-9 so in order to access more banks of sounds
// we are going to have to use a different method to create the filenames
// RETURNS : Playback success - True or False
////////////////////////////////////////////////////////////////////////////////////////
boolean playNextTrackInBank( int bank )
{
    // * CHOICE :
    // Do we want either :

    // A. All of the pins to use zero based file names such as 1, 10, 20, 30
    //int sampleIndex = getSampleIndexFromBank( bank-PIN_FIRST );

    // B. The pin to specify the name so pin 4 uses samples 40, 41, 42, 43 etc
    //specifiedSample = getSampleIndexFromBank( bank );
    //char sampleFileName[13] = getSampleNameFromBank( bank );
   
    // NB. It doesn't matter what the files are called...
    // playTrack uses indexes of the alphabetically ordered file list
    
    //uint8_t playbackStatus = MP3Player.playTrack(specifiedSample);
    
    // The +1 at the end here makes the tracks go track_a1, track_a2 
    // ie. There is No track 0
    int sampleIndex = getSampleIndexFromBank( bank ) + 1;
    
    // now swap out the Z bank for our new bank character (a-L)
    // This is a neat method of changing the character at space 6
    // To any other character without having to specify a new variable
    // and thus saves us some precious memory
    baseFileName[bankPosition] = bankNames[bank];
    
    // convert it to a file name
    char sampleFileName[13];
    sprintf( sampleFileName, baseFileName, sampleIndex);

    // http://mpflaga.github.io/Sparkfun-MP3-Player-Shield-Arduino-Library/class_s_f_e_m_p3_shield.html#aa0f78c569478259a1d8a7ed96a4c4167
    uint8_t playbackStatus = MP3Player.playMP3( sampleFileName );
   
    // Now check playback status to see if we have achieved playback
    // playTrack() returns 0 on success 
    switch (playbackStatus)
    {
        // 0 OK
        case 0:
            Serial.print(F( "Starting " ));
            Serial.print( sampleIndex );
            Serial.print(F( "/" ));
            Serial.print( bankLimits[bank] );
            Serial.print(F( " Named \"" ));
            Serial.print( sampleFileName );
            Serial.print(F( "\" From Bank " ));
            Serial.println( bankNames[bank] );

            // save current smaple index
            specifiedSample = sampleIndex;

            return true;
            
        // 1 Already playing track
        case 1:
            Serial.print(F( "Already Playing " ));
            Serial.print( sampleIndex );
            Serial.print(F( "/" ));
            Serial.print( bankLimits[bank] );
            Serial.print(F( " Named \"" ));
            Serial.print( sampleFileName );
            Serial.print(F( "\" From Bank " ));
            Serial.println( bankNames[bank] );

            return false;
            
        default:
            // 2 File not found
            // 3 indicates that the VSdsp is in reset.
            // rats... no file in this position...
            // in an ideal world this will never be called
            Serial.print(F( "Couldn't open" ));
            Serial.print(F( " Sample " ));
            Serial.print( sampleIndex );
            Serial.print(F( " Named " ));
            Serial.print( sampleFileName );
            Serial.print(F( " From Bank " ));
            Serial.println( bankNames[bank] );

            return false;
    }
}


////////////////////////////////////////////////////////////////////////////////////////
// METHOD : Read any changes that might have occurred with the touch pads
// NB. Gets run on every cycle of the application via loop()
////////////////////////////////////////////////////////////////////////////////////////
void readTouchInputs()
{
    // Has the Touch Board been Touched?
    if (MPR121.touchStatusChanged() )
    {
        // Update our Touch Hardware
        MPR121.updateTouchData();

        // only make an action if we have one or fewer pins touched
        // (ignore multiple touches)
        if (MPR121.getNumTouches() <= 1)
        {
            // Loop through all our pins
            // To check which electrodes were pressed
            for (int i = PIN_FIRST; i <= PIN_LAST; i++)
            { 
                // If this pin was touched AND the sample bank is not empty...
                if ( MPR121.isNewTouch(i) && bankLimits[i] > 0 )
                {
                    // Pin i *is* being touched...
                    // Serial.print("Pin ");
                    // Serial.print(i);
                    // Serial.println(" was just touched");

                    // turn on LED
                    digitalWrite(LED_BUILTIN, HIGH);

                    /*       
                    // Debug if Electrode I interefered with
                    if ( i == PIN_LAST )
                    {
                        // last pin pressed... list files
                        Serial.println( "Limitting... " );
                        determineLimits();
                    }
                    
                    // Debug if Electrode I interefered with
                    if ( i == PIN_LAST-1 )
                    {
                        // second to last pin... list files
                        Serial.println( "Listing Files ... " );
                        listFiles();
                    }

                    */

                    // Control playback of music
                    if ( MP3Player.isPlaying() )
                    {
                        // Serial.println("Mp3 Player is playing");

                        // As we're already playing the requested track, stop it
                        MP3Player.stopTrack();

                        // Serial.print("Stopping track -> ");
                        // Serial.print(specifiedSample);

                        // if stopOnRepeat is true, 
                        // and a repeated electrode is pressed before the sound has finished,
                        // then do not play the next sound
                        if ( stopOnRepeat && lastElectrodePressed == i)
                        {
                            // so that means the user has pressed the same electrode twice!
                            Serial.print(F( "Repeat interaction From Electrode: " ));
                            Serial.println( lastElectrodePressed );

                        } else  {

                            // Ok, so the user has pressed a different electrode to last time...
                            // How do we want to proceed?

                            // Well, firstly, we know the electrode that was pressed is called i
                            boolean success = playNextTrackInBank( i );

                            // don't forget to update lastElectrodePressed - without it we don't
                            // have a history
                            lastElectrodePressed = i;

                            Serial.print(F( "Changing Sound Electrode:" ));
                            Serial.print(i );
                            Serial.print( F(" -> sample:" ));
                            Serial.print( specifiedSample );
                            Serial.print( F(" -> sucess:" ));
                            Serial.println( success );
                        }


                    // MP3 Player is *not* playing 
                    } else {

                        // So let us start it with a new sound
                        boolean success = playNextTrackInBank( i );

                        // don't forget to update lastElectrodePressed - without it we don't
                        // have a history
                        lastElectrodePressed = i;

                        Serial.print( "Starting Sound Electrode:" );
                        Serial.print(i );
                        Serial.print( F(" -> sample:" ));
                        Serial.print( specifiedSample );
                        Serial.print( F(" -> sucess:" ));
                        Serial.println( success );
                    }

                } else {

                    // If pin is NOT being accessed anynore...
                    // (but previously was)
                    if (MPR121.isNewRelease(i))
                    {
                        // A pin was just RELEASED...
                        // Useful event if for example, you want to stop the sound...
                        // uncomment If you want the sound to stop on release
                        // MP3Player.stopTrack();

                        // Serial.print("pin ");
                        // Serial.print(i);
                        // Serial.println(" is no longer being touched");

                        // turn off LED
                        digitalWrite(LED_BUILTIN, LOW);
                    }

                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////
// Automatically gets called after setup() and thereafter On Every Frame (Clock rate)
////////////////////////////////////////////////////////////////////////////////////////
void loop() 
{
    // TODO : Only call this method if result == 1
    // as this means that everything has initialised correctly
    // but also slows this loop down...
    // realistically, it won't affect the life of the SD Card
    // but adds weight to the CPU and so may not be for the best
    readTouchInputs();
}
