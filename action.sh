# Init variables
RECONFIG=0  # Configure CMake

# Parse Arguments
if [ "$2" = "-h" ] || [ "$2" = "--help" ]; then
    echo "Uasge: action.sh build|debug [options]";
    echo "Options:"
    echo "\t-c, --config\t(Re)Generate CMake configuration files"
    echo "\t-h, --help\tDisplay this message"
fi
if [ "$1" = "build" ]; then
    DIR=build;
    BUILD_TYPE=RELEASE;
elif [ "$1" = "debug" ] ; then
    DIR=debug;
    BUILD_TYPE=DEBUG;
else
    echo "Invalid action type, use --help.";
    exit;
fi
if [ "$2" = "-c" ] || [ "$2" = "--config" ]; then
    RECONFIG=1;
fi

# Zip resources to .XRS
mkdir -p $DIR;
cd rc;
echo "Zipping resources...";
zip -rq ../$DIR/sync.xrs *;
echo "Done.";
cd ../$DIR;
# Generate CMake build configuration (opt-in)
if [ $RECONFIG -eq 1 ]; then
    echo "Generating build configuration...";
    cmake ../ -DCMAKE_BUILD_TYPE=$BUILD_TYPE;
    echo "Done.";
else
    echo "Omitting build configuration.";
fi
# Build the project
echo "Building...";
cmake --build .;
rc=$?;
echo "Done.";
printf '\a';  # Ring ring
# If success, install
if [ $rc -eq 0 ]; then
    echo "Installing... (note: only updated files will be listed)";
    sudo cmake --install .;
    echo "Installation complete!";
else
    echo "Build failed!!!";
fi
cd ..;
# Done!
