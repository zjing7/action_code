/*
 *  TPS.h
 *  
 *
 *  Created by Yael Elmatad on 6/2012
 *  Copyright 2012. All rights reserved.
 *
 */


#ifndef _TPS_H
#define	_TPS_H
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <float.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

#define MAXSIZE 10000

#include "Slice.h"
#include "Trajectory.h"
#include "Input.h"


enum ShootOrShift{
	SHOOT,
	SHIFT
};

enum Dir{
	FORW = 1,
	BACK = -1
};

/*enum OccState
 {
 OCCUPIED,
 UNOCCUPIED,
 };*/

using namespace std;

class TPS {
public:
    TPS(const Input &, double s); //overloaded const
	void TPS_move(const Trajectory &);
	void SetS(double S);
	//makes trajectory snippets.
//	void printTrajectory(int indicator) const;
	//some getters and setters
    //void setInfo(int _old, int _new,long currStep);
	
	//public variables
	//long int idum=-1; 
	
private:
	int m_n_slices;
	int m_n_slices_shift;
	double m_s;
	double m_shoot_frac;
	
	Input m_input;
	Slice tempSlice;
	
	
	
	int SlicesToRegenerate() const;	
	int ShootOrShift() const;
	int ForwardsOrBackwards() const;
	
	
	bool AcceptOrReject(double newOP, double oldOP) const;
	
	void ShootBackward(const Trajectory &myTraj, int nSlices) const;
	void ShootForward(const Trajectory &myTraj, int regenSlices) const;
	void ShiftBackward(const Trajectory &myTraj, int nslices) const;
	void ShiftForward(const Trajectory &myTraj, int nslices) const;

	//friend class Trajectory; 
};

#endif /*_TPS_H*/