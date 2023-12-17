##
# @author Luca-Philipp Grumbach
# @author Richard Hofmann
#
# # Description
# Entry-point for builds.
##
import SCons
import platform

print( '####################################' )
print( '### Tsunami Lab                  ###' )
print( '###                              ###' )
print( '### https://scalable.uni-jena.de ###' )
print( '####################################' )
print()
print('running build script')

# get OS
OS = platform.system()

# if a GUI can be built
buildGUI = False

# configuration
vars = Variables()

vars.AddVariables(
  EnumVariable( 'mode',
                'compile modes, option \'san\' enables address and undefined behavior sanitizers',
                'release',
                allowed_values=('release', 'debug', 'osx', 'release+san', 'debug+san', 'release+osx','osx+san')
              )
)

# exit in the case of unknown variables
if vars.UnknownVariables():
  print( "build configuration corrupted, don't know what to do with: " + str(vars.UnknownVariables().keys()) )
  exit(1)

# create environment
env = Environment( variables = vars )

conf = Configure(env)
if not conf.CheckLibWithHeader('netcdf', 'netcdf.h','CXX'):
        print ('Did not find the netcdf library, exiting!')
        Exit(1)

if OS == "Linux":
  if not conf.CheckLib('glfw'):
    print ('Did not find the glfw library!')
  elif not conf.CheckLib('GL'):
    print ('Did not find the GL library!')
  else: 
    buildGUI = True

elif OS == "Darwin":  
  if not conf.CheckLib('glfw'):
    print ('Did not find the glfw library!')
  else: 
    buildGUI = True

elif OS == "Windows":
  if not conf.CheckLib('glfw3'):
    print ('Did not find the glfw3 library!')
  elif not conf.CheckLib('gdi32'):
    print ('Did not find the gdi32 library!')
  elif not conf.CheckLib('opengl32'):
    print ('Did not find the opengl32 library!')
  elif not conf.CheckLib('imm32'):
    print ('Did not find the imm32 library!')
  else: 
    buildGUI = True

env = conf.Finish()

# generate help message
Help( vars.GenerateHelpText( env ) )

# add default flags
if 'osx' in env['mode']:
  env.Append( CXXFLAGS = [ '-std=c++17',
                         '-Wall',
                         '-Wextra' ] )
else:
  env.Append( CXXFLAGS = [ '-std=c++17',
                        '-Wall',
                        '-Wextra',
                        '-Werror',
                        '-Wpedantic' ] )


# set optimization mode
if 'debug' in env['mode']:
  env.Append( CXXFLAGS = [ '-g',
                           '-O0' ] )
else:
  env.Append( CXXFLAGS = [ '-O2' ] )

# add sanitizers
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

# add Catch2
env.Append( CXXFLAGS = [ '-isystem', 'submodules/Catch2/single_include' ] )

# add json
env.Append( CXXFLAGS = [ '-isystem', 'submodules/json/single_include' ] )

# add imgui
env.Append( CXXFLAGS = [ '-isystem', 'submodules/imgui/' ] )
env.Append( CXXFLAGS = [ '-isystem', 'submodules/imgui/backends/' ] )

# add other OS specific flags
if buildGUI and OS == "Darwin": 
  env.AppendUnique(FRAMEWORKS=Split('OpenGL Cocoa IOKit CoreVideo'))

# get source files
VariantDir( variant_dir = 'build/src',
            src_dir     = 'src' )

env.sources = []
env.tests = []
env.sanitychecks = []
env.gui = []

Export('env')
SConscript( 'build/src/SConscript' )
Import('env')

env.imguiSources = ['submodules/imgui/imgui.cpp',
                    'submodules/imgui/imgui_demo.cpp',
                    'submodules/imgui/imgui_draw.cpp',
                    'submodules/imgui/imgui_tables.cpp',
                    'submodules/imgui/imgui_widgets.cpp',
                    'submodules/imgui/backends/imgui_impl_glfw.cpp', 
                    'submodules/imgui/backends/imgui_impl_opengl3.cpp']

env.Program( target = 'build/tsunami_lab',
             source = env.sources + env.standalone)

env.Program( target = 'build/tests',
             source = env.sources + env.tests)

env.Program( target = 'build/sanitychecks',
             source = env.sources + env.sanitychecks)

if buildGUI:
  env.Program( target = 'build/gui',
               source = env.sources + env.gui + env.imguiSources)