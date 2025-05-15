# OnShape_EESTech_Final_Challenge

## Documentation
Description:
	Our solution uses SimpleFOC PID velocity open loop motion control mode to control the BLDC motor with TLE5012 sensor (position and velocity control) in a smooth and stable way.
We Open/Close the gripper using a GUI instead of buttons. The GUI was built on the Processing software.

- If the gripper receives "F", the gripper starts closing until it detects the object. When the gripper detects the object with the 3D TLx493D magnetic sensor data (measures the strength of a magnetic field on x-, y-, and z- axes), it verifies if the gripper is squeezing the object or is only grabbing it correctly (by correctly we mean not squeezing the object).


## Future Extra things:
Starts closing initially with a base speed. When the gripper starts to detect the object, he slows down the closing speed because in that situation we need to take small steps to don´t make wrong moves.


## Teorical links used:
https://docs.simplefoc.com/position_control_example
https://github.com/Infineon/hackathon/tree/master
