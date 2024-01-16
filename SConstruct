##
# @author Luca-Philipp Grumbach
# @author Richard Hofmann
#
# # Description
# Entry-point for builds.
##
import SCons
import platform
import os

print( '####################################' )
print( '### Tsunami Lab                  ###' )
print( '###                              ###' )
print( '### https://scalable.uni-jena.de ###' )
print( '####################################' )
print()
print('running build script')

#####################
#      GET OS       #
#####################
OS = platform.system()

#####################
#  READ ARGUMENTS   #
#####################
vars = Variables()

vars.AddVariables(
  EnumVariable( 'mode',
                'compile modes, option \'san\' enables address and undefined behavior sanitizers',
                'release',
                allowed_values=('release', 'debug', 'release+san', 'debug+san', 'benchmark')
              ),
  EnumVariable( 'opt',
                'optimization flag',
                '-O2',
                allowed_values=('-O0', 
                                '-O1', 
                                '-O2', 
                                '-O3', 
                                '-Ofast')
              ),
  EnumVariable( 'report',
                'flag for enabling reports',
                'none',
                allowed_values=('none', 
                                '-qopt-report=0', 
                                '-qopt-report=1', 
                                '-qopt-report=2',
                                '-qopt-report=3',
                                '-qopt-report=4',
                                '-qopt-report=5')
              ),
  EnumVariable( 'omp',
                'flag for enabling openmp',
                'none',
                allowed_values=('none', 
                                'gnu', 
                                'intel')
              ),
  EnumVariable( 'gui',
                'enables the GUI',
                'yes',
                allowed_values=('yes', 'no')
              ),
  EnumVariable( 'servermode',
                'starts the simulator in server mode',
                'yes',
                allowed_values=('yes', 'no')
              ),
  EnumVariable( 'use_filesystem',
                'enables or disabled the filesystem usage',
                'yes',
                allowed_values=('yes', 'no')
              )
)

# exit in the case of unknown variables
if vars.UnknownVariables():
  print( "build configuration corrupted, don't know what to do with: " + str(vars.UnknownVariables().keys()) )
  exit(1)

#####################
#  SET ENVIRONMENT  #
#####################
env = Environment( variables = vars )

# set local env
env['ENV'] = os.environ

#####################
#  COMPILER OPTION  #
#####################
if 'CXX' in os.environ:
  env['CXX'] = os.environ['CXX']
print("Using ", env['CXX'], " compiler.")

#####################
#  LOAD LIBRARIES   #
#####################

# macOS magic for finding libraries with gcc
if OS == "Darwin":
  env.Append( CXXFLAGS = [  '-I/usr/local/include/' ] )
  env.Append ( LIBPATH = [ '/usr/local/lib' ])

conf = Configure(env)

# NETCDF library
if not conf.CheckLibWithHeader('netcdf', 'netcdf.h','CXX'):
  print ('Did not find the c++ netcdf library, will try C.')
  if not conf.CheckLibWithHeader('netcdf', 'netcdf.h','C'):
    print ('Did not find the C netcdf library, exiting!')
    exit(1)

# GUI libraries
if 'yes' in env['gui']:
  if OS == "Linux":
    if not conf.CheckLib('glfw'):
      print ('Did not find the glfw library!')
      exit(1)
    elif not conf.CheckLib('GL'):
      print ('Did not find the GL library!')
      exit(1)

  elif OS == "Darwin":  
    if not conf.CheckLib('glfw'):
      print ('Did not find the glfw library!')
      exit(1)

  elif OS == "Windows":
    if not conf.CheckLib('glfw3'):
      print ('Did not find the glfw3 library!')
      exit(1)
    elif not conf.CheckLib('gdi32'):
      print ('Did not find the gdi32 library!')
      exit(1)
    elif not conf.CheckLib('opengl32'):
      print ('Did not find the opengl32 library!')
      exit(1)
    elif not conf.CheckLib('imm32'):
      print ('Did not find the imm32 library!')
      exit(1)

env = conf.Finish()

# generate help message
Help( vars.GenerateHelpText( env ) )

#####################
#   DEFAULT FLAGS   #
#####################
if OS == "Darwin":
  env.Append( CXXFLAGS = [ '-std=c++17',
                           '-Wall',
                           '-Wextra',
                           '-g' ] )
else:
  env.Append( CXXFLAGS = [ '-std=c++17',
                           '-Wall',
                           '-Wextra',
                           '-Werror',
                           '-Wpedantic',
                           '-g' ] )

#####################
# OPTIMIZATION MODE #
#####################
if 'debug' in env['mode']:
  env.Append( CXXFLAGS = [ '-g',
                           '-O0' ] )
else:
  env.Append( CXXFLAGS = [ env['opt'] ] )

#####################
#      REPORTS      #
#####################
if 'report' in env['report']:
   env.Append( CXXFLAGS = [ env['report'] ] )

#####################
#      OPENMP       #
#####################
if 'gnu' in env['omp']:
  if OS == "Darwin":
    env.Append( CXXFLAGS = [ '-Xpreprocessor', '-fopenmp' ] )
    env.Append( LINKFLAGS = [ '-Xpreprocessor', '-fopenmp' ] )
    env.Append( CXXFLAGS = [ '-DUSEOMP' ] )
  else:
    env.Append( CXXFLAGS = [ '-fopenmp' ] )
    env.Append( LINKFLAGS = [ '-fopenmp' ] )
    env.Append( CXXFLAGS = [ '-DUSEOMP' ] )
if 'intel' in env['omp']:
  env.Append( CXXFLAGS = [ '-qopenmp' ] )
  env.Append( LINKFLAGS = [ '-qopenmp' ] )
  env.Append( CXXFLAGS = [ '-DUSEOMP' ] )

if OS == "Darwin":
  env.Append( CXXFLAGS = [ '-I/usr/local/opt/libomp/include' ] )
  env.Append( LINKFLAGS = [ '-L/usr/local/opt/libomp/lib' ] )
  env.Append( LINKFLAGS = [ '-lomp' ] )
#####################
#    SANITIZERS    #
#####################
if 'san' in  env['mode']:
  env.Append( CXXFLAGS =  [ '-g',
                            '-fsanitize=float-divide-by-zero',
                            '-fsanitize=bounds',
                            '-fsanitize=address',
                            '-fsanitize=undefined',
                            '-fno-omit-frame-pointer' ] )
  env.Append( LINKFLAGS = [ '-g',
                            '-fsanitize=address',
                            '-fsanitize=undefined' ] )

#####################
#   NO FILESYSTEM   #
#####################
if 'no' in env['use_filesystem']:
  env.Append( CXXFLAGS =  [ '-DNOFILESYSTEM' ] )

env.Append( CXXFLAGS = [ '-isystem', 'lib' ] )
#####################
# CATCH 2 SUBMODULE #
#####################
env.Append( CXXFLAGS = [ '-isystem', 'submodules/Catch2/single_include' ] )

#####################
#  JSON  SUBMODULE  #
#####################
env.Append( CXXFLAGS = [ '-isystem', 'submodules/json/single_include' ] )

#####################
#  IMGUI SUBMODULE  #
#####################
if 'yes' in env['gui']:
  env.Append( CXXFLAGS = [ '-isystem', 'submodules/imgui/' ] )
  env.Append( CXXFLAGS = [ '-isystem', 'submodules/imgui/backends/' ] )
  # add other OS specific flags
  if OS == "Darwin": 
    env.AppendUnique(FRAMEWORKS=Split('OpenGL Cocoa IOKit CoreVideo'))

if 'no' in env['servermode']:
  env.Append( CXXFLAGS = [ '-DNOSERVER' ] )

#####################
# GET SOURCE FILES  #
#####################
VariantDir( variant_dir = 'build/src',
            src_dir     = 'src' )

env.sources = []
env.tests = []
env.sanitychecks = []
env.gui = []

Export('env')
SConscript( 'build/src/SConscript' )
Import('env')

if 'yes' in env['gui']:
  env.imguiSources = ['submodules/imgui/imgui.cpp',
                      'submodules/imgui/imgui_demo.cpp',
                      'submodules/imgui/imgui_draw.cpp',
                      'submodules/imgui/imgui_tables.cpp',
                      'submodules/imgui/imgui_widgets.cpp',
                      'submodules/imgui/backends/imgui_impl_glfw.cpp', 
                      'submodules/imgui/backends/imgui_impl_opengl3.cpp']

#####################
#  SPECIFY TARGETS  #
#####################

env.Program( target = 'build/tsunami_lab',
               source = env.sources + env.standalone )

env.Program( target = 'build/tests',
             source = env.sources + env.tests)

env.Program( target = 'build/sanitychecks',
             source = env.sources + env.sanitychecks)

if 'yes' in env['gui']:
  env.Program( target = 'build/gui',
               source = env.sources + env.gui + env.imguiSources )