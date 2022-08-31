import os
from argparse import ArgumentParser
from os.path import dirname
from sys import argv

SUPPORT_FILES = ("vert", "frag")


def get_file_lines(filename: str) -> list:
    return open(filename, 'r').readlines()


def read_shaders(dir_name: str, start_name: str = "") -> list:
    result_list = []
    for file in os.listdir(dir_name):
        f = dir_name + "/" + file
        if os.path.isdir(f):
            result_list = result_list + read_shaders(f, start_name + file.split('.')[0] + "_")
            continue
        if file.split('.')[-1] in SUPPORT_FILES:
            result_list.append((start_name + file, get_file_lines(f)))
        else:
            print(f"Skipping file: {file}")
    return result_list


def generate_header(header_name: str, shaders: list) -> None:
    if not header_name.endswith('.hpp') and not header_name.endswith(".h"):
        header_name = header_name + ".hpp"
    header = open(header_name, 'w')
    header.write("#pragma once\n\nnamespace Engine\n{\n")

    shaders_count = len(shaders)
    shader_num = 0
    for shader in shaders:
        try:
            shader_num += 1
            prototype = "\tconst char* " + str(shader[0]).replace('.', '_') + " = R\"***(\n"
            header.write(prototype)
            for line in shader[1]:
                header.write(line)

            header.write(")***\";\n")
            if shader_num < shaders_count:
                header.write("\n\n\n")
        except:
            pass

    header.write("}\n")


def generate(input_dir: str, output_dir: str, header_name: str) -> None:
    current_dir = dirname(argv[0])
    if(len(current_dir) > 0):
        os.chdir()
    generate_header(output_dir + "/" + header_name, read_shaders(input_dir))


if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument('--input', default='./Shaders/', help="Dir with shaders")
    parser.add_argument('--output', default='./src/EngineHeaders/', help="Out header dir")
    parser.add_argument('--name', default='shader_code', help="Header name")
    args = parser.parse_args()
    generate(str(args.input), str(args.output), str(args.name))
