import sys
import subprocess
import struct
import re
import shutil
import os
import textwrap

def get_patterns_from_obj(obj_file):
    raw_hex_string = ""
    if shutil.which("dumpbin"):
        cmd = ['dumpbin', '/RAWDATA', '/SECTION:.pat_ids', obj_file]
        tool = "dumpbin"
    elif shutil.which("llvm-objdump"):
        cmd = ['llvm-objdump', '-s', '-j', '.pat_ids', obj_file]
        tool = "objdump"
    else:
        print(f"Extraction tool not found for {obj_file}", file=sys.stderr)
        return []

    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0: return []

    for line in result.stdout.splitlines():
        if tool == "objdump":
            match = re.match(r'\s*[0-9a-fA-F]+\s+(([0-9a-fA-F]{8}\s*)+)', line)
        else:
            match = re.match(r'\s*[0-9a-fA-F]+:\s+(([0-9a-fA-F]{2}\s*)+)', line)

        if match:
            raw_hex_string += match.group(1).replace(" ", "")

    found_ids = set()
    for i in range(0, len(raw_hex_string), 8):
        chunk = raw_hex_string[i:i+8]
        if len(chunk) == 8:
            unpacked_id = struct.unpack('<I', bytes.fromhex(chunk))[0]
            if unpacked_id != 0: found_ids.add(unpacked_id)

    return sorted(list(found_ids))

def generate_resolve_patterns_body (all_ids):
    body = "\n"
    for id_val in all_ids:
        body += f"GameAddress<{id_val}>::SetResolvedAddress(manager.GetResolvedAddress ({id_val}));\n"
    return body

if __name__ == "__main__":
    if len(sys.argv) < 3: sys.exit(0)

    output_cpp = sys.argv[1]
    obj_files = sys.argv[2].split(";")
    tu_pattern_map = {}
    all_ids = set()

    for obj in obj_files:
        ids = get_patterns_from_obj(obj)
        if ids:
            all_ids.update(ids)
            base_name = os.path.basename(obj).replace('.obj', '').replace('.o', '')
            safe_name = base_name.replace('.', '_').replace('-', '_')
            tu_pattern_map[safe_name] = {"original": base_name, "ids": ids}

    resolve_patterns_body = textwrap.indent(generate_resolve_patterns_body (all_ids), "            ")
    with open(output_cpp, 'w') as f:
        f.write(textwrap.dedent(f"""
        // AUTO-GENERATED FILE - DO NOT EDIT
        #include <cstdint>
        #include <string_view>
        #include <memory/GameAddress.hh>
        #include <memory/RuntimePattern.hh>

        extern uintptr_t GetResolvedAddress(uint32_t);
        extern void HandleMissingPattern(uint32_t id, std::string_view tu_name);

        template<const uint32_t ... ids>
        bool CheckIds ()
        {{
            return (... && GameAddress<ids>::IsResolved ());
        }}

        void ResolveGameAddresses (const Rainbomizer::RuntimePatternManager &manager)
        {{{resolve_patterns_body}
        }}
        """))

        function_names = []
        for safe_name, data in tu_pattern_map.items():
            func_name = f"VerifyPatterns_{safe_name}"
            function_names.append(func_name)
            ids = ', '.join(str(i) for i in data['ids'])

            f.write (textwrap.dedent(f"""
            bool {func_name} () {{
                return CheckIds <{ids}> ();
            }}
            """))

        f.write("void VerifyAllTranslationUnits() {\n")
        for func in function_names: f.write(f"    {func}();\n")
        f.write("}\n")
