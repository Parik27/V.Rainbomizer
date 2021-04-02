import struct
import sys

OP_NAMES = {
    0: "NOP",
    1: "IADD",
    2: "ISUB",
    3: "IMUL",
    4: "IDIV",
    5: "IMOD",
    6: "INOT",
    7: "INEG",
    8: "IEQ",
    9: "INE",
    10: "IGT",
    11: "IGE",
    12: "ILT",
    13: "ILE",
    14: "FADD",
    15: "FSUB",
    16: "FMUL",
    17: "FDIV",
    18: "FMOD",
    19: "FNEG",
    20: "FEQ",
    21: "FNE",
    22: "FGT",
    23: "FGE",
    24: "FLT",
    25: "FLE",
    26: "VADD",
    27: "VSUB",
    28: "VMUL",
    29: "VDIV",
    30: "VNEG",
    31: "IAND",
    32: "IOR",
    33: "IXOR",
    34: "I2F",
    35: "F2I",
    36: "F2V",
    37: "PUSH_CONST_U8",
    38: "PUSH_CONST_U8_U8",
    39: "PUSH_CONST_U8_U8_U8",
    40: "PUSH_CONST_U32",
    41: "PUSH_CONST_F",
    42: "DUP",
    43: "DROP",
    44: "NATIVE",
    45: "ENTER",
    46: "LEAVE",
    47: "LOAD",
    48: "STORE",
    49: "STORE_REV",
    50: "LOAD_N",
    51: "STORE_N",
    52: "ARRAY_U8",
    53: "ARRAY_U8_LOAD",
    54: "ARRAY_U8_STORE",
    55: "LOCAL_U8",
    56: "LOCAL_U8_LOAD",
    57: "LOCAL_U8_STORE",
    58: "STATIC_U8",
    59: "STATIC_U8_LOAD",
    60: "STATIC_U8_STORE",
    61: "IADD_U8",
    62: "IMUL_U8",
    63: "IOFFSET",
    64: "IOFFSET_U8",
    65: "IOFFSET_U8_LOAD",
    66: "IOFFSET_U8_STORE",
    67: "PUSH_CONST_S16",
    68: "IADD_S16",
    69: "IMUL_S16",
    70: "IOFFSET_S16",
    71: "IOFFSET_S16_LOAD",
    72: "IOFFSET_S16_STORE",
    73: "ARRAY_U16",
    74: "ARRAY_U16_LOAD",
    75: "ARRAY_U16_STORE",
    76: "LOCAL_U16",
    77: "LOCAL_U16_LOAD",
    78: "LOCAL_U16_STORE",
    79: "STATIC_U16",
    80: "STATIC_U16_LOAD",
    81: "STATIC_U16_STORE",
    82: "GLOBAL_U16",
    83: "GLOBAL_U16_LOAD",
    84: "GLOBAL_U16_STORE",
    85: "J",
    86: "JZ",
    87: "IEQ_JZ",
    88: "INE_JZ",
    89: "IGT_JZ",
    90: "IGE_JZ",
    91: "ILT_JZ",
    92: "ILE_JZ",
    93: "CALL",
    94: "GLOBAL_U24",
    95: "GLOBAL_U24_LOAD",
    96: "GLOBAL_U24_STORE",
    97: "PUSH_CONST_U24",
    98: "SWITCH",
    99: "STRING",
    100: "STRINGHASH",
    101: "TEXT_LABEL_ASSIGN_STRING",
    102: "TEXT_LABEL_ASSIGN_INT",
    103: "TEXT_LABEL_APPEND_STRING",
    104: "TEXT_LABEL_APPEND_INT",
    105: "TEXT_LABEL_COPY",
    106: "CATCH",
    107: "THROW",
    108: "CALLINDIRECT",
    109: "PUSH_CONST_M1",
    110: "PUSH_CONST_0",
    111: "PUSH_CONST_1",
    112: "PUSH_CONST_2",
    113: "PUSH_CONST_3",
    114: "PUSH_CONST_4",
    115: "PUSH_CONST_5",
    116: "PUSH_CONST_6",
    117: "PUSH_CONST_7",
    118: "PUSH_CONST_FM1",
    119: "PUSH_CONST_F0",
    120: "PUSH_CONST_F1",
    121: "PUSH_CONST_F2",
    122: "PUSH_CONST_F3",
    123: "PUSH_CONST_F4",
    124: "PUSH_CONST_F5",
    125: "PUSH_CONST_F6",
    126: "PUSH_CONST_F7"
}

SIMPLE_OPS = {
    0: [""],                                       # NOP
    1: ["<I4xI4x"],                                # IADD
    2: ["<I4xI4x"],                                # ISUB
    3: ["<I4xI4x"],                                # IMUL
    4: ["<I4xI4x"],                                # IDIV
    5: ["<I4xI4x"],                                # IMOD
    6: ["<I4x"],                                   # INOT
    7: ["<I4x"],                                   # INEG
    8: ["<I4xI4x"],                                # IEQ
    9: ["<I4xI4x"],                                # INE
    10: ["<I4xI4x"],                               # IGT
    11: ["<I4xI4x"],                               # IGE
    12: ["<I4xI4x"],                               # ILT
    13: ["<I4xI4x"],                               # ILE
    14: ["<f4xf4x"],                               # FADD
    15: ["<f4xf4x"],                               # FSUB
    16: ["<f4xf4x"],                               # FMUL
    17: ["<f4xf4x"],                               # FDIV
    18: ["<f4xf4x"],                               # FMOD
    19: ["<f4x"],                                  # FNEG
    20: ["<f4x"],                                  # FEQ
    21: ["<f4x"],                                  # FNE
    22: ["<f4x"],                                  # FGT
    23: ["<f4x"],                                  # FGE
    24: ["<f4x"],                                  # FLT
    25: ["<f4x"],                                  # FLE
    26: ["<f4xf4xf4xf4xf4xf4x"],                   # VADD
    27: ["<f4xf4xf4xf4xf4xf4x"],                   # VSUB
    28: ["<f4xf4xf4xf4xf4xf4x"],                   # VMUL
    29: ["<f4xf4xf4xf4xf4xf4x"],                   # VDIV
    30: ["<f4xf4xf4x"],                            # VNEG
    31: ["<I4xI4x"],                               # IAND
    32: ["<I4xI4x"],                               # IOR
    33: ["<I4xI4x"],                               # IXOR
    34: ["<I4x"],                                  # I2F
    35: ["<f4x"],                                  # F2I
    36: ["<f4x"],                                  # F2V
    37: ["<b"],                                    # PUSH_CONST_U8
    38: ["<bb"],                                   # PUSH_CONST_U8_U8
    39: ["<bbb"],                                  # PUSH_CONST_U8_U8_U8
    40: ["<I"],                                    # PUSH_CONST_U32
    41: ["<f"],                                    # PUSH_CONST_F
    42: ["<I4x"],                                  # DUP
    43: ["<I4x"],                                  # DROP
    44: ["<4x", "$STACK"],                         # NATIVE
    45: ["<5x", "$STACK"],                         # ENTER
    46: ["<5x", "$STACK"],                         # LEAVE
    47: ["$PTR", "<Q"],                            # LOAD
    48: ["$PTR", "<I4xI4x"],                       # STORE
    49: ["<I4x", "$PTR", "I4x"],                   # STORE_REV
    50: ["$PTR", "<I4xI4x", "$PTRS"],              # LOAD_N
    51: ["$PTR", "<I4xI4x", "$STACK"],             # STORE_N
    52: ["<1x", "$PTR", "<I4x", "$PTR", "<Q"],     # ARRAY_U8
    53: ["<1x", "$PTR", "<I4x", "$PTR", "<Q"],     # ARRAY_U8_LOAD
    54: ["<1x", "$PTR", "<I4xI4x" , "$PTR", "<Q"], # ARRAY_U8_STORE
    55: ["<1x", "$PTR", "<Q"],                     # LOCAL_U8
    56: ["<1x", "$PTR", "<Q"],                     # LOCAL_U8_LOAD
    57: ["<5xI", "$PTR", "<Q"],                    # LOCAL_U8_STORE
    58: ["<1x", "$PTR", "<Q"],                     # STATIC_U8
    59: ["<1x", "$PTR", "<Q"],                     # STATIC_U8_LOAD
    60: ["<5xI", "$PTR", "<Q"],                    # STATIC_U8_STORE
    61: ["<bI4x"],                                 # IADD_U8
    62: ["<bI4x"],                                 # IMUL_U8
    #63: OffsetWriteFunction,                      # IOFFSET
    64: ["<1x", "$PTR", "$PTR", "<Q"],             # IOFFSET_U8
    65: ["<1x", "$PTR", "$PTR", "<Q"],             # IOFFSET_U8_LOAD
    66: ["<1x", "$PTR", "<I4x", "$PTR", "<Q"],     # IOFFSET_U8_STORE
    67: ["<h8x"],                                  # PUSH_CONST_S16
    68: ["<hI4x"],                                 # IADD_S16
    69: ["<hI4x"],                                 # IMUL_S16
    70: ["<1x", "$PTR", "$PTR", "<Q"],             # IOFFSET_S16
    71: ["<1x", "$PTR", "$PTR", "<Q"],             # IOFFSET_S16_LOAD
    72: ["<1x", "$PTR", "<I4x", "$PTR", "<Q"],     # IOFFSET_S16_STORE
    73: ["<2x", "$PTR", "<I4x", "$PTR", "<Q"],     # ARRAY_U16
    74: ["<2x", "$PTR", "<I4x", "$PTR", "<Q"],     # ARRAY_U16_LOAD
    75: ["<2x", "$PTR", "<I4xI4x", "$PTR", "<Q"],  # ARRAY_U16_STORE
    76: ["<1x", "$PTR", "<Q"],                     # LOCAL_U16
    77: ["<1x", "$PTR", "<Q"],                     # LOCAL_U16_LOAD
    78: ["<5xI", "$PTR", "<Q"],                    # LOCAL_U16_STORE
    79: ["<h", "$PTR", "<Q"],                      # STATIC_U16
    80: ["<h", "$PTR", "<Q"],                      # STATIC_U16_LOAD
    81: ["<hI4x", "$PTR", "<Q"],                   # STATIC_U16_STORE
    82: ["<2x", "$PTR", "<Q"],                     # GLOBAL_U16
    83: ["<2x", "$PTR", "<Q"],                     # GLOBAL_U16_LOAD
    84: ["<6xI", "$PTR", "<Q"],                    # GLOBAL_U16_STORE
    85: [""],                                      # J
    86: ["<I4x"],                                  # JZ
    87: ["<I4xI4x"],                               # IEQ_JZ
    88: ["<I4xI4x"],                               # INE_JZ
    89: ["<I4xI4x"],                               # IGT_JZ
    90: ["<I4xI4x"],                               # IGE_JZ
    91: ["<I4xI4x"],                               # ILT_JZ
    92: ["<I4xI4x"],                               # ILE_JZ
    93: [""],                                      # CALL
    94: ["<3x", "$PTR", "<Q"],                     # GLOBAL_U24
    95: ["<3x", "$PTR", "<Q"],                     # GLOBAL_U24_LOAD
    96: ["<7xI", "$PTR", "<Q"],                    # GLOBAL_U24_STORE
    97: ["<3x"],                                   # PUSH_CONST_U24
    98: ["<I4x"],                                  # SWITCH
    99: [""],                                      # STRING
    100: ["x", "$PTR", "$STR"],                    # STRINGHASH
    101: ["x", "$PTR", "$PTR", "$STR", "$STR"],    # TEXT_LABEL_ASSIGN_STRING
    102: ["x", "$PTR", "<I4x", "$STR"],            # TEXT_LABEL_ASSIGN_INT
    103: ["x", "$PTR", "$PTR", "$STR", "$STR"],    # TEXT_LABEL_APPEND_STRING
    104: ["x", "$PTR", "<I4x", "$STR"],            # TEXT_LABEL_APPEND_INT
    105: [""],                                     # TEXT_LABEL_COPY
    106: [""],                                     # CATCH
    107: [""],                                     # THROW
    108: ["<Q"],                                   # CALLINDIRECT
    109: [""],                                     # PUSH_CONST_M1
    110: [""],                                     # PUSH_CONST_0
    111: [""],                                     # PUSH_CONST_1
    112: [""],                                     # PUSH_CONST_2
    113: [""],                                     # PUSH_CONST_3
    114: [""],                                     # PUSH_CONST_4
    115: [""],                                     # PUSH_CONST_5
    116: [""],                                     # PUSH_CONST_6
    117: [""],                                     # PUSH_CONST_7
    118: [""],                                     # PUSH_CONST_FM1
    119: [""],                                     # PUSH_CONST_F0
    120: [""],                                     # PUSH_CONST_F1
    121: [""],                                     # PUSH_CONST_F2
    122: [""],                                     # PUSH_CONST_F3
    123: [""],                                     # PUSH_CONST_F4
    124: [""],                                     # PUSH_CONST_F5
    125: [""],                                     # PUSH_CONST_F6
    126: [""],                                     # PUSH_CONST_F7
}

class Opcode:

    #######################################################
    def __init__ (self, ttdFile):
        self.data = ()
        self.ttd = ttdFile
        self.opcode = 0

    #######################################################
    def __read_internal (self):
        str_buffer_size = 0
        
        for desc in SIMPLE_OPS[self.opcode]:
            if desc == "$PTR":
                ptr, = self.ttd._read ("<Q")
                self.data = self.data + (self.ttd.resolve_ptr(ptr),)

            elif desc == "$PTRS":
                numItems, = self.ttd._read ("<I4x")
                items = []

                for i in range(numItems):
                    ptr, val = self.ttd._read("<QQ")
                    items.append((self.ttd.resolve_ptr(ptr), val))
                self.data = self.data + (items,)

            elif desc == "$STACK":
                num,  = self.ttd._read ("<b")
                self.data = self.data + (list(
                    '&' + self.ttd.resolve_ptr(i) \
                    for i in self.ttd._read(f"<{num}Q")),)

            elif desc == "$STR":
                if str_buffer_size == 0:
                    str_buffer_size, = self.ttd._read("<b")
                self.data = self.data + (self.ttd._read_bytes (str_buffer_size), )

            else:
                self.data = self.data + self.ttd._read(desc)
        
    #######################################################
    def read (self):
        self.ip, self.opcode = self.ttd._read ("<Qb")
        self.ip = self.ttd.convert_ptr_to_ip (self.ip)

        self.__read_internal ()

    #######################################################
    def __str__(self):
        return (f"{self.ip:08x}: {OP_NAMES[self.opcode]} "
                f"{', '.join(str(i) for i in self.data)}")
        pass

class TTDFile:
    
    def __init__ (self):
        self.script_name = ""
        self.thread_id = 0
        self.stack_ptr = 0
        self.code_blocks = []
        self.global_ptrs = []
        self.globals = []
        self.stack = b""
        pass


    #######################################################
    def resolve_ptr (self, ptr):
        if ptr >= self.stack_ptr and ptr <= self.stack_ptr + len(self.stack):
            return f"s_{(ptr - self.stack_ptr) // 8}"
        for global_ptr in self.global_ptrs:
            if global_ptr == 0:
                continue
            
            if ptr >= global_ptr and ptr <= global_ptr + 0x40000:
                return f"G_{ptr - global_ptr}"
        return str(ptr)
    
    #######################################################
    def convert_ptr_to_ip (self, ptr):
        CODE_BLOCK_SIZE = 0x4000
        for idx, code_block in enumerate(self.code_blocks):
            if ptr >= code_block and ptr <= code_block + CODE_BLOCK_SIZE:
                return idx * CODE_BLOCK_SIZE + ptr - code_block
        return -1
    
    #######################################################
    def _read_bytes (self, numBytes):
        return self.file.read (numBytes)
    
    #######################################################
    def _read (self, formatStr):
        return struct.unpack(formatStr,
                             self._read_bytes(struct.calcsize(formatStr)))
    
    #######################################################
    def _read_header (self):

        (self.script_name,
        self.thread_id,
        self.stack_ptr) = self._read("<6x64sIQ")

        self.script_name = self.script_name.split(b"\x00")[0].decode("ascii")
        
        numPages, = self._read ("<I")
        self.code_blocks = list(self._read (f"<{numPages}Q"))

    #######################################################
    def _read_globals (self):
        self.global_ptrs = list(self._read("<64Q"))
        for i in range(64):
            size, = self._read("<I")
            self.globals.append(self._read_bytes(size))


    #######################################################
    def _read_stack (self):
        size, = self._read("<I")
        self.stack = self._read_bytes(8*size)
            
    #######################################################
    def read_file (self, f):
        self.file = f

        self._read_header ()
        self._read_globals ()
        self._read_stack ()

        depth = 0
        for i in range(int(sys.argv[2])):
            op = Opcode (self)
            op.read ()

            if op.opcode == 45:
                depth += 1
                
            print('\t'*depth, op, sep = '')

            if op.opcode == 46:
                depth -= 1

ttd = TTDFile ()
ttd.read_file (open(sys.argv[1], "rb"))
