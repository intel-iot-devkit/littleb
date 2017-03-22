{
	"targets": [
		{
			"includes": [
				"auto.gypi"
			],
			"target_name": "example",
			"sources": [
				"../../src/littlebtypes.cpp", "../../src/device.cpp",
				 "../../src/devicemanager.cpp", '../../src/littleb.c'
			],
			'include_dirs': [
			  '../../api/','../../include/',''
			],
          	"libraries": [
          	'-luv', "-L/usr/local/lib/", "-lsystemd", "-L/lib/x86_64-linux-gnu/", 
          	],
          	'conditions': [
			  ['OS=="linux" or OS=="freebsd" or OS=="openbsd" or OS=="solaris"', {
			    
    			'cflags_cc+': ['-frtti', '-std=c++11 '],
			  }]
]
		}
	],
	"includes": [
		"auto-top.gypi"
	]
}
