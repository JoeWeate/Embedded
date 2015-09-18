/*
 * CORDIC.h
 *
 *  Created on: Nov 21, 2013
 *      Author: Michael Wong
 */

#ifndef CORDIC_H_
#define CORDIC_H_

typedef enum {
	first, second, third, fourth
}Quadrant;

typedef struct {
	int angle;
	int hypotenuse;
} calculations;

typedef struct {
	unsigned int angle;
	unsigned int tilt;
} measurement;

typedef struct {
	int x;
	int y;
	int z;

}coordinates;

measurement getDisplaySetting(coordinates* position);
calculations findQuadrantAngle(int x, int y);
Quadrant getQuad(int x, int y);
unsigned int findTotalAngle(int quadAngle, Quadrant quad);


#endif /* CORDIC_H_ */
