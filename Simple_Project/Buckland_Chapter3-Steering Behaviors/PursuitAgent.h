#ifndef PURSUIT_AGENT_H
#define PURSUIT_AGENT_H
//------------------------------------------------------------------------
//
//  Name:   PursuitAgent.h
//
//  Desc:   Décrit les agents poursuivant un leader (en file indienne)
//
//------------------------------------------------------------------------
#include "Vehicle.h" // Ajoute les données de la classe Vehicule



class PursuitAgent : public Vehicle // Hérite de la classe Vehicle
{

private:

	//disallow the copying of PursuitAgent types
	PursuitAgent(const PursuitAgent&);
	PursuitAgent& operator=(const PursuitAgent&);


public:

	// Constructeur
	PursuitAgent(GameWorld* world,
		Vector2D position,
		double    rotation,
		Vector2D velocity,
		double    mass,
		double    max_force,
		double    max_speed,
		double    max_turn_rate,
		double    scale,
		Vector2D offset); // Offset servant à OffsetPursuit

	~PursuitAgent();

	void        Update(double time_elapsed);

	void		Render();
};

#endif