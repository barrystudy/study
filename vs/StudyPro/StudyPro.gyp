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
      'target_name':'StudyPro',
      'type':'executable',
      'msvs_precompiled_header': 'stdafx.h',
      'msvs_precompiled_source': 'stdafx.cpp',
      'msvs_disabled_warnings': [4995],
      #'defines': [],
      'dependencies': [
      ],
      'include_dirs': [
        '.',
      ],
      'variables': {
        'relative_path': '',
      },
      'sources': [
		'stdafx.h',
		'stdafx.cpp',
        'study.cpp',
      ],
    },
  ]
}
