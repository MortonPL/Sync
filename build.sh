# MULTI-PURPOSE PROJECT SCRIPT
# Init variables
CONFIGURE=0;    # Configure CMake
INSTALL=0;      # Install build
OTHER_DIR=0     # Copy elsewhere after build
OTHER_USER="";  # Other user
TEST=0          # Run tests
UNINSTALL=0;    # Uninstall build
BUILD_TYPE="";  # CMake build type
DIR="";         # Build output directory
BUILD_NOT_ALL=0;
BUILD_GUI="";
BUILD_CLI="";
BUILD_TEST="";

# Example usage: ./build.sh release -i
# Show user help
print_help() {
    echo -e "Usage: build.sh release|debug [TARGETS] [OPTIONS]...";
    echo -e "All options (except help) require a valid target (release|debug)!"
    echo -e "Targets (if none set, build all):";
    echo -e "\tgui";
    echo -e "\tcli";
    echo -e "\ttest";
    echo -e "Options:";
    echo -e "\t-c, --configure                 \t Force (re)generate CMake configuration files";
    echo -e "\t-h, --help                      \t Display this message";
    echo -e "\t-i, --install                   \t Install program (to ~/.sync) after build";
    echo -e "\t-t, --tests                     \t Run tests after build";
    echo -e "\t-u, --uninstall                 \t Uninstall program (from ~/.sync)";
}

parse_args() {
    arg_num=1;
    parse_mode="";
    # Parse args
    while [ $arg_num -le $# ];
    do
        case "${!arg_num}" in
        "-h" | "--help")
            print_help;
            exit;
            ;;
        "-c" | "--configure")
            CONFIGURE=1;
            ;;
        "-i" | "--install")
            INSTALL=1;
            ;;
        "-o" | "--other")
            OTHER_DIR=1;
            parse_mode="r"
            ;;
        "-t" | "--tests")
            TEST=1;
            ;;
        "-u" | "--uninstall")
            UNINSTALL=1;
            ;;
        "release")
            BUILD_TYPE="RELEASE";
            DIR="release";
            ;;
        "debug")
            BUILD_TYPE="DEBUG";
            DIR="debug";
            ;;
        "gui")
            BUILD_GUI="syncgui";
            BUILD_NOT_ALL=1;
            ;;
        "cli")
            BUILD_CLI="synccli";
            BUILD_NOT_ALL=1;
            ;;
        "test")
            BUILD_TEST="synctest";
            BUILD_NOT_ALL=1;
            ;;
        *)
            case "$parse_mode" in
            "r")
                OTHER_USER="${!arg_num}";
                parse_mode="";
                ;;
            *)
                ;;
            esac
            ;;
        esac
        arg_num=$(($arg_num + 1));
    done
    if [ "$parse_mode" != "" ]; then
        echo "Invalid arguments. One or more options are missing values.";
        exit;
    fi
}

do_uninstall() {
    if [ "$UNINSTALL" -eq 1 ]; then
        echo "Uninstalling... ";
        sudo xargs rm < $DIR/install_manifest.txt;
        echo "Removal complete!";
        exit;
    fi
}

do_xrs() {
    cd rc;
    echo "Zipping resources...";
    zip -rq ../$DIR/sync.xrs *;
    rc=$?;
    if [ $rc -ne 0 ]; then
        echo "Failed to zip resources!"
        exit;
    fi
    echo "Done.";
    cd ..
}

do_config() {
    if [ $CONFIGURE -eq 1 ]; then
        echo "Force regenerating build configuration...";
        cmake ../ -DCMAKE_BUILD_TYPE=$BUILD_TYPE;
        echo "Done.";
    fi
}

do_build() {
    echo "Building...";
    if [ $BUILD_NOT_ALL -eq 1 ]; then
        cmake --build . --target "$BUILD_GUI" "$BUILD_CLI" "$BUILD_TEST";
    else
        cmake --build .;
    fi
    rc=$?;
    if [ $rc -eq 0 ]; then
        echo "Build finished.";
    else
        echo "Build failed!!!";
        exit;
    fi
}

do_install() {
    if [ $INSTALL -eq 1 ]; then
        echo "Installing... (note: only updated files will be listed)";
        mkdir -p ~/.sync;
        mkdir -p ~/.sync/bin;
        mkdir -p ~/.sync/res;
        if [ $BUILD_NOT_ALL -eq 1 ]; then
            if [ ! -z "$BUILD_GUI" ]; then
                cmake --install . --component $BUILD_GUI;
            fi
            if [ ! -z "$BUILD_CLI" ]; then
                cmake --install . --component $BUILD_CLI;
            fi
            if [ ! -z "$BUILD_TEST" ]; then
                cmake --install . --component $BUILD_TEST;
            fi
        else
            cmake --install .;
        fi
        echo "Installation complete!";
    fi
}

# currently unused
do_other_dir() {
    if [ $OTHER_DIR -eq 1 ]; then
        echo "Copying files to OTHER_DIR: /home/$OTHER_USER/.sync";
        sudo mkdir -p /home/$OTHER_USER/.sync/bin;         # No clue why, but the normal way
        sudo cp -r -t /home/$OTHER_USER/.sync ~/.sync/bin; # doesn't work for bin
        sudo cp -r ~/.sync/lib /home/$OTHER_USER/.sync;
        sudo cp -r ~/.sync/include /home/$OTHER_USER/.sync;
        sudo cp -r ~/.sync/res /home/$OTHER_USER/.sync;
        sudo chown -R $OTHER_USER:$OTHER_USER /home/$OTHER_USER/.sync
        echo "Copy complete!";
    fi
}

do_tests() {
    if [ $TEST -eq 1 ]; then
        echo "Running tests...";
        cd test;
        ./synctest;
        cd ..;
        rc=$?;
        if [ $rc -eq 0 ]; then
            echo "Tests finished.";
        else
            echo "Tests failed!!!";
            exit;
        fi
    else
        echo "Ignoring tests.";
    fi
}

# MAIN BODY STARTS HERE
parse_args $@;
if [ "$BUILD_TYPE" == "" ]; then
    echo "Build type not specified, exiting. Use --help for help.";
    exit 1;
fi
do_uninstall;
# Zip resources to .XRS
mkdir -p $DIR;
do_xrs;
cd $DIR;
# Generate CMake build configuration (opt-in)
do_config;
# Build the project
do_build;
printf '\a';  # Ring ring
# Run tests
do_tests;
# If success, install (opt-in)
do_install;
# Copy over to another location (opt-in)
do_other_dir;
cd ..;
# Done!
