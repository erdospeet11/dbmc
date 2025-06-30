#!/bin/bash
set -e

# colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        echo "linux"
    elif [[ "$OSTYPE" == "cygwin" ]] || [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
        echo "windows"
    elif [[ -n "$COMSPEC" ]] && [[ -x "$COMSPEC" ]]; then
        echo "windows"
    else
        # Fallback detection
        case "$(uname -s)" in
            Linux*)     echo "linux";;
            CYGWIN*)    echo "windows";;
            MINGW*)     echo "windows";;
            MSYS*)      echo "windows";;
            *)          echo "unknown";;
        esac
    fi
}

check_dependencies() {
    if ! command -v gcc &> /dev/null; then
        print_error "GCC compiler not found. Please install GCC."
        exit 1
    fi
    
    print_info "GCC version: $(gcc --version | head -n1)"
}

#linux
build_linux() {
    print_info "Building for Linux..."
    
    if ! pkg-config --exists x11; then
        print_warning "X11 development libraries not found via pkg-config"
        print_warning "Trying to build anyway. If it fails, install: sudo apt-get install libx11-dev"
    fi
    
    local CFLAGS="-Wall -Wextra -std=c99"
    local LIBS="-lX11"
    local OUTPUT="dbmc"
    
    print_info "Compiling with: gcc $CFLAGS main.c -o $OUTPUT $LIBS"
    
    gcc $CFLAGS main.c -o $OUTPUT $LIBS
    
    if [[ $? -eq 0 ]]; then
        print_success "Build completed successfully!"
        print_info "Executable: ./$OUTPUT"
    else
        print_error "Build failed!"
        exit 1
    fi
}

#windows
build_windows() {
    print_info "Building for Windows..."
    
    local CFLAGS="-Wall -Wextra -std=c99"
    local LIBS="-lcomctl32 -lgdi32 -luser32"
    local OUTPUT="dbmc.exe"
    
    print_info "Compiling with: gcc $CFLAGS main.c -o $OUTPUT $LIBS"
    
    gcc $CFLAGS main.c -o $OUTPUT $LIBS
    
    if [[ $? -eq 0 ]]; then
        print_success "Build completed successfully!"
        print_info "Executable: ./$OUTPUT"
    else
        print_error "Build failed!"
        exit 1
    fi
}



clean() {
    print_info "Cleaning build artifacts..."
    rm -f dbmc dbmc.exe *.o *.obj
    print_success "Clean completed!"
}

show_help() {
    echo "DBMC Build Script"
    echo ""
    echo "Usage: ./build.sh [options]"
    echo ""
    echo "Options:"
    echo "  build, -b     Build the project (default)"
    echo "  clean, -c     Clean build artifacts"
    echo "  help, -h      Show this help message"
    echo ""
    echo "The script automatically detects your operating system and uses"
    echo "appropriate compiler flags and libraries."
}

main() {
    print_info "DBMC Build Script Starting..."
    
    case "$1" in
        clean|-c)
            clean
            exit 0
            ;;
        help|-h)
            show_help
            exit 0
            ;;
        build|-b|"")
            ;;
        *)
            print_error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac

    OS=$(detect_os)
    print_info "Detected OS: $OS"
    
    check_dependencies
    
    case "$OS" in
        linux)
            build_linux
            ;;
        windows)
            build_windows
            ;;
        *)
            print_error "Unsupported operating system: $OS"
            print_info "Supported platforms: Linux, Windows"
            exit 1
            ;;
    esac
    
    print_success "Build script completed!"
}

main "$@" 