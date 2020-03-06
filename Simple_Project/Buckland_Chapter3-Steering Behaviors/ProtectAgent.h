#ifndef PROTECT_AGENT_H
#define PROTECT_AGENT_H
//------------------------------------------------------------------------
//
//  Name:   ProtectAgent.h
//
//  Desc:   D�crit les agents qui prot�gent le leader des autres agents
//
//------------------------------------------------------------------------
#include "Vehicle.h" // Ajoute les donn�es de la classe Vehicule



class ProtectAgent : public Vehicle // H�rite de la classe Vehicle
{

private:

	//disallow the copying of ProtectAgent types
	ProtectAgent(const ProtectAgent&);
	ProtectAgent& operator=(const ProtectAgent&);


public:

	// Constructeur
	ProtectAgent(GameWorld* world,
		Vector2D position,
		double    rotation,
		Vector2D velocity,
		double    mass,
		double    max_force,
		double    max_speed,
		double    max_turn_rate,
		double    scale,
		Vector2D offset); // Offset servant � OffsetPursuit pour restre proche du leader

	~ProtectAgent();

	void        Update(double time_elapsed);

	void		Render();
};

#endif