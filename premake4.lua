
--[[ Win32 only so far ]]--

solution "Project"
--------------------------------------------------------------------------
configurations	{ "Release","Debug" }
	targetdir			"bin"
	debugdir			"bin"
	configuration "Debug"
		defines				{ "DEBUG", "_CRT_SECURE_NO_WARNINGS", "_WINSOCK_DEPRECATED_NO_WARNINGS" }
		flags					{ "Symbols" }
	configuration "Release"
		defines				{ "NDEBUG", "_CRT_SECURE_NO_WARNINGS", "_WINSOCK_DEPRECATED_NO_WARNINGS" ,"NoImportLib","NoIncrementalLink"}
		flags					{ "OptimizeSize" }

project "dtt"
	kind	"ConsoleApp"
	language	"C"
	objdir	"_build"
	targetprefix	""
	defines {"TIGR"}
	files	{"decode_test.c"}
	files	{"decoder.c"}
	files	{"bmp.c"}
	files	{"tigr.c"}
	links {"d3d9"}

project "dtc"
	kind	"ConsoleApp"
	language	"C"
	objdir	"_build"
	targetprefix	""
	files	{"decode_test.c"}
	files	{"decoder.c"}
	files	{"bmp.c"}

