// Copyright Epic Games, Inc. All Rights Reserved.
using System;
using System.IO;
using System.Runtime.InteropServices;
using UnrealBuildTool;

public class FFmpeg : ModuleRules
{
    public string ProjectDirectory{
        get{
            return Path.GetFullPath( Path.Combine(ModuleDirectory, "../../../../../")); 
        }
    }

    public string BinariesDirectory
    {
        get
        {
            return Path.GetFullPath(Path.Combine(PluginDirectory, "Binaries\\ThirdParty"));
        }
    }
    private void CopyDllAndLibToProjectBinaries(string Filepath, ReadOnlyTargetRules Target)
    { 
        string BinariesDir = Path.Combine(ProjectDirectory, "Binaries", Target.Platform.ToString());
        string Filename = Path.GetFileName(Filepath);
        string ToFileFullPath = Path.Combine(BinariesDir, Filename);
        if (!Directory.Exists(BinariesDir))
        {
            Directory.CreateDirectory(BinariesDir);
        }

        if (!File.Exists(Path.Combine(BinariesDir, Filename)))
        {
            File.Copy(Filepath, ToFileFullPath, true);
        }
        Console.WriteLine("DllPath : " + ToFileFullPath);
        RuntimeDependencies.Add(ToFileFullPath); 
    }
    public FFmpeg(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		if (Target.Platform == UnrealTargetPlatform.Win64 ||
			Target.Platform == UnrealTargetPlatform.Win32)
		{
			string PlatformDir = Target.Platform.ToString();
			string IncPath = Path.Combine(ModuleDirectory, "include");
            {
                DirectoryInfo root = new DirectoryInfo(IncPath);
                DirectoryInfo[] IncludeDirs = root.GetDirectories();
                for(int i = 0;i<IncludeDirs.Length;i++)
                {
                    PublicSystemIncludePaths.Add(IncludeDirs[i].FullName);
                }
            }
			PublicSystemIncludePaths.Add(IncPath);

			string LibPath = Path.Combine(ModuleDirectory, "lib", PlatformDir);
			string BinaryPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../../Binaries/ThirdParty", PlatformDir));
            string dllPath = Path.Combine(ModuleDirectory, "dll", PlatformDir);

            {
                DirectoryInfo root = new DirectoryInfo(LibPath);
                FileInfo[] files = root.GetFiles();
                for (Int32 i = 0; i < files.Length; i++)
                {
                    FileInfo ItemFile = files[i]; 
                    PublicAdditionalLibraries.Add(Path.Combine(LibPath, ItemFile.Name));
                }
            }
            {
                DirectoryInfo root = new DirectoryInfo(dllPath);
                FileInfo[] files = root.GetFiles();
                for (Int32 i = 0; i < files.Length; i++)
                {
                    FileInfo ItemFile = files[i];
                    PublicDelayLoadDLLs.Add(ItemFile.Name);
                    CopyDllAndLibToProjectBinaries(Path.Combine(dllPath, ItemFile.Name), Target);
                }
            }
        }
	}

  
}
