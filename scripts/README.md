# Rainbomizer Support Scripts

These scripts are used for generating data files for use with Rainbomizer. Use these to update Rainbomizer for a new update for GTA V. 

### Voice Line Randomizer scripts

These scripts generate the voice line randomizer data (VoiceLines.txt). To run these, you'll need the following files

- Exported game script files (.ysc.full - Use OpenIV)
- Exported game text files (.oxt - use OpenIV)
- Exported game speech audio metadata files (.dat4.rel)

To generate the data file, you then need to run the following scripts

- First run `speech-extractor.py <path to speech metadata files>`. This will generate two files VoiceSpeechVoices.txt and VoiceSpeechObjects.txt. Move them to the `voice-line-gen.py` folder.
- Then run `voice-line-gen.py <path to script files> <path to text files>`. This will generate the final VoiceLines.txt file. 

*Note:* To update the VoiceLines.txt, you only need the updated ysc.full, oxt and dat4.rel files.
