#include "LeaderWithPlayer.h"

#include "2d/C2DMatrix.h"
#include "2d/Geometry.h"
#include "SteeringBehaviors.h"
#include "2d/Transformations.h"
#include "GameWorld.h"
#include "misc/CellSpacePartition.h"
#include "misc/cgdi.h"

using std::vector;
using std::list;


//----------------------------- ctor -------------------------------------
//------------------------------------------------------------------------
LeaderWithPlayer::LeaderWithPlayer(GameWorld* world,
	Vector2D position,
	double    rotation,
	Vector2D velocity,
	double    mass,
	double    max_force,
	double    max_speed,
	double    max_turn_rate,
	double    scale) : Vehicle(world,
		position,
		rotation,
		velocity,
		mass,
		max_force,
		max_speed,
		max_turn_rate,
		scale)
{
	m_pSteering->ArriveOn(); // Active par d�faut la poursuite de la target et son arriv� sur la cible
}


//---------------------------- dtor -------------------------------------
//-----------------------------------------------------------------------
LeaderWithPlayer::~LeaderWithPlayer()
{
	delete m_pSteering;
	delete m_pHeadingSmoother;
}

//------------------------------ Update ----------------------------------
//
// Update identique de la classe Vehicle, seul le comportement change
// 
//------------------------------------------------------------------------
void LeaderWithPlayer::Update(double time_elapsed)
{
	Vehicle::Update(time_elapsed);
}


//------------------------------ Render ----------------------------------
//
// Render pour donne la forme et la couleur du V�hicle
// 
//------------------------------------------------------------------------
void LeaderWithPlayer::Render()
{
	gdi->RedPen(); // Couleur du v�hicle

	static std::vector<Vector2D>  m_vecVehicleVBTrans;

	if (isSmoothingOn())
	{
		m_vecVehicleVBTrans = WorldTransform(m_vecVehicleVB,
			Pos(),
			SmoothedHeading(),
			SmoothedHeading().Perp(),
			Scale());
	}
	else
	{
		m_vecVehicleVBTrans = WorldTransform(m_vecVehicleVB,
			Pos(),
			Heading(),
			Side(),
			Scale());
	}


	gdi->ClosedShape(m_vecVehicleVBTrans);

	//render any visual aids / and or user options
	if (m_pWorld->ViewKeys())
	{
		Steering()->RenderAids();
	}
}