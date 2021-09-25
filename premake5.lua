project "console-menu"
	kind "StaticLib"
	systemversion "latest"
	language "C++"
	cppdialect "C++17"
	staticruntime "On"
	
	-- check if the root premake defines a build/target/object-directory
	if rootBuildDir ~= nil then
		location (path.join(rootBuildDir, "console-menu"))
	else
		location "build"
	end
	if rootTargetDir ~= nil then
		targetdir (path.join(rootTargetDir, "console-menu/%{cfg.buildcfg}-%{cfg.architecture}"))
	else
		targetdir "bin/%{cfg.buildcfg}-%{cfg.architecture}"
	end
	if rootObjectDir ~= nil then
		objdir (path.join(rootObjectDir, "console-menu/%{cfg.buildcfg}-%{cfg.architecture}"))
	else
		objdir "build/%{cfg.buildcfg}-%{cfg.architecture}"
	end
	
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