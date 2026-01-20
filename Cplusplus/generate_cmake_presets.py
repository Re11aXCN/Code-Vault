#!/usr/bin/env python3
"""
Generate CMakePresets.json file with support for multiple compilers, architectures, and build types.

Usage:
    python generate_cmake_presets.py [--config <config-file>] [--compilers <compilers>] [--architectures <architectures>] [--build-types <build-types>]

Examples:
    # Use default configuration
    python generate_cmake_presets.py
    
    # Use custom command line options
    python generate_cmake_presets.py --compilers msvc clang --architectures x64 --build-types debug release
    
    # Use configuration file
    python generate_cmake_presets.py --config config.json
    
    # Configuration file content example (config.json):
    # {
    #     "compilers": ["msvc"],
    #     "architectures": ["x64"],
    #     "build_types": ["debug", "release"]
    # }
"""

import argparse
import json
import os
import sys


def generate_cmake_presets(compilers=None, architectures=None, build_types=None):
    """
    Generate CMakePresets.json content.
    
    Args:
        compilers: List of compilers to include (msvc, clang, gcc)
        architectures: List of architectures to include (x86, x64, arm, arm64)
        build_types: List of build types to include (debug, minisizerel, relwithdebinfo, release)
    
    Returns:
        str: Formatted JSON content for CMakePresets.json
    """
    # Default values if not provided
    if compilers is None:
        compilers = ['msvc', 'clang', 'gcc']
    
    if architectures is None:
        architectures = ['x86', 'x64', 'arm', 'arm64']
    
    if build_types is None:
        build_types = ['debug', 'minisizerel', 'relwithdebinfo', 'release']
    
    # Base configuration
    presets = {
        "version": 9,
        "configurePresets": []
    }
    
    # Windows base preset
    windows_base = {
        "name": "windows-base",
        "hidden": True,
        "generator": "Ninja",
        "binaryDir": "${sourceDir}/out/build/${presetName}",
        "installDir": "${sourceDir}/out/install/${presetName}",
        "condition": {
            "type": "equals",
            "lhs": "${hostSystemName}",
            "rhs": "Windows"
        }
    }
    presets["configurePresets"].append(windows_base)
    
    # Linux base preset
    linux_base = {
        "name": "linux-base",
        "hidden": True,
        "generator": "Ninja",
        "binaryDir": "${sourceDir}/out/build/${presetName}",
        "installDir": "${sourceDir}/out/install/${presetName}",
        "condition": {
            "type": "equals",
            "lhs": "${hostSystemName}",
            "rhs": "Linux"
        }
    }
    presets["configurePresets"].append(linux_base)
    
    # macOS base preset
    macos_base = {
        "name": "macos-base",
        "hidden": True,
        "generator": "Ninja",
        "binaryDir": "${sourceDir}/out/build/${presetName}",
        "installDir": "${sourceDir}/out/install/${presetName}",
        "condition": {
            "type": "equals",
            "lhs": "${hostSystemName}",
            "rhs": "Darwin"
        }
    }
    presets["configurePresets"].append(macos_base)
    
    # Compiler base presets
    compiler_presets = {
        "msvc": {
            "name": "msvc-base",
            "hidden": True,
            "inherits": "windows-base",
            "cacheVariables": {
                "CMAKE_C_COMPILER": "cl.exe",
                "CMAKE_CXX_COMPILER": "cl.exe"
            }
        },
        "clang": {
            "name": "clang-base",
            "hidden": True,
            "cacheVariables": {
                "CMAKE_C_COMPILER": "clang.exe" if os.name == 'nt' else "clang",
                "CMAKE_CXX_COMPILER": "clang++.exe" if os.name == 'nt' else "clang++"
            }
        },
        "gcc": {
            "name": "gcc-base",
            "hidden": True,
            "cacheVariables": {
                "CMAKE_C_COMPILER": "gcc.exe" if os.name == 'nt' else "gcc",
                "CMAKE_CXX_COMPILER": "g++.exe" if os.name == 'nt' else "g++"
            }
        }
    }
    
    # Add compiler presets to the list
    for compiler in compilers:
        if compiler in compiler_presets:
            # Set appropriate base for clang and gcc based on OS
            if compiler in ['clang', 'gcc']:
                if os.name == 'nt':
                    compiler_presets[compiler]['inherits'] = 'windows-base'
                elif os.name == 'posix':
                    if os.uname().sysname == 'Darwin':
                        compiler_presets[compiler]['inherits'] = 'macos-base'
                    else:
                        compiler_presets[compiler]['inherits'] = 'linux-base'
            
            presets["configurePresets"].append(compiler_presets[compiler])
    
    # Architecture configurations
    arch_configs = {
        "x86": {
            "value": "x86",
            "strategy": "external"
        },
        "x64": {
            "value": "x64",
            "strategy": "external"
        },
        "arm": {
            "value": "arm",
            "strategy": "external"
        },
        "arm64": {
            "value": "arm64",
            "strategy": "external"
        }
    }
    
    # Build type configurations
    build_type_configs = {
        "debug": {
            "CMAKE_BUILD_TYPE": "Debug"
        },
        "minisizerel": {
            "CMAKE_BUILD_TYPE": "MinSizeRel"
        },
        "relwithdebinfo": {
            "CMAKE_BUILD_TYPE": "RelWithDebInfo"
        },
        "release": {
            "CMAKE_BUILD_TYPE": "Release"
        }
    }
    
    # Generate presets for each combination of compiler, architecture, and build type
    for compiler in compilers:
        for arch in architectures:
            for build_type in build_types:
                # Skip unsupported combinations
                if compiler == 'msvc' and arch in ['arm', 'arm64'] and os.name != 'nt':
                    continue
                
                # Create preset name: compiler_arch_buildtype
                preset_name = f"{compiler}_{arch}_{build_type}"
                display_name = f"{compiler.upper()} {arch.upper()} {build_type.capitalize()}"
                
                # Create preset
                preset = {
                    "name": preset_name,
                    "displayName": display_name,
                    "inherits": f"{compiler}-base",
                    "architecture": arch_configs[arch],
                    "cacheVariables": build_type_configs[build_type]
                }
                
                presets["configurePresets"].append(preset)
    
    # Convert to JSON string with indentation
    return json.dumps(presets, indent=4, default=lambda x: str(x))


def load_config(config_file):
    """
    Load configuration from a JSON file.
    
    Args:
        config_file: Path to the configuration file
        
    Returns:
        dict: Configuration dictionary
    """
    if not os.path.exists(config_file):
        print(f"❌ Configuration file not found: {config_file}")
        sys.exit(1)
    
    try:
        with open(config_file, 'r') as f:
            config = json.load(f)
        return config
    except json.JSONDecodeError:
        print(f"❌ Invalid JSON format in configuration file: {config_file}")
        sys.exit(1)


def main():
    """
    Main function to handle command-line arguments and generate CMakePresets.json.
    """
    parser = argparse.ArgumentParser(description="Generate CMakePresets.json with multiple compiler/architecture/buildtype support")
    
    # Configuration file option
    parser.add_argument('--config', type=str, help='Path to configuration file')
    
    # Optional arguments
    parser.add_argument('--compilers', nargs='+', choices=['msvc', 'clang', 'gcc'],
                        help='List of compilers to include')
    
    parser.add_argument('--architectures', nargs='+', choices=['x86', 'x64', 'arm', 'arm64'],
                        help='List of architectures to include')
    
    parser.add_argument('--build-types', nargs='+', choices=['debug', 'minisizerel', 'relwithdebinfo', 'release'],
                        help='List of build types to include')
    
    # Windows default configuration shortcut
    parser.add_argument('--windows-default', action='store_true',
                        help='Use Windows default configuration (msvc, x64, debug/release)')
    
    args = parser.parse_args()
    
    # Load configuration from file if provided
    config = {}
    if args.config:
        config = load_config(args.config)
    
    # Windows default configuration
    if args.windows_default:
        args.compilers = ['msvc']
        args.architectures = ['x64']
        args.build_types = ['debug', 'release']
    
    # Command line arguments override configuration file
    compilers = args.compilers if args.compilers is not None else config.get('compilers')
    architectures = args.architectures if args.architectures is not None else config.get('architectures')
    build_types = args.build_types if args.build_types is not None else config.get('build_types')
    
    # Generate presets
    presets_content = generate_cmake_presets(
        compilers=compilers,
        architectures=architectures,
        build_types=build_types
    )
    
    # Write to file
    output_path = os.path.join(os.path.dirname(__file__), 'CMakePresets.json')
    with open(output_path, 'w') as f:
        f.write(presets_content)
    
    # Print summary
    print(f"✅ Generated CMakePresets.json at {output_path}")
    print(f"📋 Generated presets for:")
    print(f"   - Compilers: {', '.join(compilers) if compilers else 'msvc, clang, gcc'}")
    print(f"   - Architectures: {', '.join(architectures) if architectures else 'x86, x64, arm, arm64'}")
    print(f"   - Build types: {', '.join(build_types) if build_types else 'debug, minisizerel, relwithdebinfo, release'}")


if __name__ == "__main__":
    main()
