{
  'variables': {
    'visibility%': 'hidden',         # V8's visibility setting
    'target_arch%': 'ia32',          # set v8's target architecture, ia32, x64
    'want_separate_host_toolset': 0, # V8 should not build target and host
    'library%': 'static_library',    # allow override to 'shared_library' for DLL/.so builds
    'component%': 'static_library',  # NB. these names match with what V8 expects
    'msvs_multi_core_compile': '0',  # we do enable multicore compiles, but not using the V8 way
    'gcc_version%': 'unknown',
    'clang%': 0,
    'python%': 'python',
    'os_posix': 0,
    'link_shared_boost%': 0,
    'android%': 0,
	'msvs_version%': 2010,
    'luajit%': 1,
    'nginx_release%': 1,
    'rapidjson_sse%': 'RAPIDJSON_SSE2',
    'enable_vmp%': 1,
  },

  'target_defaults': {
    'default_configuration': 'Release',
    'defines': [ 'UNICODE', '_UNICODE' ],
    'configurations': {
      'Debug': {
        'defines': [ 'DEBUG', '_DEBUG' ],
        'cflags': [ '-ggdb3', '-O0' ],
        'conditions': [
          ['target_arch=="x64"', {
            'msvs_configuration_platform': 'x64',
          }],
        ],
        'msvs_settings': {
          'VCCLCompilerTool': {
            'RuntimeLibrary': 3, # static debug
            'Optimization': 0, # /Od, no optimization
            'MinimalRebuild': 'false',
            'OmitFramePointers': 'false',
            'BasicRuntimeChecks': 3, # /RTC1
          },
          'VCLinkerTool': {
            'LinkIncremental': 2, # enable incremental linking
          },
        },
        'xcode_settings': {
          'GCC_OPTIMIZATION_LEVEL': '0', # stop gyp from defaulting to -Os
        },
      },
	  'ForTest': {
	    'inherit_from': ['Debug'],
        'defines': ['MGV_FOR_TEST'],
	  },
      'ReleaseNoLtcg': {
        'defines': [ 'NDEBUG' ],
        'cflags': [ '-O2', '-ffunction-sections', '-fdata-sections' ],
        'conditions': [
          ['target_arch=="x64"', {
            'msvs_configuration_platform': 'x64',
            'msvs_settings': {
              'VCCLCompilerTool': {
                'AdditionalOptions': ['/bigobj'],
              }
            }
          }],
          ['OS=="solaris"', {
            'cflags': [ '-fno-omit-frame-pointer' ],
            # pull in V8's postmortem metadata
            'ldflags': [ '-Wl,-z,allextract' ]
          }, {
            # Doesn't work with the Solaris linker.
            'ldflags': [ '-Wl,--gc-sections' ],
          }],
          ['clang == 0 and gcc_version >= 40', {
            'cflags': [ '-fno-tree-vrp' ],  # Work around compiler bug.
          }],
          ['clang == 0 and gcc_version <= 44', {
            'cflags': [ '-fno-tree-sink' ],  # Work around compiler bug.
          }],
        ],
        'msvs_settings': {
          'VCCLCompilerTool': {
            'RuntimeLibrary': 2, # static release
            'Optimization': 3, # /Ox, full optimization
            'FavorSizeOrSpeed': 1, # /Ot, favour speed over size
            'InlineFunctionExpansion': 2, # /Ob2, inline anything eligible
            'OmitFramePointers': 'true',
            'EnableFunctionLevelLinking': 'true',
            'EnableIntrinsicFunctions': 'true',
            'RuntimeTypeInfo': 'true',
            'ExceptionHandling': '1',
            #'AdditionalOptions': [
			#	'/MP', # compile across multiple CPUs
            #],
          },
          'VCLinkerTool': {
            'OptimizeReferences': 1, # /OPT:REF, to avoid ffmpeg runtime error
            'EnableCOMDATFolding': 2, # /OPT:ICF
            'LinkIncremental': 1, # disable incremental linking
          },
        },
      },
      'Release': {
	    'inherit_from': ['ReleaseNoLtcg'],
        'msvs_settings': {
          'VCCLCompilerTool': {
            'WholeProgramOptimization': 'true', # /GL, whole program optimization, needed for LTCG
          },
          'VCLibrarianTool': {
            'AdditionalOptions': [
              '/LTCG', # link time code generation
            ],
          },
          'VCLinkerTool': {
            'LinkTimeCodeGeneration': 1, # link-time code generation
          },
        }
	  },
    },
    'msvs_settings': {
      'VCCLCompilerTool': {
        'StringPooling': 'true', # pool string literals
        'DebugInformationFormat': 3, # Generate a PDB
        'WarningLevel': 3,
        'BufferSecurityCheck': 'true',
        'ExceptionHandling': 1, # /EHsc
        'SuppressStartupBanner': 'true',
        'WarnAsError': 'false',
      },
      'VCLibrarianTool': {
        'conditions': [
          ['target_arch=="x64"', {
            'TargetMachine' : 17 # /MACHINE:X64
          }],
          ['target_arch=="ia32"', {
            'TargetMachine' : 1 # /MACHINE:X86
          }],
        ],
      },
      'VCLinkerTool': {
        'conditions': [
          ['target_arch=="x64"', {
            'TargetMachine' : 17 # /MACHINE:X64
          }],
          ['target_arch=="ia32"', {
            'TargetMachine' : 1 # /MACHINE:X86
          }],
        ],
        'GenerateDebugInformation': 'true',
        'RandomizedBaseAddress': 2, # enable ASLR
        'DataExecutionPrevention': 2, # enable DEP
        'AllowIsolation': 'true',
        'SuppressStartupBanner': 'true',
        'target_conditions': [
          ['_type=="executable"', {
            'SubSystem': 1, # console executable
          }],
        ],
      },
    },
    'msvs_disabled_warnings': [4251, 4351, 4355, 4503, 4800, 4819],
    'conditions': [
      ['OS == "win"', {
        'msvs_cygwin_shell': 0, # prevent actions from trying to use cygwin
        'defines': [
          'WIN32',
          'WINDOWS',
          '_WIN32',
          # we don't really want VC++ warning us about
          # how dangerous C functions are...
          '_CRT_SECURE_NO_DEPRECATE',
          # ... or that C implementations shouldn't use
          # POSIX names
          '_CRT_NONSTDC_NO_DEPRECATE',

          '_CRT_SECURE_NO_WARNINGS',
          '_CRT_NONSTDC_NO_WARNINGS',
          '_SCL_SECURE_NO_WARNINGS',
          '_SCL_SECURE_NO_DEPRECATE',
        ],
      }],
      [ 'OS in "linux freebsd openbsd solaris" and android == 0', {
        'cflags': [ '-pthread', ],
        'ldflags': [ '-pthread' ],
      }],
      [ 'OS in "linux freebsd openbsd solaris" and android == 0', {
        'cflags': [ '-Wall', '-Wextra', '-Wno-unused-parameter', ],
        'cflags_cc': [ '-frtti', '-fexceptions' ],
        'ldflags': [ '-rdynamic' ],
        'target_conditions': [
          ['_type=="static_library"', {
            'standalone_static_library': 1, # disable thin archive which needs binutils >= 2.19
          }],
        ],
        'conditions': [
          [ 'target_arch=="ia32"', {
            'cflags': [ '-m32' ],
            'ldflags': [ '-m32' ],
          }],
          [ 'target_arch=="x64"', {
            'cflags': [ '-m64' ],
            'ldflags': [ '-m64' ],
          }],
          [ 'OS=="solaris"', {
            'cflags': [ '-pthreads' ],
            'ldflags': [ '-pthreads' ],
            'cflags!': [ '-pthread' ],
            'ldflags!': [ '-pthread' ],
          }],
        ],
      }],
      [ 'android == 1', {
	    'cflags': [ '-Wall', '-Wextra', '-Wno-unused-parameter', ],
        'cflags_cc': [ '-frtti', '-fexceptions', '-fpermissive' ],
        'ldflags': [ '-rdynamic' ],
        'defines': ['_GLIBCXX_USE_C99_MATH'],
        'libraries': [ '-llog' ],
        'defines!': [ 'UNICODE', '_UNICODE' ],
      }],
      ['OS=="mac"', {
        'defines': ['_DARWIN_USE_64_BIT_INODE=1'],
        'xcode_settings': {
          'ALWAYS_SEARCH_USER_PATHS': 'NO',
          'GCC_CW_ASM_SYNTAX': 'NO',                # No -fasm-blocks
          'GCC_DYNAMIC_NO_PIC': 'NO',               # No -mdynamic-no-pic
                                                    # (Equivalent to -fPIC)
          'GCC_ENABLE_CPP_EXCEPTIONS': 'NO',        # -fno-exceptions
          'GCC_ENABLE_CPP_RTTI': 'NO',              # -fno-rtti
          'GCC_ENABLE_PASCAL_STRINGS': 'NO',        # No -mpascal-strings
          'GCC_THREADSAFE_STATICS': 'NO',           # -fno-threadsafe-statics
          'PREBINDING': 'NO',                       # No -Wl,-prebind
          'MACOSX_DEPLOYMENT_TARGET': '10.5',       # -mmacosx-version-min=10.5
          'USE_HEADERMAP': 'NO',
          'OTHER_CFLAGS': [
            '-fno-strict-aliasing',
          ],
          'WARNING_CFLAGS': [
            '-Wall',
            '-Wendif-labels',
            '-W',
            '-Wno-unused-parameter',
          ],
        },
        'target_conditions': [
          ['_type!="static_library"', {
            'xcode_settings': {'OTHER_LDFLAGS': ['-Wl,-search_paths_first']},
          }],
        ],
        'conditions': [
          ['target_arch=="ia32"', {
            'xcode_settings': {'ARCHS': ['i386']},
          }],
          ['target_arch=="x64"', {
            'xcode_settings': {'ARCHS': ['x86_64']},
          }],
        ],
      }],
      ['OS=="freebsd" and node_use_dtrace=="true"', {
        'libraries': [ '-lelf' ],
      }]
    ],
  }
}
