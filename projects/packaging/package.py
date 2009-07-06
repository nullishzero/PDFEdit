# package.py 
# Creates win32 tools package for now
#
import os,sys,re,traceback,copy,subprocess,string

env = {
  "bin_dir" : "..\\output",
  "products"  : ["tools",],
  "platforms" : ["win32",],
}

#
# basic helpers
#
def run_command (cmd):
  """
    Run command from command line
  """
  #p = subprocess.call(cmd, shell=True)
  p = subprocess.Popen(cmd, shell=True).communicate()
  return p

def subst (env, line):
  """
    Subst var in lines with values in dict
  """
  return string.Template(line).safe_substitute(env)

def inherit (base, d):
  """
    Create new dict by inheritting from base dict and update it with new values
  """
  newd = copy.deepcopy (base)
  newd.update (d)
  return newd    
  
#
# base classes
#
class Execute:
  def __init__(self, d):
    self.__dict = d
  def exe (self,env):
    # split "cmd" from underlying dict, strip them and substitute with env
    cmds = map (lambda x: subst(inherit(env,self.__dict),x.strip()), self.__dict["cmd"].split("\n"))
    for cmd in cmds:
      run_command (cmd)

#
# building blocks
#
pack_tools = {
  "class" : "Execute",
  "cmd"   : """
              cd $bin_dir && 7z a -r *tool -x!*CVS* -o$output
              echo Done.  
            """,
}


#
# products
#
product = {
  "output"  : "",
  "platform": "",
  "start_dir": "",
  "bin_dir" : "",
  "do"      : [],
}

tools_product = inherit(product, {
  "do"  : [
            pack_tools,
          ]
})

#
# helper functions
#
def print_traceback ():
  print "-"*80
  traceback.print_exc(file=sys.stdout)
  print "-"*80
  
def create (env, what, where, platform="win32"):
  """
    create package and zip it to "where"
  """
  what += "_product"
  try:
    what = eval(what)
    what ["output"] = where 
    what ["platform"] = platform
    what ["start_dir"] = os.getcwd()
    what ["bin_dir"] = env["bin_dir"]
    # each class in do has to have class key, we create the class and give
    # do dict to ctor, call exe afterwards 
    for do in what["do"]:
      eval(do["class"])(inherit(what,do)).exe(env)
  except:
    print_traceback()
  

def parse_cmd ():
  """
    todo : checks, more reasonable
  """
  try:
    return [sys.argv[1].strip("--"),
            sys.argv[2].strip("--"),
            "win32"]
  except:
    print "Invalid cmd!"
    print "Example usage: package.py tools package.zip"
    print_traceback()
    sys.exit()


#
# main procedure
#
if __name__ == "__main__":
  what, where, platform = parse_cmd()
  create (env, what, where, platform)      
  