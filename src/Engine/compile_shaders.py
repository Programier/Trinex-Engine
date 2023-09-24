import os

def find_files_with_extension(directory, extension):
    matching_files = []

    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith(extension):
                matching_files.append(os.path.join(root, file))

    return matching_files




def compile(in_file, in_extension, out_extension):
    out_file = str(in_file).replace(in_extension, out_extension)
    command = f'glslc {in_file} -g -o {out_file}'
    print('Execute:', command)
    os.system(command)


def compile_vertex_shaders():
    print('Compile vertex shaders')
    files = find_files_with_extension('shaders', '.vert')
    for file in files:
        compile(file, '.vert', '.vm')

def compile_fragment_shaders():
    print('Compile fragments shaders')
    files = find_files_with_extension('shaders', '.frag')
    for file in files:
        compile(file, '.frag', '.fm')


def main():
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    compile_vertex_shaders()
    compile_fragment_shaders()

if __name__ == '__main__':
    main()
