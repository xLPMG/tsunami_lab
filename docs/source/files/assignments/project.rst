###################
10. Project Phase
###################

In the project phase we decided to implement a user-friendly Gui. The aim was to make the usage of our Tsunami solver
as easy and interactive as possible. 

*********************
Preface
*********************

While we tried to stick to our plan as best as we could, we decided to do some things a little different.
The biggest difference being a Client-Server approach although we explicitly mentioned we would not realize this.
The original triggering point for this were all kinds of loading times and issues with concurrency: 
for any GUI application, one of the key features is its responsiveness to user input. Be it numerical input or just clicking and moving things around - the GUI should react as fast as possible.
Integrating a GUI directly into the solver application while maintaining this responsiveness meant that the GUI and every other task the actual solver does, need to be separated in different threads 
to assure that the GUI thread has always time for the user.

We started off with this directly integrated and threaded approach and quickly realized that handling correct communication between two or more asynchronous running threads was getting closer and closer to just having two separate applications communicating over an API.
Knowing that this approach meant we would have to invest a lot more effort and work overtime, we decided on taking this path nontheless as it would come with a lot of other benefits as well.
Now, using the Server-Client model we have a GUI that can just send commands to the server without needing to wait for a response (most of the time, unless you're specifically asking for data such as water/bathymetry data).

It can also be used in scenarios when using a more powerful machine (e.g. HPC-Cluster) without display access. Just start the program as a server on the remote machine and control it from home via laptop - its that easy.

*********************
GUI (Client-side)
*********************

*********************
Server-side
*********************

The first thing we did was to modularize our main ``tsunami_lab`` program. 

The original ``main.cpp`` had one main function that executed the whole program loop.
The code was moved to ``Simulator.cpp`` and we now have separate functions for the different parts of the program, such as ``prepareForCalculation()`` and ``runCalculation()``. 
``prepareForCalculation()`` itself also calls different functions, such as ``setupFolders()``, ``loadConfiguration()``, ``constructSetup()`` and so on.
We did this to have finer control, as we can now execute single parts of the program without having to run the whole simulation.

The next step was to implement a server that can be controlled via a GUI. The code for this can be found in ``Server.cpp``.
If the program is started with the ``server`` flag, it will start a server on the local machine and wait for connections. 
Once a connection is established, the server will wait for commands from the client and execute them.
To not iterate through all message keys to find the right one, a message contains extra information such as urgency and response expectation.

The actual simulation is started by sending a ``START_SIMULATION`` message to the server. 
We then use an ``std::thread`` to run the simulation in a separate thread, so the server can still receive commands while the simulation is running.
Note that only one simulation thread can be running at a time, but the GUI also provides an option to kill the current thread.

*********************
Build process
*********************

*********************
Libraries
*********************

To keep the main code clean, we decided to export most of the code associated with communication to external libraries:
the Communicator and the API. The code itself is already extensively documented, so we will only give a brief overview here.

.. note:: 
    
    The Communicator itself and the API do not depend on eachother, so the Communicator can be used without the API and vice versa.
    The Communicator provides functionality for sending text over TCP and the API provides a structure for the messages that are sent.
    That is why messages from the API have to converted to text before they can be sent.

Communicator
=====================

For communication between simulation and the GUI we implemented a communication library. 
The **Communicator.hpp** library can be used to easily create a client-server TCP connection and handle its communication and logging.
Both the server and client use the same library, but call different functions to initialize the connection and send messages.

There are also features such as different log messages with time stamps or automatic buffered sending if the message exceeds the buffer size.
All this code is hidden behind a simple interface, so the actual code stays clean and easy to read.

Communicator API
=====================

(**File: communicator_api.h**)

Since all communication happens using text over TCP, we had to implement a structure that both server and client can adhere to
in order to guarantee correct communication. For this, we decided to send all data in JSON format and a ``Message`` struct.


.. note:: For further information, see :ref:`ns-lib`