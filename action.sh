# MULTI-PURPOSE PROJECT SCRIPT
# Init variables
CONFIGURE=0;    # Configure CMake
INSTALL=0;      # Install build
REMOTE=0        # Copy to remote
REMOTE_USER=""; # Remote user
TEST=0          # Make tests
UNINSTALL=0;    # Uninstall build
BUILD_TYPE="";  # CMake build type
DIR="";         # Build output directory

# Example usage: ./action.sh debug -i
# Show user help
print_help() {
    echo "Usage: action.sh build|debug [OPTIONS]...";
    echo "All options (except help) require a valid target (build|debug)!"
    echo "Options:";
    echo "\t-c, --configure                 \t Force (re)generate CMake configuration files";
    echo "\t-h, --help                      \t Display this message";
    echo "\t-i, --install                   \t Install program (to ~/.sync) after build";
    echo "\t-t, --tests                     \t Build and run tests";
    echo "\t-u, --uninstall                 \t Uninstall program (from ~/.sync)";
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
        "-r" | "--remote")
            REMOTE=1;
            parse_mode="r"
            ;;
        "-t" | "--tests")
            TEST=1;
            ;;
        "-u" | "--uninstall")
            UNINSTALL=1;
            ;;
        "build")
            BUILD_TYPE="RELEASE";
            DIR="build";
            ;;
        "debug")
            BUILD_TYPE="DEBUG";
            DIR="debug";
            ;;
        *)
            case "$parse_mode" in
            "r")
                REMOTE_USER="${!arg_num}";
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
    cmake --build .;
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
        cmake --install .;
        echo "Installation complete!";
    fi
}

do_remote() {
    if [ $REMOTE -eq 1 ]; then
        echo "Copying files to remote: /home/$REMOTE_USER/.sync";
        sudo mkdir -p /home/$REMOTE_USER/.sync/bin;         # No clue why, but the normal way
        sudo cp -r -t /home/$REMOTE_USER/.sync ~/.sync/bin; # doesn't work for bin
        sudo cp -r ~/.sync/lib /home/$REMOTE_USER/.sync;
        sudo cp -r ~/.sync/include /home/$REMOTE_USER/.sync;
        sudo cp -r ~/.sync/res /home/$REMOTE_USER/.sync;
        sudo chown -R $REMOTE_USER:$REMOTE_USER /home/$REMOTE_USER/.sync
        echo "Copy complete!";
    fi
}

do_tests() {
    if [ $TEST -eq 1 ]; then
        echo "Running tests...";
        cd test;
        ./sync_test;
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
# If success, install
do_install;
# Copy over to another location
do_remote;
cd ..;
# Done!
