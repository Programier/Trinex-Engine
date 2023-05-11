import os


INCLUDE_DIR = './include/'
SKIP_DIR = ['glm', 'net', 'LuaJIT', 'LuaBridge', 'ImGui']



def get_names_list():
    names = os.listdir()
    files = []
    dirs = []
    for name in names:
        if os.path.isdir(name):
            dirs.append(name)
        else:
            files.append(name)
    return files, dirs


def get_class_name(line):
    if "ENGINE_EXPORT" not in line:
        return ''
    
    if 'class' not in line and 'struct' not in line:
        return ''

    words: list = line.split()
    index = words.index('ENGINE_EXPORT')
    return words[index + 1]


def parse_member(classname, member):
    print(f'Member<{classname}>: {member}')

def process_file(filename):
    print("Processing file", filename)
    lines = open(filename).readlines()

    classes_stack = []

    need_skip = False

    for line in lines:
        line = line.strip()
        name = get_class_name(line)
        need_skip = need_skip or 'enum' in line

        if len(name) != 0:
            classes_stack.append(name)

        if len(classes_stack) > 0 and line.endswith(';'):
            parse_member(classes_stack[-1], line)

        if line == '};':
            if need_skip:
                need_skip = False
            else:
                classes_stack.pop(len(classes_stack) - 1)
        
    print(classes_stack)


def visit_dir(dirname):
    os.chdir(dirname)

    files, dirs = get_names_list()
    for file in files:
        process_file(file)

    for dir in dirs:
        if dir not in SKIP_DIR:
            visit_dir(dir)

    os.chdir('../')

visit_dir(INCLUDE_DIR)