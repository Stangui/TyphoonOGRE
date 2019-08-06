workspace "TyphoonEngine"
	architecture "x64"
	startproject "TyphoonApp"

	configurations
	{
		"Debug",
		"Release",
		"Shipping"
	}

	platforms { "x64" }

	outputdir = "%{cfg.buildcfg}-%{cfg.platform}"

	IncludeDir = {}
	IncludeDir["OGRE"] = "$(OGRE)/include/OGRE"
	IncludeDir["SDL2"] = "$(OGRE)/include/SDL2"

	project "TyphoonApp"
		kind "WindowedApp"
		language "C++"

		targetdir ("bin/" ..  outputdir .. "/%{prj.name}")
		objdir ("Intermediate/" .. outputdir .. "/%{prj.name}")

		files
		{
			"Private/**.cpp",
			"Public/**.h",
		}

		includedirs
		{
			"Public",
			"Public/Renderer",
			"Public/Input",
			IncludeDir.OGRE,
			IncludeDir.OGRE .. "/hlms",
			IncludeDir.OGRE .. "/hlms/Unlit",
			IncludeDir.OGRE .. "/hlms/PBS",
			IncludeDir.OGRE .. "/hlms/Common",
			IncludeDir.SDL2
		}

		libdirs
		{
			"$(OGRE)/lib"
		}

		links
		{
			"SDL2",
			"SDL2Main",
			"winmm",
			"imm32",
			"version"
		}

		filter "system:windows"
			cppdialect "C++11"
			staticruntime "Off"
			systemversion "latest"

			defines
			{
				"TYPHOON_PLATFORM_WINDOWS"
			}

		filter "configurations:Debug"
			defines "TYPHOON_DEBUG"
			symbols "On"
			libdirs { "$(OGRE)/lib/Debug" }
			links { "OgreMain_d", "OgreHlmsPbs_d", "OgreHlmsUnlit_d", "OgreOverlay_d" }


		filter "configurations:Release"
			defines { "TYPHOON_RELEASE", "OGRE_IGNORE_UNKNOWN_DEBUG" }
			symbols "On"
			optimize "On"
			libdirs { "$(OGRE)/lib/Release" }
			links { "OgreMain", "OgreHlmsPbs", "OgreHlmsUnlit", "OgreOverlay" }

		filter "configurations:Shipping"
			defines { "TYPHOON_SHIPPING", "OGRE_IGNORE_UNKNOWN_DEBUG" }
			optimize "On"
			libdirs { "$(OGRE)/lib/Release" }
			links { "OgreMain", "OgreHlmsPbs", "OgreHlmsUnlit", "OgreOverlay" }
