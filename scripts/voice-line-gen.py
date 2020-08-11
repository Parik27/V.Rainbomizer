import struct
import os
import sys
import re

label_hashes = {}
voice_lines = {}
voice_files = []
voice_voices = []

current_voice_banks = {}

conflicts = open("VoiceConflicts.txt", "w")
warnings = open("VoiceWarnings.txt", "w")

current_file = ""

def calculate_num_blocks (size):
    return size // 0x4000 + 1

def calculate_block_size (index, size):
    if index == calculate_num_blocks(size) - 1:
        return size % 0x4000
    return 0x4000

def atStringHash(ba):
    hash = 0
    for i in range(len(ba)):
        hash += ba[i]
        hash &= 0xFFFFFFFF
        hash += hash << 10
        hash &= 0xFFFFFFFF
        hash ^= hash >> 6
        hash &= 0xFFFFFFFF
    hash += hash << 3
    hash &= 0xFFFFFFFF
    hash ^= hash >> 11
    hash &= 0xFFFFFFFF
    hash += hash << 15
    hash &= 0xFFFFFFFF
    return hash

def _process_voice_line (lineHash, gxt, label):
    context = label_hashes[lineHash][1]
    context_hash = atStringHash(bytes(context.lower(), "ascii"))

    if context.startswith("SFX_") or context in voice_lines:
        return
    
    bank = None
    for speaker in current_voice_banks:                
        if atStringHash(bytes("%x" % (context_hash ^ speaker), "ascii")) in voice_files:
            
            if bank is not None:
                print ("Context conflict detected: %s (%s) - %s and %s" %
                       (context, label, current_voice_banks[speaker], bank),
                       file = conflicts)
                    
            bank = current_voice_banks[speaker]

    if bank is None:
        print ("Failed to find bank for context: %s %s" % (context, label),
               file = warnings)
        return
    
    voice_lines[context] = [gxt, bank, label]

def process_voice_line (voiceLine, gxt):

    # Single context voice lines
    scvl_hash = atStringHash(bytes((voiceLine + "A").lower(), "ascii"))
    if scvl_hash in label_hashes:
        _process_voice_line(scvl_hash, gxt, voiceLine)

    else:
        for i in range(1, 80):
            voice_line_label = "%s_%dA" % (voiceLine, i)
            mcvl_hash = atStringHash(bytes(voice_line_label.lower(), "ascii"))

            if mcvl_hash not in label_hashes:
                if i == 1:
                    print("Warning: Invalid voice line detected: %s" %
                          (voiceLine), file = warnings)
                break

            _process_voice_line(mcvl_hash, gxt, voice_line_label[:-1])
        

def process_str_block_contexts (strings):
    for i, string in enumerate(strings):
        print ("%s: (%05d/%05d) (%05d/%05d)" %
               (current_file, len(strings), len(strings), i, len(strings)), end = '\r')
        
        label_hash = atStringHash(bytes((string + "SL").lower(), "ascii"))
        if label_hash in label_hashes:
            process_voice_line(string, label_hashes[label_hash][0])
            #voice_lines[string] = label_hashes[label_hash][0]

def process_str_block_voices (strings):
    for i, string in enumerate(strings):
        print ("%s: (%05d/%05d) (%05d/%05d)" %
               (current_file, i, len(strings), 0, len(strings)), end = '\r')
        label_hash = atStringHash(bytes(string.lower(), "ascii"))
        if label_hash in voice_voices:
            current_voice_banks[label_hash] = string

def process_str_block (block):
    string = ""
    strings = []
    for char in block:
        if char == 0:
            if len(string) != 0:
                strings.append (string)
                    
            string = ""

        if char < 127 and char > 31:
            string += chr(char)

    return strings

def process_scr_file (filepath):
    with open(filepath, 'rb') as scr_file:
        scr_data = scr_file.read()
        strings_offset, strings_size = struct.unpack_from("<QI", scr_data, 0x68)

        strings_offset = strings_offset & 0xFFFFFF
        num_strings_blocks = calculate_num_blocks(strings_size)

        strs = []

        global current_voice_banks
        current_voice_banks = {}
        
        for i in range(num_strings_blocks):
            block_offset = struct.unpack_from(
                "<Q", scr_data, strings_offset + 8 * i)[0]
            block_offset = block_offset & 0xFFFFFF
            
            block_size = calculate_block_size(i, strings_size)

            strs += process_str_block(
                scr_data[block_offset:block_offset+block_size])

        process_str_block_voices(strs)
        process_str_block_contexts(strs)
    pass

def process_voices_file (filepath):
    with open(filepath, 'r') as voices_file:
        for line in voices_file.readlines():
            voice_voices.append(int(line, base=0))

def process_speech_file (filepath):
    with open(filepath, 'r') as speech_file:
        for line in speech_file.readlines():
            voice_files.append(int(line, base=0))
            
def process_oxt_file (filepath):
    with open(filepath, 'r') as oxt_file:
        oxt_data = oxt_file.read()
        hashes = re.findall("	0x(.+?) = (.*)", oxt_data)
        for label_hash, data in hashes:
            label_hashes[int(label_hash, base=16)] = (os.path.splitext(
                os.path.basename(filepath))[0], data)

process_voices_file("VoiceSpeechVoices.txt")
process_speech_file("VoiceSpeechObjects.txt")
            
for root, subdirs, files in os.walk(sys.argv[2]):
    for filename in files:
        if filename.endswith('oxt'):
            process_oxt_file (os.path.join(root, filename))

for root, subdirs, files in os.walk(sys.argv[1]):
    for i, filename in enumerate(files):
        if filename.endswith('ysc.full'):
            current_file = filename
            print("")
            process_scr_file(os.path.join(root, filename))

with open("VoiceLines.txt", "w") as vls:
    for line in voice_lines:
        print(line, *voice_lines[line], file = vls)

warnings.close()
conflicts.close()
