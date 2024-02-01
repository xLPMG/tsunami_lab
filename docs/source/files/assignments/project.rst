###################
10. Project Phase
###################

In the project phase we decided to implement a userfriendly Gui. The aim is to make the usage of our Tsunami solver
as easy and interactive as possible. 

*********************
GUI (Client-side)
*********************

..  image:: ../../_static/assets/task-10-Gui_help.png

For the layout we chose to use multiple tabs, in order to make the use more clear.
The first page gives general informations about the project and refers to our website.

..  image:: ../../_static/assets/task-10-Gui_connectivity.png

Tab number two handles the the connection to thhe server. The properties for interacting with the server are getting set here. 
Furthermore, the connection can also be disconnected here.

..  image:: ../../_static/assets/task-10-Gui_windows.png

On the next page the user can find all options of configuration for the simulation. The simulation parameters like cell amount, size and offset can be set here.
In Addition to that, compiler and run time options can be found here. These contain differetn modes, compiler options, flags and usage choices. 
After selecting the simulation has to be recompiled with the according button below.

..  image:: ../../_static/assets/task-10-Gui_select.png

The last tab contains further actions to interact with the simulation. First, the simulation can be started or killed here.
Also files for the bathymetry and displacement can be chosen. As an addition, the user can get data like the heigth from the simulation. 

*********************
Server-side
*********************

*********************
Libraries
*********************

Communicator
=====================

For communication between simulation and the GUI we implemented a communication library. 
The **Communicator.hpp** library can be used to easily create a client-server TCP connection and handle its communication and logging.

Communicator API
=====================

(**File: communicator_api.h**)

Since all communication happens using text over TCP, we had to implement a structure that both server and client can adhere to
in order to guarantee correct communication. For this, we decided to send all data in JSON format and a ``Message`` struct.

.. note:: For further information, see :ref:`ns-lib`