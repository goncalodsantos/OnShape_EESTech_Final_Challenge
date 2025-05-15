# OnShape_EESTech_Final_Challenge

## Documentation
Description:
	Our solution uses SimpleFOC PID velocity open loop motion control mode to control the BLDC motor with TLE5012 sensor (position and velocity control) in a smooth and stable way.

We Open/Close the gripper using a GUI instead of buttons. The GUI was built on the Processing software. The GUI has two buttons, that sends the commands "A" and "F".

- If the gripper receives "F", the gripper starts closing until it detects the object. When the gripper detects the object with the 3D TLx493D magnetic sensor data (measures the strength of a magnetic field on x-, y-, and z- axes), it verifies if the gripper is squeezing the object or is only grabbing it correctly (by correctly we mean not squeezing the object). If it starts squeezing, we stop the motor and the object is not squeezed . If we try to send one more time the command "F" trying to "force" closing one more time the gripper, it will not be able to close the gripper because the gripper knows that he is squeezing the object. 
- If the gripper receives "A", the gripper starts opening until it doesn´t squeeze anymore.


## Future Extra things:
The gripper starts closing initially with a base speed. When the gripper starts to detect the object, he slows down the closing speed because in that situation we need to take small steps to don´t make wrong moves.

## Teorical links used:
https://docs.simplefoc.com/position_control_example

https://github.com/Infineon/hackathon/tree/master
