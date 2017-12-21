import os
import argparse
import sys

VALID_PLATFORMS = ["win32", "darwin", "linux"]

app_folder, script_name = os.path.split(os.path.abspath(sys.argv[0]))

def message_and_die(message):
    print('\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!')
    print(message)
    print('!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n')
    sys.exit(1)


def path_to_os(path):
    full_path = os.path.abspath(path)
    parts = full_path.split("/")
    return os.path.sep.join(parts)


def create_folder_if_not_exists(path):
    if not os.path.exists(path):
        os.makedirs(path)


generate_file_template_SHELL="""###THIS IS AN AUTOGENERATED FILE###
#!/bin/bash

PROJECT_PATH="{project_folder_path}"
EXTRA_CMAKE_PARAMETERS=$1

if [ -d ${{PROJECT_PATH}} ]; then
    set -e
    rm -rf ${{PROJECT_PATH}}
fi

mkdir -p ${{PROJECT_PATH}}

cd ${{PROJECT_PATH}}

{cmake_call} ${{EXTRA_CMAKE_PARAMETERS}}
"""


generate_file_template_BATCH="""@echo off
REM THIS IS AN AUTOGENERATED FILE

set project_folder="{project_folder_path}"
set root=%cd%
set extra_cmake_parameters=%1

IF EXIST %project_folder% (
    rmdir /s /q %project_folder%
)

mkdir %project_folder%

cd %project_folder%

{cmake_call} %extra_cmake_parameters%

cd %root%
"""


update_file_template_SHELL="""###THIS IS AN AUTOGENERATED FILE###
#!/bin/bash

set PROJECT_PATH="{project_folder_path}"
EXTRA_CMAKE_PARAMETERS=$1

if [ ! -d ${{PROJECT_PATH}} ]; then
    mkdir -p ${{PROJECT_PATH}}
fi

cd ${{PROJECT_PATH}}

{cmake_call} ${{EXTRA_CMAKE_PARAMETERS}}
"""


update_file_template_BATCH="""@echo off
REM THIS IS AN AUTOGENERATED FILE

set project_folder="{project_folder_path}"
set root=%cd%
set extra_cmake_parameters=%1

IF NOT EXIST %project_folder% (
    mkdir %project_folder%
)

cd %project_folder%

{cmake_call} %extra_cmake_parameters%

cd %root%
"""


def generate_scripts_for_platform(platform, generate_tests, max_entities, max_tags, xcode):
    script_folder = path_to_os("{}/generate_solution_scripts/{}".format(app_folder, platform))

    file_extension = "sh"
    generate_template = generate_file_template_SHELL
    update_template = update_file_template_SHELL
    if platform == "win32":
        file_extension = "bat"
        generate_template = generate_file_template_BATCH
        update_template = update_file_template_BATCH

    generate_script_file_debug = path_to_os("{}/debug-01-generate.{}".format(script_folder, file_extension))
    generate_script_file_release = path_to_os("{}/release-01-generate.{}".format(script_folder, file_extension))
    update_script_file_debug = path_to_os("{}/debug-02-update.{}".format(script_folder, file_extension))
    update_script_file_release = path_to_os("{}/release-02-update.{}".format(script_folder, file_extension))
    project_folder_path_debug = path_to_os("{}/projects/{}-debug".format(app_folder, platform))
    project_folder_path_release = path_to_os("{}/projects/{}-release".format(app_folder, platform))

    create_folder_if_not_exists(script_folder)

    cmake_call_debug = generate_cmake_call("Debug", platform, generate_tests, max_entities, max_tags, xcode)
    cmake_call_release = generate_cmake_call("Release", platform, generate_tests, max_entities, max_tags, xcode)

    generate_templated_file(generate_script_file_debug, generate_template, project_folder_path_debug, cmake_call_debug)
    generate_templated_file(generate_script_file_release, generate_template, project_folder_path_release, cmake_call_release)

    generate_templated_file(update_script_file_debug, update_template, project_folder_path_debug, cmake_call_debug)
    generate_templated_file(update_script_file_release, update_template, project_folder_path_release, cmake_call_release)


def generate_cmake_call(configuration, platform, generate_tests, max_entities, max_tags, xcode):
    tests_flags = ""
    if(generate_tests):
        tests_flags = "-DDECS_ENABLE_TESTS=1"

    common_cmake_flags = ""
    platform_flags = ""
    if platform == "darwin" and xcode:
        platform_flags = "-G Xcode"

    root_cmake_path = path_to_os("{}/DonerECS".format(app_folder))

    return 'cmake "{}" -DCMAKE_CONFIGURATION_TYPES="{}" {} {} {} -DMAX_ENTITIES={} -DMAX_TAGS={}'.format(root_cmake_path, configuration, common_cmake_flags, platform_flags, tests_flags, max_entities, max_tags).replace("  ", " ").replace("  ", " ")


def generate_templated_file(script_file_path, template, project_folder_path, cmake_call):
    with open(script_file_path, "w") as file:
        file.write(template.format(
            project_folder_path=project_folder_path,
            cmake_call=cmake_call
        ))


def read_parameters():
    parser = argparse.ArgumentParser()
    parser.add_argument('--generate-tests', action='store_true', help="Generate Tests project", dest='generate_tests')
    parser.add_argument('--max-entities', help="Max number of entities. Maximum allowed is 8192. Default is 4096", default="4096", dest='max_entities')
    parser.add_argument('--max-tags', help="Max number of registrable tags. Rounded to highest power of two. Default is 64", default="64", dest='max_tags')
    parser.add_argument('--all-platforms', action='store_true', help="Generate scripts for all supported platforms", dest='all_platforms')
    parser.add_argument('--xcode', action='store_true', help="Generate OSX projects using XCode", dest='xcode')

    args = parser.parse_args()

    return args.generate_tests, args.max_entities, args.max_tags, args.all_platforms, args.xcode


def generate_scripts(generate_tests, max_entities, max_tags, all_platforms, xcode):
    if all_platforms:
        for platform in VALID_PLATFORMS:
            generate_scripts_for_platform(platform, generate_tests, max_entities, max_tags, xcode)
    elif sys.platform in VALID_PLATFORMS:
        generate_scripts_for_platform(sys.platform, generate_tests, max_entities, max_tags, xcode)
    else:
        message_and_die("You're working under an unsupported OS! Valid OS are {}".format(VALID_PLATFORMS))


if __name__ == '__main__':
    generate_tests, max_entities, max_tags, all_platforms, xcode = read_parameters()
    generate_scripts(generate_tests, max_entities, max_tags, all_platforms, xcode)
