{
  'targets': [
    {
      'target_name': 'study',
      'type': 'none',
      'dependencies': [
        'StudyPro/StudyPro.gyp:*',
		'Another/Another.gyp:*',
      ],
      'sources': [
        'build/common.gypi',
        'build/win_precompile.gypi',
      ],
    },
  ],
}