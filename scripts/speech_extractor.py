import os
import sys
import struct

voices = open("VoiceSpeechVoices.txt", "w")
objects = open("VoiceSpeechObjects.txt", "w")

def process_dat4_rel_file (filename):
    with open(filename, "rb") as f:
        data = f.read()
        dataLen = struct.unpack_from("<I", data, 4)[0]
        nameTableLen = struct.unpack_from("<I", data, 8 + dataLen)[0]

        objectMapOffset = 8 + dataLen + nameTableLen + 4
        objectMapLen = struct.unpack_from(
            "<I", data, objectMapOffset)[0]

        for i in range(objectMapOffset + 4,
                       objectMapOffset + 4 + objectMapLen * 12,
                       12):
            name, offset, length = struct.unpack_from("<III", data, i)
            if length != 8:
                print("0x%x" % name, file = voices if length == 1 else objects)
            
            pass
    pass

for root, subdirs, files in os.walk(sys.argv[1]):
    for filename in files:
        if filename.endswith('.dat4.rel'):
            process_dat4_rel_file(os.path.join(root, filename))
