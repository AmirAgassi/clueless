#!/usr/bin/env python3
import struct
import json
import os

def crack_asar(filename):
    with open(filename, 'rb') as f:
        data = f.read()
    
    print(f"file size: {len(data)} bytes")
    
    # read the corrupted header
    header_size = struct.unpack('<I', data[:4])[0]
    json_size1 = struct.unpack('<I', data[4:8])[0]
    json_size2 = struct.unpack('<I', data[8:12])[0]
    unknown = struct.unpack('<I', data[12:16])[0]
    
    print(f"corrupted header size: {header_size}")
    print(f"json size 1: {json_size1}")
    print(f"json size 2: {json_size2}")
    print(f"unknown field: {unknown}")
    
    # the json starts at byte 16 and should be json_size1 bytes long
    json_start = 16
    json_data = data[json_start:json_start+json_size1]
    
    try:
        # find the actual end of json by looking for the closing brace
        # and ignoring extra data
        json_str = json_data.decode('utf-8')
        
        # find balanced braces
        brace_count = 0
        json_end = 0
        for i, char in enumerate(json_str):
            if char == '{':
                brace_count += 1
            elif char == '}':
                brace_count -= 1
                if brace_count == 0:
                    json_end = i + 1
                    break
        
        if json_end > 0:
            clean_json = json_str[:json_end]
            json_obj = json.loads(clean_json)
            print("json successfully parsed after cleaning!")
            
            # file data starts after the json
            file_data_offset = json_start + json_end
            file_data = data[file_data_offset:]
            
            return json_obj, file_data
        else:
            print("could not find end of json")
            
    except Exception as e:
        print(f"failed to parse json: {e}")
    
    return None, None

def extract_files(json_obj, file_data, output_dir):
    # extract files from asar archive into output_dir
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    def extract_node(node, current_path):
        if 'files' in node:
            # directory
            if not os.path.exists(current_path):
                os.makedirs(current_path, exist_ok=True)
            for name, child in node['files'].items():
                new_path = os.path.join(current_path, name)
                extract_node(child, new_path)
        else:
            # file
            if 'size' in node and 'offset' in node:
                offset = int(node['offset'])
                size = int(node['size'])
                
                # ensure directory exists
                dir_path = os.path.dirname(current_path)
                if dir_path:
                    os.makedirs(dir_path, exist_ok=True)
                
                try:
                    with open(current_path, 'wb') as f:
                        f.write(file_data[offset:offset+size])
                    print(f"extracted: {current_path}")
                except Exception as e:
                    print(f"error extracting {current_path}: {e}")
    
    extract_node(json_obj, output_dir)

if __name__ == "__main__":
    json_obj, file_data = crack_asar('cluely/resources/app.asar')
    if json_obj and file_data:
        print(f"found {len(file_data)} bytes of file data")
        extract_files(json_obj, file_data, 'cluely_decompiled')
        print("extraction completed!")
        
        # also save the directory structure for analysis
        with open('directory_structure.json', 'w') as f:
            json.dump(json_obj, f, indent=2)
        print("directory structure saved to directory_structure.json")
    else:
        print("failed to crack asar file") 