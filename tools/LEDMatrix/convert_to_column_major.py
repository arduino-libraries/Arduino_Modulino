"""
Script to convert C frame data in uint32_t big-endian row-major order 
into uint8_t or uint32_t C format column-major order for Modulino LED Matrix animations.
You can use the output of this script to provide the frame data to your animation code. E.g.:

```
constexpr uint8_t animation[][16] = {
    { 0x00, 0x00, 0x00, 0xfe, 0x82, 0x8a, 0x82, 0x8a, 0x82, 0xfe, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0xfe, 0x82, 0x8a, 0xa2, 0x8a, 0x82, 0xfe, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0xfe, 0x82, 0xaa, 0xa2, 0xaa, 0x82, 0xfe, 0x00, 0x00, 0xe8, 0x03, 0x00, 0x00 }
};

matrix.setMode(DisplayMode::MONOCHROMATIC_VERTICAL);
matrix.setSequence(animation);
matrix.play();
```

Usage: python convert_frames_c.py <input_file> [--format uint8|uint32]

Expected input format:
	Data type: uint32_t (32-bit unsigned integers), 4th value is duration in milliseconds
	Endianness: Big-endian bit ordering within the 32-bit words
	Pixel order: Row-major (pixels are ordered left-to-right, top-to-bottom)

Example input:
	const uint32_t animation[][4] = {
		{ 0x1fc10, 0x41041041, 0x41041fc, 66 },
		{ 0x1fc10, 0x41441041, 0x41041fc, 66 }
	};

Output format (uint8, default):
	constexpr uint8_t frames[][16] = {
		{ 0x00, 0x00, 0x00, 0xfe, 0x82, 0x82, 0x82, 0x82, 0x82, 0xfe, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00 },
		{ 0x00, 0x00, 0x00, 0xfe, 0x82, 0x8a, 0x82, 0x82, 0x82, 0xfe, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00 }
	};
	- Each frame is represented as 16 bytes: 12 bytes for the column data (each byte corresponds to a column, with bits representing rows) 
	followed by 4 bytes for the duration (in milliseconds, little-endian format)

Output format (uint32):
	constexpr uint32_t frames[][4] = {
		{ 0x000000fe, 0x82828282, 0x82fe0000, 66 },
    	{ 0x000000fe, 0x828a8282, 0x82fe0000, 66 }
	};
	- Each frame is represented as 4 words: 3x 32-bit unsigned words for the column data 
	followed by an integer for the duration (in milliseconds)

Initial author: Sebastian Romero (s.romero@arduino.cc)
Copyright (C) Arduino s.r.l. and/or its affiliated companies
"""

import argparse
import re
import struct
import sys

parser = argparse.ArgumentParser(description="Convert Modulino LED Matrix animations.")
parser.add_argument("input_file", help="Input C/C++ file containing frame/icon arrays")
parser.add_argument("--format", choices=["uint8", "uint32"], default="uint8", help="Output format type (default: uint8)")
args = parser.parse_args()

with open(args.input_file, 'r') as f:
	cpp_code = f.read()

if not cpp_code.strip():
	print("Input file is empty.")
	sys.exit(1)

output_format = args.format

# RegEx to match an array declaration and its content (supports both 1D and 2D arrays)
array_pattern = re.compile(r"(?:const\s+|constexpr\s+)?(?:uint32_t|int|uint8_t)\s+(\w+)\s*(?:\[[^\]]*\])+\s*=\s*\{([\s\S]*?)\}\s*;")

# RegEx to match content inside brackets for a single frame
frame_pattern = re.compile(r"\{\s*(0x[0-9a-fA-F]+)\s*,\s*(0x[0-9a-fA-F]+)\s*,\s*(0x[0-9a-fA-F]+)\s*,\s*(\d+)\s*\}")

# RegEx to match a single icon (1D array of 3 hex values)
icon_pattern = re.compile(r"^\s*(0x[0-9a-fA-F]+)\s*,\s*(0x[0-9a-fA-F]+)\s*,\s*(0x[0-9a-fA-F]+)\s*$")

def print_as_uint32_array(frames, array_name="frames"):
	print(f'constexpr uint32_t {array_name}[][4] = {{')
	for frame_bytes, duration in frames:
		# Pack 12 bytes into 3x 32-bit integers (Big Endian)
		ints = struct.unpack('>III', frame_bytes)
		print(f"    {{ 0x{ints[0]:x}, 0x{ints[1]:x}, 0x{ints[2]:x}, {duration} }},")
	print('};\n')

def print_as_uint8_array(frames, array_name="frames", skip_duration=False):
	if skip_duration:
		print(f'constexpr uint8_t {array_name}[][12] = {{')
	else:
		print(f'constexpr uint8_t {array_name}[][16] = {{')
	for frame_bytes, duration in frames:
		byte_literals = ", ".join(f"0x{b:02x}" for b in frame_bytes)
		if skip_duration:
			print(f"    {{ {byte_literals} }}, // Duration: {duration} ms")
		else:
			# Include duration as 4 bytes at the end (Little Endian)
			duration_bytes = struct.pack('<I', duration)
			duration_literals = ", ".join(f"0x{b:02x}" for b in duration_bytes)
			print(f"    {{ {byte_literals}, {duration_literals} }},")
	print('};\n')

def print_as_uint8_icon(frame_bytes, array_name="icon"):
	byte_literals = ", ".join(f"0x{b:02x}" for b in frame_bytes)
	print(f"constexpr uint8_t {array_name}[12] = {{ {byte_literals} }};\n")

def print_as_uint32_icon(frame_bytes, array_name="icon"):
	# Pack 12 bytes into 3x 32-bit integers (Big Endian)
	ints = struct.unpack('>III', frame_bytes)
	print(f"constexpr uint32_t {array_name}[3] = {{ 0x{ints[0]:08x}, 0x{ints[1]:08x}, 0x{ints[2]:08x} }};\n")

# Find all arrays in the input file
found_arrays = False
for array_match in array_pattern.finditer(cpp_code):
    found_arrays = True
    array_name = array_match.group(1)
    array_content = array_match.group(2)
    
    # Check if it's a single icon
    icon_match = icon_pattern.match(array_content)
    if icon_match:
        frame_parts = [int(icon_match.group(i), 16) for i in range(1, 4)]
        
        column_buffer = bytearray(12)
        for col in range(12):
            for row in range(8):
                linear_pixel_index = row * 12 + col
                part_index = linear_pixel_index // 32
                bit_index = 31 - (linear_pixel_index % 32)
                
                if (frame_parts[part_index] >> bit_index) & 1:
                    column_buffer[col] |= (1 << row)
                    
        if output_format == "uint32":
            print_as_uint32_icon(bytes(column_buffer), array_name=array_name)
        else:
            print_as_uint8_icon(bytes(column_buffer), array_name=array_name)
        continue

    # Otherwise treat it as an animation with multiple frames
    frames = []
    for match in frame_pattern.finditer(array_content):
        frame_parts = [int(match.group(i), 16) for i in range(1, 4)]
        duration = int(match.group(4))
        
        # Conversion logic
        column_buffer = bytearray(12)
        for col in range(12):
            for row in range(8):
                linear_pixel_index = row * 12 + col
                part_index = linear_pixel_index // 32
                bit_index = 31 - (linear_pixel_index % 32)
                
                if (frame_parts[part_index] >> bit_index) & 1:
                    column_buffer[col] |= (1 << row)
        
        frames.append((bytes(column_buffer), duration))
    
    if frames:
        if output_format == "uint32":
            print_as_uint32_array(frames, array_name=array_name)
        else:
            print_as_uint8_array(frames, array_name=array_name, skip_duration=False)

if not found_arrays:
    print("No matching arrays found in the input file.")