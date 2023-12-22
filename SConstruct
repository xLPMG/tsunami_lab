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
                                '-qopt-report', 
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
  env.Append( CXXFLAGS = [ '-fopenmp' ] )
  env.Append( LINKFLAGS = [ '-fopenmp' ] )
if 'intel' in env['omp']:
  env.Append( CXXFLAGS = [ '-qopenmp' ] )
  env.Append( LINKFLAGS = [ '-qopenmp' ] )

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
# BENCHMARKING MODE #
#####################
if 'benchmark' in env['mode']:
  env.Append( CXXFLAGS =  [ '-DBENCHMARK' ] )

#####################
# CATCH 2 SUBMODULE #
#####################
env.Append( CXXFLAGS = [ '-isystem', 'submodules/Catch2/single_include' ] )

#####################
#  JSON  SUBMODULE  #
#####################
env.Append( CXXFLAGS = [ '-isystem', 'submodules/json/single_include' ] )

#####################
# GET SOURCE FILES  #
#####################
VariantDir( variant_dir = 'build/src',
            src_dir     = 'src' )

env.sources = []
env.tests = []
env.sanitychecks = []

Export('env')
SConscript( 'build/src/SConscript' )
Import('env')

#####################
#  SPECIFY TARGETS  #
#####################
env.Program( target = 'build/tsunami_lab',
             source = env.sources + env.standalone)

env.Program( target = 'build/tests',
             source = env.sources + env.tests)

env.Program( target = 'build/sanitychecks',
             source = env.sources + env.sanitychecks)