# create.py 
# Creates all projects from ../../src/tests/tools directory
#
import os,sys,re, shutil

env = {
  "sln"   : "../pdfedit.vc2008.sln",  # relative to SolutionDir
  "vcproj_dir":".",
  "guids"     : {}, # dictionary - [Project, GUID]
}

def gather_guids (env):
  """
    Returns and array of guids
  """
	#ProjectGUID="{F4B0B7E4-A405-4EB1-A74F-0765181FE3BC}"
  guidpattern = re.compile ("\"{(\S+-\S+-\S+-\S+-\S+)}\"")
  namepattern = re.compile ("Name=\"(\S+)\"")
  ret = ""; vcproj_dir = env["vcproj_dir"]; guids = {}
  for file in os.listdir(vcproj_dir):
    if file.endswith(".vcproj"):
      name = ""; guid = ""
      for l in open(os.path.join(vcproj_dir, file), "r+").readlines():
        m = guidpattern.search (l)
        if m: guid = m.group(1)
        m = namepattern.search (l)
        if m and name == "": name = m.group(1)
        if guid != "" and name != "": break
      guids.update ({name : guid.upper()})
  env["guids"] = guids
 
def update_sln (env):
  """
    Updates sln with projects from env["guids"]
  """
  lines = open(env["sln"], "r+").readlines()
  # split lines to projects, global section
  projects = []; globals = {}
  for i in range(0, len(lines)):
    if lines[i].strip() == "Global":
      projects = lines[:i]
      g = ""
      for l in lines [i+1:-2]:
        if l.strip().startswith("GlobalSection"):
          g = l.strip()
          globals.update ({g : []})
        else:
          if g == "":
            print "Invalid line in GlobalSection [%s]"%l
            sys.exit()
          if not "EndGlobalSection" in l:
            globals[g].append(l)

  # read projects
  # 	ProjectSection(ProjectDependencies) = postProject
  # 		{4E265089-FF11-4B4E-B0DA-7BFDEE750F9F} = {4E265089-FF11-4B4E-B0DA-7BFDEE750F9F}
  # 	EndProjectSection
  projectpattern = re.compile ('\s*Project\("{(\S+)}"\) = "(\S+)", "tools\\\\\S+.vcproj", "\{(\S+)\}"\s*')
  g_guid = ""; guid = ""; g_project_inards = []
  for i in range (0, len(projects)):
    l = projects[i]
    m = projectpattern.match(l)
    if m:
      g_guid = m.group(1).upper(); project = m.group(2); guid = m.group(3) 
      if project in env["guids"].keys():
        if guid != env["guids"][project]:
          print "Guid do not match [%s, %s]! Aborting..." % (guid, env["guids"][project])
          sys.exit()
        del env["guids"][project]
        print "Already present: %s"%project
        if len(g_project_inards) == 0:
          for j in range (i+1, len(projects)):
            if projects[j].strip() == "EndProject":
              break
            g_project_inards += projects[j]
      else:
        print "Unknown project found [%s], aborting..."%project
        sys.exit()

  # Update project section        
  print "Updating these projects:\n%s" % "\n".join(env["guids"].keys())
  for p,g in env["guids"].iteritems():
    lines = ['Project("{%s}") = "%s", "tools\%s.vcproj", "{%s}"\n' % (g_guid, p, p, g),
              "".join(g_project_inards),
              'EndProject\n']
    projects += lines
  
  # Update global 
  for p,g in env["guids"].iteritems():
    for k,v in globals.iteritems():
      for l in v:
        if guid in l:
          v.append (l.replace(guid,g))

  # bckp  
  shutil.copyfile (env["sln"], env["sln"]+".bckp")
  # store it
  lines = projects
  lines.append ("Global\n")
  for k,v in globals.iteritems():
      lines += ["\t%s\n" % k] + v + ["\tEndGlobalSection\n"]
  lines.append ("EndGlobal\n")
  open(env["sln"]+".test.sln", "w+").writelines(lines)

#
# main procedure
#
if __name__ == "__main__":
  gather_guids (env)
  update_sln (env)
  