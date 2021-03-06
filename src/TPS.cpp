/*
 *  TPS.cpp
 *  Action
 *
 *  Created by Yael Elmatad on 06/2012
 *  Copyright 2012. All rights reserved.
 *
 */

#include "TPS.h"
#include <stdlib.h>
#include <math.h>
#include "Trajectory.h"

TPS::TPS()
{
	return;
}

TPS::TPS( const Input &myInput, int my_rank, int comm_sz)
{
	double startS = myInput.GetDoubleInput(D_START_S);
	double endS = myInput.GetDoubleInput(D_END_S);
	SetFirstS(startS, endS, my_rank, comm_sz);
	//cout << m_s << endl;
	m_input = myInput;
	m_n_slices_shift = myInput.GetIntInput(N_SLICES_SHIFT);
	m_n_slices = myInput.GetIntInput(N_SLICES);
	m_shoot_frac = myInput.GetDoubleInput(D_SHOOT_FRACTION);
		
}

void TPS::SetFirstS(double start_s, double end_s, int my_rank, int comm_sz)
{
	
	if (comm_sz == 1)
	{
		m_s = start_s;
		return;
	}
	else if (end_s > start_s)
	{	double range = end_s - start_s;
		double increment = range/(comm_sz-1);
		m_s = start_s + increment*my_rank;
		return;
	}
	else 
	{
		double range = start_s - end_s;
		double increment = range/(comm_sz-1);
		m_s = start_s - increment*my_rank;
		return;
	}
}

double TPS::GetCurrS() const
{
	return m_s;
}

void TPS::SetS(double tempS)
{
	m_s = tempS;
}


void TPS::TPS_move(Trajectory &myTraj)
{
	int shootOrShift = ShootOrShift();
	int direction = ForwardsOrBackwards();
	int slicesToRegen = SlicesToRegenerate();

	if (shootOrShift == SHOOT && direction == FORW)
		ShootForward(myTraj, slicesToRegen);
	else if (shootOrShift == SHOOT && direction == BACK)
		ShootBackward(myTraj, slicesToRegen);
	else if (shootOrShift == SHIFT && direction == FORW)
		ShiftForward(myTraj, slicesToRegen);
	else 
		ShiftBackward(myTraj, slicesToRegen);
	
}

bool TPS::AcceptOrReject(double newOP, double oldOP) const
{
	double boltz = exp(-(newOP-oldOP)*m_s);
	//cout << "boltz = " << boltz << endl;
	double randNum = ((double)rand()/(double)RAND_MAX);
	//cout << randNum << endl;
	if (randNum <= boltz)
	{
		return true;
	}
	else 
	{
		return false;
	}
}

void TPS::ShootForward(Trajectory &myTraj, int regenSlices) const
{
	int lastKept = m_n_slices - 1 - regenSlices;
	//last slice is m_n_slices -1.  

	//slices from first and last trajectory to "keep"
	//const Slice lastKeptSlice = myTraj.GetSlice(lastKept+1);
	const Slice lastKeptSlice = myTraj.GetSlice(lastKept);
	const Config lastConfig = lastKeptSlice.GetLastConfig();
	
	const Trajectory snippet(m_input, lastConfig, EDirection_FORWARD, regenSlices);
	
	double oldOrderParameter = (double)(myTraj.GetOrderParameter(lastKept+1,m_n_slices-1));
	double newOrderParameter = (double)(snippet.GetOrderParameter(0,regenSlices-1));
	//cout << oldOrderParameter << " " << newOrderParameter << endl;
	
	bool acceptance = AcceptOrReject(newOrderParameter, oldOrderParameter);
	if (acceptance)
	{
		//cout << "Trajectory Accepted! \n";
		myTraj.MergeTrajectories(snippet, lastKept+1, m_n_slices-1, ESide_END, EDirection_FORWARD);
	
	}
	else {
		//cout << "Trajectory Rejected!" << endl;
	}	
}

void TPS::ShootBackward( Trajectory &myTraj, int regenSlices) const
{
	int firstKept = regenSlices;
	
	int lastKept = m_n_slices - 1;
	//last slice is m_n_slices -1.  
	
	//slices from first and last trajectory to "keep"
	const Slice firstKeptSlice = myTraj.GetSlice(firstKept);
	const Config firstConfig = firstKeptSlice.GetFirstConfig();
	
	const Trajectory snippet(m_input, firstConfig, EDirection_BACKWARD, regenSlices);


	
	double oldOrderParameter = (double)(myTraj.GetOrderParameter(0,firstKept-1));
	double newOrderParameter = (double)(snippet.GetOrderParameter(0,regenSlices-1));
	//cout << oldOrderParameter << " " << newOrderParameter << endl;
	
	bool acceptance = AcceptOrReject(newOrderParameter, oldOrderParameter);
	if (acceptance)
	{
		//cout << "Trajectory Accepted! \n";
		myTraj.MergeTrajectories(snippet, 0, firstKept-1, ESide_BEGIN, EDirection_BACKWARD);
		
	}
	else {
		//cout << "Trajectory Rejected!" << endl;
	}
		
}

void TPS::ShiftBackward( Trajectory &myTraj, int regenSlices) const
{

	//shift backwards moves the beginning of a traj to the end and then regenerates
	
	int firstKept = 0;
	
	int lastKept = m_n_slices - 1 - regenSlices;
	//last slice is m_n_slices -1.  
	
	//slices from first and last trajectory to "keep"
	//const Slice lastKeptSlice = myTraj.GetSlice(lastKept+1);
	const Slice firstKeptSlice = myTraj.GetSlice(firstKept);
	const Config firstConfig = firstKeptSlice.GetFirstConfig();
	
	const Trajectory snippet(m_input, firstConfig, EDirection_BACKWARD, regenSlices);
	
	double oldOrderParameter = (double)(myTraj.GetOrderParameter(lastKept+1,m_n_slices-1));
	double newOrderParameter = (double)(snippet.GetOrderParameter(0,regenSlices-1));
	//cout << oldOrderParameter << " " << newOrderParameter << endl;
	
	bool acceptance = AcceptOrReject(newOrderParameter, oldOrderParameter);
	if (acceptance)
	{
		//cout << "Trajectory Accepted! \n";
		myTraj.MergeTrajectories(snippet, lastKept+1, m_n_slices-1, ESide_BEGIN, EDirection_BACKWARD);
		
	}
	else {
		//cout << "Trajectory Rejected!" << endl;
	}
	
}

void TPS::ShiftForward( Trajectory &myTraj, int regenSlices) const
{
	//moves end of traj to beginning and regenerates new end
	int firstKept = regenSlices;
	int lastKept = m_n_slices - 1;
	//last slice is m_n_slices -1.  
	
	//slices from first and last trajectory to "keep"
	const Slice lastKeptSlice = myTraj.GetSlice(lastKept);
	const Config lastConfig = lastKeptSlice.GetLastConfig();
	
	const Trajectory snippet(m_input, lastConfig, EDirection_FORWARD, regenSlices);
	
	double oldOrderParameter = (double)(myTraj.GetOrderParameter(0,firstKept-1));
	double newOrderParameter = (double)(snippet.GetOrderParameter(0,regenSlices-1));
	//cout << oldOrderParameter << " " << newOrderParameter << endl;
	
	bool acceptance = AcceptOrReject(newOrderParameter, oldOrderParameter);
	if (acceptance)
	{
	    //cout << "Trajectory Accepted! \n";
		myTraj.MergeTrajectories(snippet, 0, firstKept-1, ESide_END, EDirection_FORWARD);
		
	}
	else {
		//cout << "Trajectory Rejected!" << endl;
	}
}


int TPS::SlicesToRegenerate() const
{
	double rand1 =((double)rand()/((double)RAND_MAX+1));
	double randNum =rand1*(m_n_slices_shift-1);
	//doing this +1 thing ensures that the rand part returns a number between [0,N_SLICES_SHIFT)
	//thus adding one can and making an int would give the range [1,N_SLICES_SHIFT];
	return ((int)(randNum+1));
	
}


int TPS::ShootOrShift() const
{
	
	double randNum = ((double)rand()/(double)RAND_MAX);
	if (randNum < m_shoot_frac)
	{
		return SHOOT;
	}
	else 
	{
		return SHIFT;
	}
	
}

int TPS::ForwardsOrBackwards() const
{
	double randNum = ((double)rand()/(double)RAND_MAX);
	if (randNum < 0.5)
	{
		return FORW;
	}
	else 
	{
		return BACK;
	}

	
}
