project "console-menu"
	kind "StaticLib"
	systemversion "latest"
	language "C++"
	cppdialect "C++17"
	staticruntime "On"
	
	-- check if the root premake defines paths and directories for the file generation
	location (rootLocationPath and rootLocationPath or "build")
	targetdir (path.join(rootTargetDir and rootTargetDir or "bin", "%{cfg.buildcfg}-%{cfg.architecture}"))
	objdir (path.join(rootObjectDir and rootObjectDir or "build", "%{cfg.buildcfg}-%{cfg.architecture}"))
	
	includedirs { "." }
	files { "**.h", "**.cpp", "*.lua" }
	
	filter "configurations:debug"
		defines { "BIN_DEBUG" }
		symbols "On"
		runtime "Debug"
	
	filter "configurations:release"
		defines { "BIN_RELEASE" }
		optimize "Full"
		symbols "On"
		runtime "Release"