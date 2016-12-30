{
  'target_defaults': {
    'configurations': {
      'Debug': {
        'msvs_settings': {
          'VCLinkerTool': {
            #'SubSystem': 2,
            #'EntryPointSymbol': 'wWinMainCRTStartup',
            #'UACExecutionLevel' : 2 ,#'RequireAdministrator'
          },
        },
      },
      'Release': {
        'msvs_settings': {
          'VCLinkerTool': {
            #'SubSystem': 2,
            #'EntryPointSymbol': 'wWinMainCRTStartup',
            #'UACExecutionLevel' : 2 ,#'RequireAdministrator'
          },
        },
      },
    },
  },
  'targets': [
    {
      'target_name':'CurlTest',
      'type':'executable',
      #'defines': [],
      'dependencies': [
	     '<(DEPTH)/3rdlibs/boost/boost.gyp:*',
         '<(DEPTH)/3rdlibs/curl/curl.gyp:*',
      ],
      'include_dirs': [
        '.',
      ],
      'variables': {
        'relative_path': '',
      },
      'sources': [
        'CurlTest.cpp',
        'CurlMgr.h',
        'CurlMgr.cpp',
      ],
    },
  ]
}
