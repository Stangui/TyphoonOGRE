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

	IncludeDir = {}
	IncludeDir["OGRE"] = "ThirdParty/OGRE/include/OGRE"
	IncludeDir["SDL2"] = "ThirdParty/SDL2/include/SDL2"

	project "TyphoonApp"
		kind "WindowedApp"
		language "C++"

		targetdir ("bin/" .. "%{cfg.buildcfg}")
		objdir ("Intermediate/" .. "%{cfg.buildcfg}")

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

		links
		{
			"winmm",
			"imm32",
			"version"
		}

		libdirs
		{
			"ThirdParty/OGRE/lib/%{cfg.buildcfg}",
			"ThirdParty/SDL2/lib"
		}

		filter "system:windows"
			cppdialect "C++17"
			staticruntime "Off"
			systemversion "latest"

			defines
			{
				"TYPHOON_PLATFORM_WINDOWS"
			}

		filter "configurations:Debug"
			defines "TYPHOON_DEBUG"
			symbols "On"
			links { "SDL2_d", "SDL2main_d", "OgreMain_d", "OgreHlmsPbs_d", "OgreHlmsUnlit_d", "OgreOverlay_d" }

		filter "configurations:Release"
			defines { "TYPHOON_RELEASE", "OGRE_IGNORE_UNKNOWN_DEBUG" }
			symbols "On"
			optimize "On"
			links { "SDL2", "SDL2main", "OgreMain", "OgreHlmsPbs", "OgreHlmsUnlit", "OgreOverlay" }

		filter "configurations:Shipping"
			defines { "TYPHOON_SHIPPING", "OGRE_IGNORE_UNKNOWN_DEBUG" }
			optimize "On"
			libdirs { "ThirdParty/OGRE/lib/Release" }
			links { "SDL2", "SDL2main", "OgreMain", "OgreHlmsPbs", "OgreHlmsUnlit", "OgreOverlay" }
