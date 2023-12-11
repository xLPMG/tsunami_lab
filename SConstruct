##
# @author Luca-Philipp Grumbach
# @author Richard Hofmann
#
# # Description
# Entry-point for builds.
##
import SCons
import os

print( '####################################' )
print( '### Tsunami Lab                  ###' )
print( '###                              ###' )
print( '### https://scalable.uni-jena.de ###' )
print( '####################################' )
print()
print('runnning build script')

# configuration
vars = Variables()

vars.AddVariables(
  EnumVariable( 'mode',
                'compile modes, option \'san\' enables address and undefined behavior sanitizers',
                'release',
                allowed_values=('release', 'debug', 'osx', 'release+san', 'debug+san', 'release+osx','osx+san')
              ),
  EnumVariable( 'opt',
                'optimization flag',
                '-O3',
                allowed_values=('-O0', '-O1', '-O2', '-O3', '-fast')
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
env = conf.Finish()

# set local env
env['ENV'] = os.environ

# choose compiler
if 'CXX' in os.environ:
  env['CXX'] = os.environ['CXX']

print("Using ", env['CXX'], " compiler.")

# enable filesystem for icpc
if env['CXX'] == "icpc":
   env.Append( CXXFLAGS = ["-lstdc++fs"] )

# generate help message
Help( vars.GenerateHelpText( env ) )

# add default flags
if 'osx' in env['mode']:
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


# set optimization mode
if 'debug' in env['mode']:
  env.Append( CXXFLAGS = [ '-g',
                           '-O0' ] )
else:
  env.Append( CXXFLAGS = [ env['opt'] ] )

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

# get source files
VariantDir( variant_dir = 'build/src',
            src_dir     = 'src' )

env.sources = []
env.tests = []
env.sanitychecks = []

Export('env')
SConscript( 'build/src/SConscript' )
Import('env')

env.Program( target = 'build/tsunami_lab',
             source = env.sources + env.standalone)

env.Program( target = 'build/tests',
             source = env.sources + env.tests)

env.Program( target = 'build/sanitychecks',
             source = env.sources + env.sanitychecks)