#ifndef GameWorld_H
#define GameWorld_H
#pragma warning (disable:4786)
//------------------------------------------------------------------------
//
//  Name:   GameWorld.h
//
//  Desc:   All the environment data and methods for the Steering
//          Behavior projects. This class is the root of the project's
//          update and render calls (excluding main of course)
//
//  Author: Mat Buckland 2002 (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
#include <windows.h>
#include <vector>

#include "2d/Vector2D.h"
#include "2D/Wall2D.h"
#include "time/PrecisionTimer.h"
#include "misc/CellSpacePartition.h"
#include "BaseGameEntity.h"
#include "EntityFunctionTemplates.h"
#include "vehicle.h"


class Obstacle;
class Wall2D;
class Path;


typedef std::vector<BaseGameEntity*>::iterator  ObIt;


class GameWorld
{ 
private:

  //a container of all the moving entities
  std::vector<Vehicle*>         m_Vehicles;

  //any obstacles
  std::vector<BaseGameEntity*>  m_Obstacles;

  //container containing any walls in the environment
  std::vector<Wall2D>           m_Walls;

  CellSpacePartition<Vehicle*>* m_pCellSpace;

  //any path we may create for the vehicles to follow
  Path*                         m_pPath;

  //set true to pause the motion
  bool                          m_bPaused;

  //local copy of client window dimensions
  int                           m_cxClient,
                                m_cyClient;
  //the position of the crosshair
  Vector2D                      m_vCrosshair;

  //keeps track of the average FPS
  double                        m_dAvFrameTime;

  // Contient le multiplier pour avoir l'offset
  // des agents qui protègent le joueur
  std::vector<Vector2D> m_OffsetProtectAgent;

  // Permet de savoir dans qu'elle mode on est
  bool							m_bLeaderPlayer;

  // Permet de savoir si le clic d'un joueur est en dehors des murs
  std::vector<Vector2D> m_vPointsWall;


  //flags to turn aids and obstacles etc on/off
  bool  m_bShowWalls;
  bool  m_bShowObstacles;
  bool  m_bShowPath;
  bool  m_bShowDetectionBox;
  bool  m_bShowWanderCircle;
  bool  m_bShowFeelers;
  bool  m_bShowSteeringForce;
  bool  m_bShowFPS;
  bool  m_bRenderNeighbors;
  bool  m_bViewKeys;
  bool  m_bShowCellSpaceInfo;


  // Correspond à la création d'un leader normal et des agents en fil indienne
  // Argument : Donne la position initiale du leader, minimum et maximum index pour crée les PursuitAgent
  void CreateLeaderWithPursuitAgent(const int _minIndex, const int _maxIndex);

  // Crée les agents de bases
  // Index pour pouvoir crée ces agents avec le leader normal ou guidé par le joueur (protect agent sont 
  // à déduire du nombre d'agent)
  void CreatePursuitAgent(const int _minIndex, const int _maxIndex);

  // Crée le leader guidé par le joueur, les agents qui le protègent et ceux que le poursuivent
  // Argument : Donne la position initiale du leader, minimum et maximum index pour crée les PursuitAgent et ProtectAgent
  void CreateLeaderPlayerWithAgent(const int _minIndexProtect, const int _maxIndexProtect, const int _maxIndexPursuitAgent);

  void CreateProtectAgent(const int _minIndex, const int _maxIndex);

  bool isLeaderPlayer() { return m_bLeaderPlayer; }

  void CreateObstacles();

  void CreateWalls();

  

public:

  GameWorld(int cx, int cy);

  ~GameWorld();

  void  Update(double time_elapsed);

  // Vide les tableaux des véhicules, murs et obstacles
  void DeleteArray()
  {
	  m_Vehicles.clear();
	  m_bShowWalls = false;
	  m_Walls.clear();
	  m_vPointsWall.clear();
	  m_bShowObstacles = false;
	  m_Obstacles.clear();
  }

  void  Render();


  void  NonPenetrationContraint(Vehicle* v){EnforceNonPenetrationConstraint(v, m_Vehicles);}

  void  TagVehiclesWithinViewRange(BaseGameEntity* pVehicle, double range)
  {
    TagNeighbors(pVehicle, m_Vehicles, range);
  }

  void  TagObstaclesWithinViewRange(BaseGameEntity* pVehicle, double range)
  {
    TagNeighbors(pVehicle, m_Obstacles, range);
  }

  const std::vector<Wall2D>&          Walls(){return m_Walls;}                          
  CellSpacePartition<Vehicle*>*       CellSpace(){return m_pCellSpace;}
  const std::vector<BaseGameEntity*>& Obstacles()const{return m_Obstacles;}
  const std::vector<Vehicle*>&        Agents(){return m_Vehicles;}


  //handle WM_COMMAND messages
  void        HandleKeyPresses(WPARAM wParam);
  void        HandleMenuItems(WPARAM wParam, HWND hwnd);
  
  void        TogglePause(){m_bPaused = !m_bPaused;}
  bool        Paused()const{return m_bPaused;}

  Vector2D    Crosshair()const{return m_vCrosshair;}
  void        SetCrosshair(POINTS p);
  void        SetCrosshair(Vector2D v){m_vCrosshair=v;}

  int   cxClient()const{return m_cxClient;}
  int   cyClient()const{return m_cyClient;}
 
  bool  RenderWalls()const{return m_bShowWalls;}
  bool  RenderObstacles()const{return m_bShowObstacles;}
  bool  RenderPath()const{return m_bShowPath;}
  bool  RenderDetectionBox()const{return m_bShowDetectionBox;}
  bool  RenderWanderCircle()const{return m_bShowWanderCircle;}
  bool  RenderFeelers()const{return m_bShowFeelers;}
  bool  RenderSteeringForce()const{return m_bShowSteeringForce;}

  bool  RenderFPS()const{return m_bShowFPS;}
  void  ToggleShowFPS(){m_bShowFPS = !m_bShowFPS;}
  
  void  ToggleRenderNeighbors(){m_bRenderNeighbors = !m_bRenderNeighbors;}
  bool  RenderNeighbors()const{return m_bRenderNeighbors;}
  
  void  ToggleViewKeys(){m_bViewKeys = !m_bViewKeys;}
  bool  ViewKeys()const{return m_bViewKeys;}

};



#endif