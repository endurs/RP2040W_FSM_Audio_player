# RP2040W_FSM_Audio_player
WAV audio player using a finite state machine (FSM) to switch tracks using switch inputs.

The FSM configuration and audio files are stored on and SD-card.

- the FSM config is set in a .json file, through this file you can select how many states, what state(s) it can transition to, what condition(s) has to be fulfilled for it to move to another state, and finally what music folder to play sound from. Detailed instructions on .json config can be found in "UHH COMING SOON".md.

- Audio files the music player is configured to play are 16 bit (PCM) .WAV files (mono reccomended). Any audio (or audio from a video file) can easily be converted to this format using Audacity (https://www.audacityteam.org/). Detailed instructions on how to make sure the file is in correct format can be found in "UHH COMING SOON2".md.

- Currently only on/off switch inputs are supported for conditions to change states.

- Quickstart guide can be found in "UHHHHHH.md"

