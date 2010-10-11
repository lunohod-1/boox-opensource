#ifndef SPEAK_LIB_H
#define SPEAK_LIB_H
/***************************************************************************
 *   Copyright (C) 2005 to 2007 by Jonathan Duddington                     *
 *   email: jonsd@users.sourceforge.net                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see:                                 *
 *               <http://www.gnu.org/licenses/>.                           *
 ***************************************************************************/


/*************************************************************/
/* This is the header file for the library version of espeak */
/*                                                           */
/*************************************************************/

#include <stdio.h>

#define ESPEAK_API_REVISION  3
/*
Revision 2
   Added parameter "options" to eSpeakInitialize()

Revision 3
   Added espeakWORDGAP to  espeak_PARAMETER

Revision 4
   Added flags parameter to espeak_CompileDictionary()

*/
         /********************/
         /*  Initialization  */
         /********************/


typedef enum {
  espeakEVENT_LIST_TERMINATED = 0, // Retrieval mode: terminates the event list.
  espeakEVENT_WORD = 1,            // Start of word
  espeakEVENT_SENTENCE,            // Start of sentence
  espeakEVENT_MARK,                // Mark
  espeakEVENT_PLAY,                // Audio element
  espeakEVENT_END,                 // End of sentence or clause
  espeakEVENT_MSG_TERMINATED,      // End of message
  espeakEVENT_PHONEME              // Phoneme, if enabled in espeak_Initialize()
} espeak_EVENT_TYPE;



typedef struct {
	espeak_EVENT_TYPE type;
	unsigned int unique_identifier; // message identifier (or 0 for key or character)
	int text_position;    // the number of characters from the start of the text
	int length;           // word length, in characters (for espeakEVENT_WORD)
	int audio_position;   // the time in mS within the generated speech output data
	int sample;           // sample id (internal use)
	void* user_data;      // pointer supplied by the calling program
	union {
		int number;        // used for WORD and SENTENCE events. For PHONEME events this is the phoneme mnemonic.
		const char *name;  // used for MARK and PLAY events.  UTF8 string
	} id;
} espeak_EVENT;
/* 
   When a message is supplied to espeak_synth, the request is buffered and espeak_synth returns. When the message is really processed, the callback function will be repetedly called.


   In RETRIEVAL mode, the callback function supplies to the calling program the audio data and an event list terminated by 0 (LIST_TERMINATED).

   In PLAYBACK mode, the callback function is called as soon as an event happens.

   For example suppose that the following message is supplied to espeak_Synth: 
   "hello, hello."


   * Once processed in RETRIEVAL mode, it could lead to 3 calls of the callback function :

   ** Block 1:
   <audio data> + 
   List of events: SENTENCE + WORD + LIST_TERMINATED
 
   ** Block 2:
   <audio data> +
   List of events: WORD + END + LIST_TERMINATED

   ** Block 3:
   no audio data
   List of events: MSG_TERMINATED + LIST_TERMINATED


   * Once processed in PLAYBACK mode, it could lead to 5 calls of the callback function:

   ** SENTENCE
   ** WORD (call when the sounds are actually played)
   ** WORD
   ** END (call when the end of sentence is actually played.)
   ** MSG_TERMINATED


   The MSG_TERMINATED event is the last event. It can inform the calling program to clear the user data related to the message.
   So if the synthesis must be stopped, the callback function is called for each pending message with the MSG_TERMINATED event.

   A MARK event indicates a <mark> element in the text.
   A PLAY event indicates an <audio> element in the text, for which the calling program should play the named sound file.
*/



typedef enum {
	POS_CHARACTER = 1,
	POS_WORD,
	POS_SENTENCE
} espeak_POSITION_TYPE;


typedef enum {
	/* PLAYBACK mode: plays the audio data, supplies events to the calling program*/
	AUDIO_OUTPUT_PLAYBACK, 

	/* RETRIEVAL mode: supplies audio data and events to the calling program */
	AUDIO_OUTPUT_RETRIEVAL,
 
	/* SYNCHRONOUS mode: as RETRIEVAL but doesn't return until synthesis is completed */
	AUDIO_OUTPUT_SYNCHRONOUS,

	/* Synchronous playback */
	AUDIO_OUTPUT_SYNCH_PLAYBACK

} espeak_AUDIO_OUTPUT;


typedef enum {
	EE_OK=0,
	EE_INTERNAL_ERROR=-1,
	EE_BUFFER_FULL=1,
	EE_NOT_FOUND=2
} espeak_ERROR;


#ifdef __cplusplus
extern "C"
#endif
int espeak_Initialize(espeak_AUDIO_OUTPUT output, int buflength, const char *path, int options);
/* Must be called before any synthesis functions are called.
   output: the audio data can either be played by eSpeak or passed back by the SynthCallback function.

   buflength:  The length in mS of sound buffers passed to the SynthCallback function.

   path: The directory which contains the espeak-data directory, or NULL for the default location.

   options: bit 0: 1=allow espeakEVENT_PHONEME events.


   Returns: sample rate in Hz, or -1 (EE_INTERNAL_ERROR).
*/

typedef int (t_espeak_callback)(short*, int, espeak_EVENT*);

#ifdef __cplusplus
extern "C"
#endif
void espeak_SetSynthCallback(t_espeak_callback* SynthCallback);
/* Must be called before any synthesis functions are called.
   This specifies a function in the calling program which is called when a buffer of
   speech sound data has been produced. 


   The callback function is of the form:

int SynthCallback(short *wav, int numsamples, espeak_EVENT *events);

   wav:  is the speech sound data which has been produced.
      NULL indicates that the synthesis has been completed.

   numsamples: is the number of entries in wav.  This number may vary, may be less than
      the value implied by the buflength parameter given in espeak_Initialize, and may
      sometimes be zero (which does NOT indicate end of synthesis).

   events: an array of espeak_EVENT items which indicate word and sentence events, and
      also the occurance if <mark> and <audio> elements within the text.  The list of
      events is terminated by an event of type = 0.


   Callback returns: 0=continue synthesis,  1=abort synthesis.
*/

#ifdef __cplusplus
extern "C"
#endif
void espeak_SetUriCallback(int (*UriCallback)(int, const char*, const char*));
/* This function may be called before synthesis functions are used, in order to deal with
   <audio> tags.  It specifies a callback function which is called when an <audio> element is
   encountered and allows the calling program to indicate whether the sound file which
   is specified in the <audio> element is available and is to be played.

   The callback function is of the form:

int UriCallback(int type, const char *uri, const char *base);

   type:  type of callback event.  Currently only 1= <audio> element

   uri:   the "src" attribute from the <audio> element

   base:  the "xml:base" attribute (if any) from the <speak> element

   Return: 1=don't play the sound, but speak the text alternative.
           0=place a PLAY event in the event list at the point where the <audio> element
             occurs.  The calling program can then play the sound at that point.
*/


         /********************/
         /*    Synthesis     */
         /********************/


#define espeakCHARS_AUTO   0
#define espeakCHARS_UTF8   1
#define espeakCHARS_8BIT   2
#define espeakCHARS_WCHAR  3

#define espeakSSML        0x10
#define espeakPHONEMES    0x100
#define espeakENDPAUSE    0x1000
#define espeakKEEP_NAMEDATA 0x2000

#ifdef __cplusplus
extern "C"
#endif
espeak_ERROR espeak_Synth(const void *text,
	size_t size,
	unsigned int position,
	espeak_POSITION_TYPE position_type,
	unsigned int end_position,
	unsigned int flags,
	unsigned int* unique_identifier,
	void* user_data);
/* Synthesize speech for the specified text.  The speech sound data is passed to the calling
   program in buffers by means of the callback function specified by espeak_SetSynthCallback(). The command is asynchronous: it is internally buffered and returns as soon as possible. If espeak_Initialize was previously called with AUDIO_OUTPUT_PLAYBACK as argument, the sound data are played by eSpeak.

   text: The text to be spoken, terminated by a zero character. It may be either 8-bit characters,
      wide characters (wchar_t), or UTF8 encoding.  Which of these is determined by the "flags"
      parameter.

   size: Equal to (or greatrer than) the size of the text data, in bytes.  This is used in order
      to allocate internal storage space for the text.  This value is not used for
      AUDIO_OUTPUT_SYNCHRONOUS mode.

   position:  The position in the text where speaking starts. Zero indicates speak from the
      start of the text.

   position_type:  Determines whether "position" is a number of characters, words, or sentences.
      Values: 

   end_position:  If set, this gives a character position at which speaking will stop.  A value
      of zero indicates no end position.

   flags:  These may be OR'd together:
      Type of character codes, one of:
         espeakCHARS_UTF8     UTF8 encoding
         espeakCHARS_8BIT     The 8 bit ISO-8859 character set for the particular language.
         espeakCHARS_AUTO     8 bit or UTF8  (this is the default)
         espeakCHARS_WCHAR    Wide characters (wchar_t)

      espeakSSML   Elements within < > are treated as SSML elements, or if not recognised are ignored.

      espeakPHONEMES  Text within [[ ]] is treated as phonemes codes (in espeak's Hirshenbaum encoding).

      espeakENDPAUSE  If set then a sentence pause is added at the end of the text.  If not set then
         this pause is suppressed.

   unique_identifier: message identifier; helpful for identifying later 
     data supplied to the callback.

   user_data: pointer which will be passed to the callback function.

   Return: EE_OK: operation achieved 
           EE_BUFFER_FULL: the command can not be buffered; 
             you may try after a while to call the function again.
	   EE_INTERNAL_ERROR.
*/

#ifdef __cplusplus
extern "C"
#endif
espeak_ERROR espeak_Synth_Mark(const void *text,
	size_t size,
	const char *index_mark,
	unsigned int end_position,
	unsigned int flags,
	unsigned int* unique_identifier,
	void* user_data);
/* Synthesize speech for the specified text.  Similar to espeak_Synth() but the start position is
   specified by the name of a <mark> element in the text.

   index_mark:  The "name" attribute of a <mark> element within the text which specified the
      point at which synthesis starts.  UTF8 string.

   For the other parameters, see espeak_Synth()

   Return: EE_OK: operation achieved 
           EE_BUFFER_FULL: the command can not be buffered; 
             you may try after a while to call the function again.
	   EE_INTERNAL_ERROR.
*/

#ifdef __cplusplus
extern "C"
#endif
espeak_ERROR espeak_Key(const char *key_name);
/* Speak the name of a keyboard key.
   If key_name is a single character, it speaks the name of the character.
   Otherwise, it speaks key_name as a text string.

   Return: EE_OK: operation achieved 
           EE_BUFFER_FULL: the command can not be buffered; 
             you may try after a while to call the function again.
	   EE_INTERNAL_ERROR.
*/

#ifdef __cplusplus
extern "C"
#endif
espeak_ERROR espeak_Char(wchar_t character);
/* Speak the name of the given character 

   Return: EE_OK: operation achieved 
           EE_BUFFER_FULL: the command can not be buffered; 
             you may try after a while to call the function again.
	   EE_INTERNAL_ERROR.
*/




         /***********************/
         /*  Speech Parameters  */
         /***********************/

typedef enum {
  espeakSILENCE=0, /* internal use */
  espeakRATE=1,
  espeakVOLUME=2,
  espeakPITCH=3,
  espeakRANGE=4,
  espeakPUNCTUATION=5,
  espeakCAPITALS=6,
  espeakWORDGAP=7,
  espeakOPTIONS=8,   // reserved for misc. options.  not yet used
  espeakINTONATION=9,

  espeakRESERVED1=10,
  espeakRESERVED2=11,
  espeakEMPHASIS,   /* internal use */
  espeakLINELENGTH, /* internal use */
  espeakVOICETYPE,  // internal, 1=mbrola
  N_SPEECH_PARAM    /* last enum */
} espeak_PARAMETER;

typedef enum {
  espeakPUNCT_NONE=0,
  espeakPUNCT_ALL=1,
  espeakPUNCT_SOME=2
} espeak_PUNCT_TYPE;

#ifdef __cplusplus
extern "C"
#endif
espeak_ERROR espeak_SetParameter(espeak_PARAMETER parameter, int value, int relative);
/* Sets the value of the specified parameter.
   relative=0   Sets the absolute value of the parameter.
   relative=1   Sets a relative value of the parameter.

   parameter:
      espeakRATE:    speaking speed in word per minute.

      espeakVOLUME:  volume in range 0-100    0=silence

      espeakPITCH:   base pitch, range 0-100.  50=normal

      espeakRANGE:   pitch range, range 0-100. 0-monotone, 50=normal

      espeakPUNCTUATION:  which punctuation characters to announce:
         value in espeak_PUNCT_TYPE (none, all, some), 
         see espeak_GetParameter() to specify which characters are announced.

      espeakCAPITALS: announce capital letters by:
         0=none,
         1=sound icon,
         2=spelling,
         3 or higher, by raising pitch.  This values gives the amount in Hz by which the pitch
            of a word raised to indicate it has a capital letter.

      espeakWORDGAP:  pause between words, units of 10mS (at the default speed)

   Return: EE_OK: operation achieved 
           EE_BUFFER_FULL: the command can not be buffered; 
             you may try after a while to call the function again.
	   EE_INTERNAL_ERROR.
*/

#ifdef __cplusplus
extern "C"
#endif
int espeak_GetParameter(espeak_PARAMETER parameter, int current);
/* current=0  Returns the default value of the specified parameter.
   current=1  Returns the current value of the specified parameter, as set by SetParameter()
*/

#ifdef __cplusplus
extern "C"
#endif
espeak_ERROR espeak_SetPunctuationList(const wchar_t *punctlist);
/* Specified a list of punctuation characters whose names are to be spoken when the
   value of the Punctuation parameter is set to "some".

   punctlist:  A list of character codes, terminated by a zero character.

   Return: EE_OK: operation achieved 
           EE_BUFFER_FULL: the command can not be buffered; 
             you may try after a while to call the function again.
	   EE_INTERNAL_ERROR.
*/

#ifdef __cplusplus
extern "C"
#endif
void espeak_SetPhonemeTrace(int value, FILE *stream);
/* Controls the output of phoneme symbols for the text
   value=0  No phoneme output (default)
   value=1  Output the translated phoneme symbols for the text
   value=2  as (1), but also output a trace of how the translation was done (matching rules and list entries)

   stream   output stream for the phoneme symbols (and trace).  If stream=NULL then it uses stdout.
*/

#ifdef __cplusplus
extern "C"
#endif
void espeak_CompileDictionary(const char *path, FILE *log, int flags);
/* Compile pronunciation dictionary for a language which corresponds to the currently
   selected voice.  The required voice should be selected before calling this function.

   path:  The directory which contains the language's '_rules' and '_list' files.
          'path' should end with a path separator character ('/').
   log:   Stream for error reports and statistics information. If log=NULL then stderr will be used.

   flags:  Bit 0: include source line information for debug purposes (This is displayed with the
          -X command line option).
*/
         /***********************/
         /*   Voice Selection   */
         /***********************/


// voice table
typedef struct {
	const char *name;      // a given name for this voice. UTF8 string.
	const char *languages;       // list of pairs of (byte) priority + (string) language (and dialect qualifier)
	const char *identifier;      // the filename for this voice within espeak-data/voices
	unsigned char gender;  // 0=none 1=male, 2=female,
	unsigned char age;     // 0=not specified, or age in years
	unsigned char variant; // only used when passed as a parameter to espeak_SetVoiceByProperties
	unsigned char xx1;     // for internal use 
	int score;       // for internal use
	void *spare;     // for internal use
} espeak_VOICE;

/* Note: The espeak_VOICE structure is used for two purposes:
  1.  To return the details of the available voices.
  2.  As a parameter to  espeak_SetVoiceByProperties() in order to specify selection criteria.

   In (1), the "languages" field consists of a list of (UTF8) language names for which this voice
   may be used, each language name in the list is terminated by a zero byte and is also preceded by
   a single byte which gives a "priority" number.  The list of languages is terminated by an
   additional zero byte.

   A language name consists of a language code, optionally followed by one or more qualifier (dialect)
   names separated by hyphens (eg. "en-uk").  A voice might, for example, have languages "en-uk" and
   "en".  Even without "en" listed, voice would still be selected for the "en" language (because
   "en-uk" is related) but at a lower priority.

   The priority byte indicates how the voice is preferred for the language. A low number indicates a
   more preferred voice, a higher number indicates a less preferred voice.

   In (2), the "languages" field consists simply of a single (UTF8) language name, with no preceding
   priority byte.
*/

#ifdef __cplusplus
extern "C"
#endif
const espeak_VOICE **espeak_ListVoices(espeak_VOICE *voice_spec);
/* Reads the voice files from espeak-data/voices and creates an array of espeak_VOICE pointers.
   The list is terminated by a NULL pointer

   If voice_spec is NULL then all voices are listed.
   If voice spec is give, then only the voices which are compatible with the voice_spec
   are listed, and they are listed in preference order.
*/

#ifdef __cplusplus
extern "C"
#endif
espeak_ERROR espeak_SetVoiceByName(const char *name);
/* Searches for a voice with a matching "name" field.  Language is not considered.
   "name" is a UTF8 string.

   Return: EE_OK: operation achieved 
           EE_BUFFER_FULL: the command can not be buffered; 
             you may try after a while to call the function again.
	   EE_INTERNAL_ERROR.
*/

#ifdef __cplusplus
extern "C"
#endif
espeak_ERROR espeak_SetVoiceByProperties(espeak_VOICE *voice_spec);
/* An espeak_VOICE structure is used to pass criteria to select a voice.  Any of the following
   fields may be set:

   name     NULL, or a voice name

   languages  NULL, or a single language string (with optional dialect), eg. "en-uk", or "en"

   gender   0=not specified, 1=male, 2=female

   age      0=not specified, or an age in years

   variant  After a list of candidates is produced, scored and sorted, "variant" is used to index
            that list and choose a voice.
            variant=0 takes the top voice (i.e. best match). variant=1 takes the next voice, etc
*/

#ifdef __cplusplus
extern "C"
#endif
espeak_VOICE *espeak_GetCurrentVoice(void);
/* Returns the espeak_VOICE data for the currently selected voice.
   This is not affected by temporary voice changes caused by SSML elements such as <voice> and <s>
*/

#ifdef __cplusplus
extern "C"
#endif
espeak_ERROR espeak_Cancel(void);
/* Stop immediately synthesis and audio output of the current text. When this
   function returns, the audio output is fully stopped and the synthesizer is ready to
   synthesize a new message.

   Return: EE_OK: operation achieved 
	   EE_INTERNAL_ERROR.
*/


#ifdef __cplusplus
extern "C"
#endif
int espeak_IsPlaying(void);
/* Returns 1 if audio is played, 0 otherwise.
*/

#ifdef __cplusplus
extern "C"
#endif
espeak_ERROR espeak_Synchronize(void);
/* This function returns when all data have been spoken.
   Return: EE_OK: operation achieved 
	   EE_INTERNAL_ERROR.
*/

#ifdef __cplusplus
extern "C"
#endif
espeak_ERROR espeak_Terminate(void);
/* last function to be called.
   Return: EE_OK: operation achieved 
	   EE_INTERNAL_ERROR.
*/


#ifdef __cplusplus
extern "C"
#endif
const char *espeak_Info(void* ptr);
/* Returns the version number string.
   The parameter is for future use, and should be set to NULL
*/
#endif