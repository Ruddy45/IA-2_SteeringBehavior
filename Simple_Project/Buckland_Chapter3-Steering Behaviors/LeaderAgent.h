#ifndef LEADER_AGENT_H
#define LEADER_AGENT_H
//------------------------------------------------------------------------
//
//  Name:   LeaderAgent.h
//
//  Desc:   D�crit l'agent qui sera suivis par tout les autres (donne la direction)
//
//------------------------------------------------------------------------
#include "Vehicle.h" // Ajoute les donn�es de la classe Vehicule
#include "misc/cgdi.h"

class LeaderAgent : public Vehicle // H�rite de la classe Vehicle
{

private:

	//disallow the copying of LeaderAgent types
	LeaderAgent(const LeaderAgent&);
	LeaderAgent& operator=(const LeaderAgent&);


public:

	// Constructeur
	LeaderAgent(GameWorld* world,
		Vector2D position,
		double    rotation,
		Vector2D velocity,
		double    mass,
		double    max_force,
		double    max_speed,
		double    max_turn_rate,
		double    scale);

	~LeaderAgent();

	void        Update(double time_elapsed);

	void		Render();
};

#endif