# create.py 
# Creates all projects from ../../src/tests/tools directory
#
import os,sys,re

env = {
  "tools_dir" : "../../src/tools",  # relative to SolutionDir
  "vcproj_dir":".",
  "guids"     : [],
  "exclude"   : ["common.cc",]
}

#
# helper functions
#

def gather_guids (env):
  """
  Returna array of guids
  """
	#ProjectGUID="{F4B0B7E4-A405-4EB1-A74F-0765181FE3BC}"
  guidpattern = re.compile ("\"{(\S+-\S+-\S+-\S+-\S+)}\"")
  ret = ""; vcproj_dir = env["vcproj_dir"]; guids = []
  for file in os.listdir(vcproj_dir):
    if file.endswith(".vcproj"):
      for l in open(os.path.join(vcproj_dir, file), "r+").readlines():
        m = guidpattern.search (l)
        if m:
          guids.append (m.group(1))
  guids.sort()
  env["guids"] = guids


def create_guid (env):
  """
  Returns new unique guid and appends it to env["guids"]
  """
  if len (env["guids"]) != 0:
    newguid = env["guids"][-1].split("-")
    newguid[-1] = hex(int(newguid[-1],16)+1).lstrip("0x").rstrip("L")
    newguid = "-".join(newguid)
    env["guids"].append (newguid.upper())
    env["guids"].sort()
    return newguid
  else:
    env["guids"].append ("F4B0B7E4-A405-4EB1-A74F-0765181FE3BC")
    return env["guids"][-1]

def create_files_section (env, files):
  """
  Returns string representing files section in vcproj
  """
  ret = ""
  for file in files:
    ret += """
    		<File
    			RelativePath="%s"
    			>
    		</File>
        """ % (file)
    return ret

def create_vcproj (env, project, guid, files, vsver = 9):
  """
  Return string representing vcproj
  """
  return  \
  """<?xml version="1.0" encoding="Windows-1252"?>
<VisualStudioProject
	ProjectType="Visual C++"
	Version="%s.00"
	Name="%s"
	ProjectGUID="{%s}"
	>
	<Platforms>
		<Platform
			Name="Win32"
		/>
		<Platform
			Name="WINCESDK_600 (ARMV4I)"
		/>
	</Platforms>
	<ToolFiles>
	</ToolFiles>
	<Configurations>
		<Configuration
			Name="Debug|Win32"
			ConfigurationType="1"
			InheritedPropertySheets="$(SolutionDir)/vsprops/base.vsprops;$(SolutionDir)/vsprops/tools.vsprops;$(SolutionDir)/vsprops/debug.vsprops;$(SolutionDir)/vsprops/win32.vsprops"
			>
		</Configuration>
		<Configuration
			Name="Release|Win32"
			ConfigurationType="1"
			InheritedPropertySheets="$(SolutionDir)/vsprops/base.vsprops;$(SolutionDir)/vsprops/tools.vsprops;$(SolutionDir)/vsprops/release.vsprops;$(SolutionDir)/vsprops/win32.vsprops"
			>
		</Configuration>
</Configurations>
	<References>
	</References>
	<Files>
  %s
	</Files>
	<Globals>
	</Globals>
</VisualStudioProject>
  """ % (vsver, project, guid, create_files_section(env, files))

#
# main procedure
#
if __name__ == "__main__":
  gather_guids (env)
  files = filter (lambda x: x.endswith(".cc") and not x in env["exclude"], os.listdir(env["tools_dir"]))
  # create [tool file, new vcproj, project name]
  files = map (lambda x: [os.path.join(env["tools_dir"], x),
                          os.path.join(env["vcproj_dir"], x.replace(".cc","")+".vcproj"),
                          x.replace(".cc","")], files)
  for ftool, fvcproj, project in filter (lambda x: not os.path.exists(x[1]), files):
    open(fvcproj, "w+").writelines(create_vcproj(env, project, create_guid(env), [ftool]))
    #print ftool, fvcproj,project
    #print create_vcproj(env, project, create_guid(env), [ftool])
      
  