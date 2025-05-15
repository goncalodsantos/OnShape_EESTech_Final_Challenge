# OnShape_EESTech_Final_Challenge

## Documentation
Description:
	Our solution uses SimpleFOC PID velocity open loop motion control mode to control the BLDC motor with TLE5012 sensor (position and velocity control) in a smooth and stable way.

We Open/Close the gripper using a GUI instead of buttons. The GUI was built on the Processing software. The GUI has two buttons, that sends the commands "A" and "F".

- If the gripper receives "F", the gripper starts closing until it detects the object. When the gripper detects the object with the 3D TLx493D magnetic sensor data (measures the strength of a magnetic field on x-, y-, and z- axes), it verifies if the gripper is squeezing the object or is only grabbing it correctly (by correctly we mean not squeezing the object). If it starts squeezing, we stop the motor and the object is not squeezed . If we try to send one more time the command "F" trying to "force" closing one more time the gripper, it will not be able to close the gripper because the gripper knows that he is squeezing the object. 
- If the gripper receives "A", the gripper starts opening until he can´t more.

## Path During the Challenge
First we started using the SimpleFOC library and tested the different control types. Then we chosed to use the position (angle) in a closed loop and tuned it. Since most of all grippers started do don´t work and the only one working on the room was only working with open-loop we changed our approach to open-loop but it didn´t go well in start. Then we decided to used velocity open loop approach and it worked well in a first time.
On the second day of the hackathon, we enhanced our solution and we did a step by step debugging approach to understand what was failling. After a hard work day, we made it! Maybe not the best way but we made it work.

## Future Extra things:
The gripper starts closing initially with a base speed. When the gripper starts to detect the object, he slows down the closing speed because in that situation we need to take small steps to don´t make wrong moves.

## Teorical links used:
https://docs.simplefoc.com/position_control_example

https://github.com/Infineon/hackathon/tree/master
