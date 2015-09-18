/*
 * CORDIC.c
 *
 *  Created on: Nov 21, 2013
 *  Updated on: Dec 1, 2013
 *      Author: Michael Wong
 */
#include <msp430.h>
#include "CORDIC.h"


/*
 * plane definition
 * z+ is out of page, z- is into page
 *
 * 					*usb
 * 					x +
 * 					|
 * 		quad 2		|	quad 1
 * 					|
 * 	y+	------------|------------- y negative
 * 					|
 * 		quad 3		|	quad 4
 * 					|
 * 					x-
 * 					*button
 */


/*
 *
 * tangent lookup table
 * Each angle * 100
 *
 */
const unsigned int TANANGLES[] = {	//lookup table
		4500,					//tan(45) = 1
		2656,					//tan(26.565) = 0.5
		1403,					//tan(14.036) = 0.25
		712,					//tan(7.125) = 0.125
		357,					//tan(3.576) = 0.06
		179,					//tan(1.790) = 0.03
		89,						//tan(0.895) = 0.015
		45						//tan(0.448) = 0.007
};

/*
 * product sum of cosines scaled by 100
 */
const unsigned int COSSCALE[] = {
		71,			//cos(45) = .707
		63,			//cos(26.565) = .8944 ; cos(45)*cos(26.565)
		60,			//cos(14.036) = .9701; cos(45)*cos(26.565)*cos(14.036)
		61,			//cos(7.125) =.9922; etc...
		61,			//cos(3.576) = .998
		61,			//cos(1.790) = .9995
		61, 		//cos(0.895) = .9998
		61			//cos(0.448) = .9999
};

/*
 * gets xy angle and tilt to display
 * @param position pointer of coordinates
 * @return measurement struct that corresponds which xy angle and amount of tilt
 *
 */
measurement getDisplaySetting(coordinates* position) {
	unsigned int totalAngle;

	int hypotenuse;
	/*edge cases for x and y*/
	if(position->x == 0) {
		if(position->y < 0) {
			totalAngle = 9000;
			hypotenuse = -100*position->y;
		} else if(position->y > 0) {
			totalAngle = 27000;
			hypotenuse = 100*position->y;
		} else {
			hypotenuse = 0;
		}
	} else if(position->y == 0) {
		if(position->x < 0) {
			totalAngle = 18000;
			hypotenuse = -100*position->x;
		} else {
			totalAngle = 0;
			hypotenuse = 100*position->x;
		}
	} else {//make the x and y component positive to pass to findQuadrantAngle
		Quadrant quad = getQuad(position->x, position->y);
		if(quad == third || quad == fourth) {
			position->x = position->x * -1;//for coordinates in third and fourth quadrants make x positive for angle
		}
		if(quad == first || quad == fourth) {
			position->y = position->y * -1;//for coordinates in first and fourth quadrants make y positive for angle
		}

		calculations xyCalc = findQuadrantAngle(position->x, position->y); //gets angle between x and y, arctan(y/x)
		totalAngle = findTotalAngle(xyCalc.angle, quad); //gets angle between 0 - 36000

		hypotenuse = xyCalc.hypotenuse;
	}

	measurement angles;
	angles.angle = totalAngle;		//angle between 0 - 36000


	if(position->z == 0) {
		angles.tilt = 9000;
		return angles;
	} else if(position->z < 0) {	//make z component positive
		position->z = -1*position->z;
	}

	calculations tiltCalc = findQuadrantAngle(position->z*100, hypotenuse);	//gets angle of tilt, arctan(hypotenuse/z)
	angles.tilt = tiltCalc.angle;

	return angles;
}

/*
 * computes the angle between two coordinates, x must be POSITIVE
 * @param x first coordinate
 * @param y second coordinate
 * @return an instance of calculations struct,
 * 	calculations.angle = sumAngle the angle between the two points multiplied by 100. returns angles from -90 to 90, arctan
 *	calculations.hypotenuse = the hypotenuse between x and y
 */
calculations findQuadrantAngle(int x, int y){
	int sumAngle = 0;
	char loopNum = 0;
	int xnew = 0;
	int ynew = 0;
	calculations calc;

	while (y != 0 && loopNum < 8) {//"base case"
		if (y > 0) {
			xnew = x + (y >> loopNum);
			ynew = y - (x >> loopNum);
			sumAngle = sumAngle + TANANGLES[loopNum];
			loopNum++;
		}
		if (y < 0) {
			xnew = x - (y >> loopNum);
			ynew = y + (x >> loopNum);
			sumAngle = sumAngle - TANANGLES[loopNum];
			loopNum++;
		}

		y = ynew;	//set y to be passed back to while loop
		x = xnew;	//x is the hypotenuse
	}
	calc.hypotenuse = x * COSSCALE[loopNum-1];	//scale x by COSSCALE to get hypotenuse
	calc.angle = sumAngle;	//arc tan angle between -90 to 90
	return calc;
}

/*
 * gets quadrant the coordinate is in based on the sign of x and y in plane
 * @param x coordinate
 * @param y coordinate
 * @return Quadrant type (first, second, third, fourth)
 *
 */
Quadrant getQuad(int x, int y){
	if(x >= 0 && y >= 0) {
		return second;
	} else if (x <= 0 && y >= 0) {
		return third;
	} else if (x <= 0 && y <= 0) {
		return fourth;
	} else {
		return first;
	}
}

/*
 * returns angle from 0-36000
 * @param quadangle angle from 0-90
 * @param quad quadrant that the coordinate is in
 * @return integer from 0 -36000
 *
 */
unsigned int findTotalAngle(int quadAngle, Quadrant quad) {
	if(quad == first) {
		return quadAngle;		//first quadrant 0 - 9000
	} else if(quad == second) {
		return 36000 - quadAngle;	//second quadrant 27000 - 36000
	} else if(quad == third) {
		return 18000 + quadAngle;	//third quadrant 18000 - 27000
	} else {
		return 18000 - quadAngle;	//fourth quadrant 9000 - 18000
	}
}
