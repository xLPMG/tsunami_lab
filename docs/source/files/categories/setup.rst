.. _setup:

Setup
=================

1. Clone the repository from GitHub
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code::

    git clone https://github.com/xLPMG/tsunami_lab.git

2. Initialize and update submodules
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code::

    git submodule init
    git submodule update

3. Install SCons & the GCC compiler
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- SCons: https://scons.org/doc/production/HTML/scons-user/ch01.html
- GCC: https://gcc.gnu.org/install/

On MacOS, you can use homebrew for both:

.. code-block::

    brew install scons
    brew install gcc

4. Build the project
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Navigate to the in step 1 created `tsunami_lab` folder and run

.. code::

    scons 

for Linux/Windows. If you are on MacOS, use

.. code::

    scons mode="osx"

For further run configurations view the SConstruct file at the root of the tsunami_lab directory.