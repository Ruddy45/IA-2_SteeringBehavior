#ifndef LEADER_WITH_PLAYER_H
#define LEADER_WITH_PLAYER_H
//------------------------------------------------------------------------
//
//  Name:   LeaderWithAgent.h
//
//  Desc:   Décrit l'agent qui sera suivis par tout les autres et guidé par le joueur.
//
//------------------------------------------------------------------------
#include "Vehicle.h" // Ajoute les données de la classe Vehicule
#include "misc/cgdi.h"


class LeaderWithPlayer : public Vehicle // Hérite de la classe Vehicle
{
private:

	//disallow the copying of LeaderAgent types
	LeaderWithPlayer(const LeaderWithPlayer&);
	LeaderWithPlayer& operator=(const LeaderWithPlayer&);


public:

	// Constructeur
	LeaderWithPlayer(GameWorld* world,
		Vector2D position,
		double    rotation,
		Vector2D velocity,
		double    mass,
		double    max_force,
		double    max_speed,
		double    max_turn_rate,
		double    scale);

	~LeaderWithPlayer();

	void        Update(double time_elapsed);

	void		Render();
};

#endif