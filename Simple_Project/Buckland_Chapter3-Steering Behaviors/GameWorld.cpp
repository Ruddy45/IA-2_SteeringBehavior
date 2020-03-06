#include "GameWorld.h"
#include "GameWorld.h"
#include "Vehicle.h"
#include "LeaderAgent.h" // Ajoute le leader
#include "LeaderWithPlayer.h" // Ajoute le leader guidé par le joueur
#include "PursuitAgent.h" // Ajoute les agents poursuiveurs
#include "ProtectAgent.h" // Ajoute les agents protègeant le leader
#include "constants.h"
#include "Obstacle.h"
#include "2d/Geometry.h"
#include "2d/Wall2D.h"
#include "2d/Vector2D.h" // Pour utiliser le multiplier d'offset pour protection d'agent
#include "2d/Transformations.h"
#include "SteeringBehaviors.h"
#include "time/PrecisionTimer.h"
#include "misc/Smoother.h"
#include "ParamLoader.h"
#include "misc/WindowUtils.h"
#include "misc/Stream_Utility_Functions.h"


#include "resource.h"

#include <list>
using std::list;


//------------------------------- ctor -----------------------------------
//------------------------------------------------------------------------
GameWorld::GameWorld(int cx, int cy):

            m_cxClient(cx),
            m_cyClient(cy),
            m_bPaused(false),
			m_vCrosshair(Vector2D(cxClient() / 2.0, cxClient() / 2.0)),
            m_bShowWalls(false),
            m_bShowObstacles(false),
            m_bShowPath(false),
            m_bShowWanderCircle(false),
            m_bShowSteeringForce(false),
            m_bShowFeelers(false),
            m_bShowDetectionBox(false),
            m_bShowFPS(true),
            m_dAvFrameTime(0),
            m_pPath(NULL),
            m_bRenderNeighbors(false),
            m_bViewKeys(false),
            m_bShowCellSpaceInfo(false),
			m_OffsetProtectAgent{Vector2D(0.0, 1.0), // Contient les multipliers pour l'offset des agents qui proègent le joueur
								 Vector2D(1.0, 1.0),
								 Vector2D(1.0, 0.0),
								 Vector2D(1.0, -1.0),
								 Vector2D(0.0, -1.0),
								 Vector2D(-1.0, -1.0),
								 Vector2D(-1.0, 0.0),
								 Vector2D(-1.0, 1.0)}
	        
{

  //setup the spatial subdivision class
  m_pCellSpace = new CellSpacePartition<Vehicle*>((double)cx, (double)cy, Prm.NumCellsX, Prm.NumCellsY, Prm.NumAgents);

  double border = 30;
  m_pPath = new Path(5, border, border, cx-border, cy-border, true);

  //-------------------------- Crée les Agents -----------------------------
  //------------------------------------------------------------------------
  
  //CreateLeaderWithPursuitAgent(1, Prm.NumAgents);
  CreateLeaderPlayerWithAgent(1, Prm.NumAgentProtectLeader, Prm.NumAgents);


  //create any obstacles or walls
  //CreateObstacles();
  //CreateWalls();
}


//-------------------------------- dtor ----------------------------------
//------------------------------------------------------------------------
GameWorld::~GameWorld()
{
  for (unsigned int a=0; a<m_Vehicles.size(); ++a)
  {
    delete m_Vehicles[a];
  }

  for (unsigned int ob=0; ob<m_Obstacles.size(); ++ob)
  {
    delete m_Obstacles[ob];
  }

  delete m_pCellSpace;
  
  delete m_pPath;
}


//----------------------------- Update -----------------------------------
//------------------------------------------------------------------------
void GameWorld::Update(double time_elapsed)
{ 
  if (m_bPaused) return;

  //create a smoother to smooth the framerate
  const int SampleRate = 10;
  static Smoother<double> FrameRateSmoother(SampleRate, 0.0);

  m_dAvFrameTime = FrameRateSmoother.Update(time_elapsed);
  

  //update the vehicles
  for (unsigned int a=0; a<m_Vehicles.size(); ++a)
  {
    m_Vehicles[a]->Update(time_elapsed);
  }
}


//--------------------------- CreateLeaderWithPursuitAgent ---------------
//
//  Crée le leader normal avec les agents qui le poursuivent
//	Forme une file indienne
//
//------------------------------------------------------------------------
void GameWorld::CreateLeaderWithPursuitAgent(int _minIndex, int _maxIndex)
{
	m_bLeaderPlayer = false;

	Vector2D SpawnPos = Vector2D(m_cxClient / 2.0 + RandomClamped()*m_cxClient / 2.0,
		m_cyClient / 2.0 + RandomClamped()*m_cyClient / 2.0);

	LeaderAgent* pLeaderAgent = new LeaderAgent(this,
		SpawnPos,                 //initial position
		RandFloat()*TwoPi,        //start rotation
		Vector2D(0, 0),           //velocity
		Prm.VehicleMass,          //mass
		Prm.MaxSteeringForce,     //max force
		Prm.LeaderMaxSpeed, //max velocity
		Prm.MaxTurnRatePerSecond, //max turn rate
		Prm.LeaderScale);        //scale, pour reconnaitre le leader

	// Ajoute le leader dans la liste des vehicules et sur le terrain
	m_Vehicles.push_back(pLeaderAgent);
	m_pCellSpace->AddEntity(pLeaderAgent);

	CreatePursuitAgent(_minIndex, _maxIndex);
}


//--------------------------- CreatePursuitAgent -------------------------------
//
//  Crée les agents poursuites de bases
//
//------------------------------------------------------------------------
void GameWorld::CreatePursuitAgent(int _minIndex, int _maxIndex)
{
	// Définie les agents poursuiveurs, a = 1 car il y a déjà le leader
	for (int a = _minIndex; a < _maxIndex; ++a)
	{
		Vector2D SpawnPos = Vector2D(m_cxClient / 2.0 + RandomClamped()*m_cxClient / 2.0,
			m_cyClient / 2.0 + RandomClamped()*m_cyClient / 2.0);

		PursuitAgent* pPursuitAgent = new PursuitAgent(this,
			SpawnPos,                 //initial position
			RandFloat()*TwoPi,        //start rotation
			Vector2D(0, 0),           //velocity
			Prm.VehicleMass,          //mass
			Prm.MaxSteeringForce,     //max force
			Prm.MaxSpeed,             //max velocity
			Prm.MaxTurnRatePerSecond, //max turn rate
			Prm.VehicleScale,         //scale
			Vector2D(Prm.OffsetAgent, Prm.OffsetAgent)); // Offset qui sépare de l'agent cible

		// Définie la target à suivre pour les agents poursuiveurs
		pPursuitAgent->Steering()->SetTargetAgent1(m_Vehicles[a - 1]);

		// Stocke le pointeur de l'agent crée
		m_Vehicles.push_back(pPursuitAgent);

		// Donne l'informations aux cases qu'il y a un nouvel agent
		m_pCellSpace->AddEntity(pPursuitAgent);
	}
}


//--------------------------- CreateLeaderPlayerWithAgent ---------------
//
//  Crée le leader guidé par le joueur avec les agents qui le protègent 
//	et les autres agents qui pousuivent le leader
//
//------------------------------------------------------------------------
void GameWorld::CreateLeaderPlayerWithAgent(int _minIndexProtect, int _maxIndexProtect, int _maxIndexPursuitAgent)
{
	m_bLeaderPlayer = true;

	Vector2D SpawnPos = Vector2D(m_cxClient / 2.0 + RandomClamped()*m_cxClient / 2.0,
		m_cyClient / 2.0 + RandomClamped()*m_cyClient / 2.0);

	LeaderWithPlayer* pLeaderWithPlayer = new LeaderWithPlayer(this,
		SpawnPos,                 //initial position
		RandFloat()*TwoPi,        //start rotation
		Vector2D(0, 0),           //velocity
		Prm.VehicleMass,          //mass
		Prm.MaxSteeringForce,     //max force
		Prm.LeaderPlayerMaxSpeed, //max velocity
		Prm.MaxTurnRatePerSecond, //max turn rate
		Prm.LeaderScale);        //scale, pour reconnaitre le leader

	// Ajoute le leader dans la liste des vehicules et sur le terrain
	m_Vehicles.push_back(pLeaderWithPlayer);
	m_pCellSpace->AddEntity(pLeaderWithPlayer);

	// Crée les agents de protections
	CreateProtectAgent(_minIndexProtect, _maxIndexProtect);

	// Crée les agents de poursuites (index contient déjà les ProtectAgent)
	CreatePursuitAgent(_maxIndexProtect + 1, _maxIndexPursuitAgent);

	for (unsigned int i = _maxIndexProtect + 1; i < _maxIndexPursuitAgent; ++i)
	{
		// Redéfinie la cible principale sur le leader, permet d'utiliser 
		// la même fonction de création en modifiant juste la target
		m_Vehicles[i]->Steering()->SetTargetAgent1(m_Vehicles[0]);
	}
}


//--------------------------- CreateProtectAgent -------------------------
//
//  Crée les agents de protection du leader
//
//------------------------------------------------------------------------
void GameWorld::CreateProtectAgent(int _minIndex, int _maxIndex)
{
	// Définie les agents poursuiveurs, a = 1 car il y a déjà le leader
	for (int a = _minIndex; a <= _maxIndex; ++a)
	{
		Vector2D SpawnPos = Vector2D(m_cxClient / 2.0 + RandomClamped()*m_cxClient / 2.0,
			m_cyClient / 2.0 + RandomClamped()*m_cyClient / 2.0);

		ProtectAgent* pProtectAgent = new ProtectAgent(this,
			SpawnPos,
			RandFloat()*TwoPi,
			Vector2D(0, 0),
			Prm.VehicleMass,
			Prm.MaxSteeringForce,
			Prm.ProtectLeaderMaxSpeed, // Vitesse supplémentaire sur les agents
			Prm.MaxTurnRatePerSecond,
			Prm.VehicleScale,
			Vector2D(Vector2D(Prm.OffsetAgent * m_OffsetProtectAgent[a - 1].x, // Offset qui sépare de l'agent cible
							  Prm.OffsetAgent * m_OffsetProtectAgent[a - 1].y)));

		// Définie la target à suivre pour les agents poursuiveurs
		pProtectAgent->Steering()->SetTargetAgent1(m_Vehicles[0]);

		// Stocke le pointeur de l'agent crée
		m_Vehicles.push_back(pProtectAgent);

		// Donne l'informations aux cases qu'il y a un nouvel agent
		m_pCellSpace->AddEntity(pProtectAgent);
	}
}


//--------------------------- CreateWalls --------------------------------
//
//  creates some walls that form an enclosure for the steering agents.
//  used to demonstrate several of the steering behaviors
//------------------------------------------------------------------------
void GameWorld::CreateWalls()
{
  //create the walls  
  double bordersize = 20.0;
  double CornerSize = 0.2;
  double vDist = m_cyClient-2*bordersize;
  double hDist = m_cxClient-2*bordersize;
  
  const int NumWallVerts = 8;

  Vector2D walls[NumWallVerts] = {Vector2D(hDist*CornerSize+bordersize, bordersize),
                                   Vector2D(m_cxClient-bordersize-hDist*CornerSize, bordersize),
                                   Vector2D(m_cxClient-bordersize, bordersize+vDist*CornerSize),
                                   Vector2D(m_cxClient-bordersize, m_cyClient-bordersize-vDist*CornerSize),
                                         
                                   Vector2D(m_cxClient-bordersize-hDist*CornerSize, m_cyClient-bordersize),
                                   Vector2D(hDist*CornerSize+bordersize, m_cyClient-bordersize),
                                   Vector2D(bordersize, m_cyClient-bordersize-vDist*CornerSize),
                                   Vector2D(bordersize, bordersize+vDist*CornerSize)};
  
  for (int w=0; w<NumWallVerts-1; ++w)
  {
    m_Walls.push_back(Wall2D(walls[w], walls[w+1]));
	m_vPointsWall.push_back(walls[w]);
  }

  m_vPointsWall.push_back(walls[NumWallVerts - 1]);

  m_Walls.push_back(Wall2D(walls[NumWallVerts-1], walls[0]));
}


//--------------------------- CreateObstacles -----------------------------
//
//  Sets up the vector of obstacles with random positions and sizes. Makes
//  sure the obstacles do not overlap
//------------------------------------------------------------------------
void GameWorld::CreateObstacles()
{
    //create a number of randomly sized tiddlywinks
  for (int o=0; o<Prm.NumObstacles; ++o)
  {   
    bool bOverlapped = true;

    //keep creating tiddlywinks until we find one that doesn't overlap
    //any others.Sometimes this can get into an endless loop because the
    //obstacle has nowhere to fit. We test for this case and exit accordingly

    int NumTrys = 0; int NumAllowableTrys = 2000;

    while (bOverlapped)
    {
      NumTrys++;

      if (NumTrys > NumAllowableTrys) return;
      
      int radius = RandInt((int)Prm.MinObstacleRadius, (int)Prm.MaxObstacleRadius);

      const int border                 = 10;
      const int MinGapBetweenObstacles = 20;

      Obstacle* ob = new Obstacle(RandInt(radius+border, m_cxClient-radius-border),
                                  RandInt(radius+border, m_cyClient-radius-30-border),
                                  radius);

      if (!Overlapped(ob, m_Obstacles, MinGapBetweenObstacles))
      {
        //its not overlapped so we can add it
        m_Obstacles.push_back(ob);

        bOverlapped = false;
      }

      else
      {
        delete ob;
      }
    }
  }
}


//------------------------- Set Crosshair ------------------------------------
//
//  The user can set the position of the crosshair by right clicking the
//  mouse. This method makes sure the click is not inside any enabled
//  Obstacles and sets the position appropriately
//------------------------------------------------------------------------
void GameWorld::SetCrosshair(POINTS p)
{
  Vector2D ProposedPosition((double)p.x, (double)p.y);

  //make sure it's not inside an obstacle
  for (ObIt curOb = m_Obstacles.begin(); curOb != m_Obstacles.end(); ++curOb)
  {
    if (PointInCircle((*curOb)->Pos(), (*curOb)->BRadius(), ProposedPosition))
    {
      return;
    }
  }

  if (m_bShowWalls) // Si les murs sont visibles
  {
	  // Boucle permettant de savoir si le clic d'un joueur est en dehors des murs ou non
	  for (int i = 0; i < m_vPointsWall.size(); ++i)
	  {
		  if (i + 1 != m_vPointsWall.size())
		  {
			  if (LineIntersection2D(ProposedPosition, Vector2D(cxClient() / 2.0, cyClient() / 2.0), m_vPointsWall[i], m_vPointsWall[i + 1]))
			  {
				  return;
			  }
		  }
		  else
		  {
			  if (LineIntersection2D(ProposedPosition, Vector2D(cxClient() / 2.0, cyClient() / 2.0), m_vPointsWall[i], m_vPointsWall[0]))
			  {
				  return;
			  }
		  }
	  }
  }

  m_vCrosshair.x = (double)p.x;
  m_vCrosshair.y = (double)p.y;
}


//------------------------- HandleKeyPresses -----------------------------
void GameWorld::HandleKeyPresses(WPARAM wParam)
{

  switch(wParam)
  {
	case 'U':
    {
      delete m_pPath;
      double border = 60;
      m_pPath = new Path(RandInt(3, 7), border, border, cxClient()-border, cyClient()-border, true); 
      m_bShowPath = true; 
      for (unsigned int i=0; i<m_Vehicles.size(); ++i)
      {
        m_Vehicles[i]->Steering()->SetPath(m_pPath->GetPath());
      }
    }
    break;


    case 'P':
      
      TogglePause();
	  break;


    case 'I':

      {
        for (unsigned int i=0; i<m_Vehicles.size(); ++i)
        {
          m_Vehicles[i]->ToggleSmoothing();
        }

      }

      break;


    case 'Y':

       m_bShowObstacles = !m_bShowObstacles;

        if (!m_bShowObstacles)
        {
          m_Obstacles.clear();

          for (unsigned int i=0; i<m_Vehicles.size(); ++i)
          {
            m_Vehicles[i]->Steering()->ObstacleAvoidanceOff();
          }
        }
        else
        {
          CreateObstacles();

          for (unsigned int i=0; i<m_Vehicles.size(); ++i)
          {
            m_Vehicles[i]->Steering()->ObstacleAvoidanceOn();
          }
        }
        break;


	case 'W': // Recrée un leader normal, pour la file indienne
		{
			DeleteArray();
			CreateLeaderWithPursuitAgent(1, Prm.NumAgents);
		}
		break;


	case 'X': // Cas d'un leader guidé par le joueur
		{
			DeleteArray();
			CreateLeaderPlayerWithAgent(1, Prm.NumAgentProtectLeader, Prm.NumAgents);
		}
		break;


	case 'C': // Augmente l'offset des agents
	{
		// i = 1 car le leader est 0 et n'a pas besoin d'offset
		for (unsigned int i = 1; i < m_Vehicles.size(); ++i)
		{
			m_Vehicles[i]->Steering()->SetOffset(m_Vehicles[i]->Steering()->GetOffset() * Prm.ChangeOffsetAgent); // Ancien offset multiplié par le nouveau
		}
	}
	break;


	case 'V': // Diminue l'offset des agents
	{
		// i = 1 car le leader est 0 et n'a pas besoin d'offset
		for (unsigned int i = 1; i < m_Vehicles.size(); ++i)
		{
			m_Vehicles[i]->Steering()->SetOffset(m_Vehicles[i]->Steering()->GetOffset() / Prm.ChangeOffsetAgent); // Ancien offset divisé par le nouveau
		}
	}

	break;


	case 'B': // Ajoute un agent poursuivant
	{
		CreatePursuitAgent(m_Vehicles.size(), m_Vehicles.size() + 1);

		if (isLeaderPlayer()) // Si le leader est guidé par le joueur, la cible devient le leader
		{
			m_Vehicles[m_Vehicles.size() - 1]->Steering()->SetTargetAgent1(m_Vehicles[0]);
			// Donne le même offset précédant, si on a déjà modifié l'offset
			m_Vehicles[m_Vehicles.size() - 1]->Steering()->SetOffset(m_Vehicles[m_Vehicles.size() - 2]->Steering()->GetOffset());
		}
	}

  }//end switch
}



//-------------------------- HandleMenuItems -----------------------------
void GameWorld::HandleMenuItems(WPARAM wParam, HWND hwnd)
{
  switch(wParam)
  {
    case ID_OB_OBSTACLES:

        m_bShowObstacles = !m_bShowObstacles;

        if (!m_bShowObstacles)
        {
          m_Obstacles.clear();

          for (unsigned int i=0; i<m_Vehicles.size(); ++i)
          {
            m_Vehicles[i]->Steering()->ObstacleAvoidanceOff();
          }

          //uncheck the menu
         ChangeMenuState(hwnd, ID_OB_OBSTACLES, MFS_UNCHECKED);
        }
        else
        {
          CreateObstacles();

          for (unsigned int i=0; i<m_Vehicles.size(); ++i)
          {
            m_Vehicles[i]->Steering()->ObstacleAvoidanceOn();
          }

          //check the menu
          ChangeMenuState(hwnd, ID_OB_OBSTACLES, MFS_CHECKED);
        }

       break;

    case ID_OB_WALLS:

      m_bShowWalls = !m_bShowWalls;

      if (m_bShowWalls)
      {
        CreateWalls();

        for (unsigned int i=0; i<m_Vehicles.size(); ++i)
        {
          m_Vehicles[i]->Steering()->WallAvoidanceOn();
        }

        //check the menu
         ChangeMenuState(hwnd, ID_OB_WALLS, MFS_CHECKED);
      }

      else
      {
        m_Walls.clear();
		m_vPointsWall.clear();

        for (unsigned int i=0; i<m_Vehicles.size(); ++i)
        {
          m_Vehicles[i]->Steering()->WallAvoidanceOff();
        }

        //uncheck the menu
         ChangeMenuState(hwnd, ID_OB_WALLS, MFS_UNCHECKED);
      }

      break;


    case IDR_PARTITIONING:
		{
			if (!isLeaderPlayer()) // Utilisé que dans la situation sans le leader guidé par le joueur
			{
				for (unsigned int i = 0; i < m_Vehicles.size(); ++i)
				{
					m_Vehicles[i]->Steering()->ToggleSpacePartitioningOnOff();
				}

				//if toggled on, empty the cell space and then re-add all the 
				//vehicles
				if (m_Vehicles[0]->Steering()->isSpacePartitioningOn())
				{
					m_pCellSpace->EmptyCells();

					for (unsigned int i = 0; i < m_Vehicles.size(); ++i)
					{
						m_pCellSpace->AddEntity(m_Vehicles[i]);
					}

					ChangeMenuState(hwnd, IDR_PARTITIONING, MFS_CHECKED);
				}
				else
				{
					ChangeMenuState(hwnd, IDR_PARTITIONING, MFS_UNCHECKED);
					ChangeMenuState(hwnd, IDM_PARTITION_VIEW_NEIGHBORS, MFS_UNCHECKED);
					m_bShowCellSpaceInfo = false;

				}
			}
		}

		 break;


    case IDM_PARTITION_VIEW_NEIGHBORS:
		{
			if (!isLeaderPlayer()) // Utilisé que dans la situation sans le leader guidé par le joueur
			{
				m_bShowCellSpaceInfo = !m_bShowCellSpaceInfo;

				if (m_bShowCellSpaceInfo)
				{
					ChangeMenuState(hwnd, IDM_PARTITION_VIEW_NEIGHBORS, MFS_CHECKED);

					if (!m_Vehicles[0]->Steering()->isSpacePartitioningOn())
					{
						SendMessage(hwnd, WM_COMMAND, IDR_PARTITIONING, NULL);
					}
				}
				else
				{
					ChangeMenuState(hwnd, IDM_PARTITION_VIEW_NEIGHBORS, MFS_UNCHECKED);
				}
			}
		}
      break;
        

    case IDR_WEIGHTED_SUM:
      {
        ChangeMenuState(hwnd, IDR_WEIGHTED_SUM, MFS_CHECKED);
        ChangeMenuState(hwnd, IDR_PRIORITIZED, MFS_UNCHECKED);
        ChangeMenuState(hwnd, IDR_DITHERED, MFS_UNCHECKED);

        for (unsigned int i=0; i<m_Vehicles.size(); ++i)
        {
          m_Vehicles[i]->Steering()->SetSummingMethod(SteeringBehavior::weighted_average);
        }
      }

      break;

    case IDR_PRIORITIZED:
      {
        ChangeMenuState(hwnd, IDR_WEIGHTED_SUM, MFS_UNCHECKED);
        ChangeMenuState(hwnd, IDR_PRIORITIZED, MFS_CHECKED);
        ChangeMenuState(hwnd, IDR_DITHERED, MFS_UNCHECKED);

        for (unsigned int i=0; i<m_Vehicles.size(); ++i)
        {
          m_Vehicles[i]->Steering()->SetSummingMethod(SteeringBehavior::prioritized);
        }
      }

      break;

    case IDR_DITHERED:
      {
        ChangeMenuState(hwnd, IDR_WEIGHTED_SUM, MFS_UNCHECKED);
        ChangeMenuState(hwnd, IDR_PRIORITIZED, MFS_UNCHECKED);
        ChangeMenuState(hwnd, IDR_DITHERED, MFS_CHECKED);

        for (unsigned int i=0; i<m_Vehicles.size(); ++i)
        {
          m_Vehicles[i]->Steering()->SetSummingMethod(SteeringBehavior::dithered);
        }
      }

      break;


      case ID_VIEW_KEYS:
      {
        ToggleViewKeys();

        CheckMenuItemAppropriately(hwnd, ID_VIEW_KEYS, m_bViewKeys);
      }

      break;


      case ID_VIEW_FPS:
      {
        ToggleShowFPS();

        CheckMenuItemAppropriately(hwnd, ID_VIEW_FPS, RenderFPS());
      }

      break;


      case ID_MENU_SMOOTHING:
      {
        for (unsigned int i=0; i<m_Vehicles.size(); ++i)
        {
          m_Vehicles[i]->ToggleSmoothing();
        }

        CheckMenuItemAppropriately(hwnd, ID_MENU_SMOOTHING, m_Vehicles[0]->isSmoothingOn());
      }

      break;


	  case ID_LEADER: // Cas d'un leader normal
	  {
		  ChangeMenuState(hwnd, ID_LEADER_PLAYER, MFS_UNCHECKED);
		  ChangeMenuState(hwnd, ID_LEADER, MFS_CHECKED);
		  
		  DeleteArray();
		  CreateLeaderWithPursuitAgent(1, Prm.NumAgents);
	  }

	  break;


	  case ID_LEADER_PLAYER: // Cas d'un leader guidé par le joueur
	  {
		  ChangeMenuState(hwnd, ID_LEADER, MFS_UNCHECKED);
		  ChangeMenuState(hwnd, ID_LEADER_PLAYER, MFS_CHECKED);

		  DeleteArray();
		  CreateLeaderPlayerWithAgent(1, Prm.NumAgentProtectLeader, Prm.NumAgents);
	  }

	  break;


	  case ID_OFFSET_MORE: // Augmente l'offset des agents
	  {
		  // i = 1 car le leader est 0 et n'a pas besoin d'offset
		  for (unsigned int i = 1; i < m_Vehicles.size(); ++i)
		  {
			  m_Vehicles[i]->Steering()->SetOffset(m_Vehicles[i]->Steering()->GetOffset() * Prm.ChangeOffsetAgent); // Ancien offset multiplié par le nouveau
		  }
	  }

	  break;


	  case ID_OFFSET_LESS: // Diminue l'offset des agents
	  {
		  // i = 1 car le leader est 0 et n'a pas besoin d'offset
		  for (unsigned int i = 1; i < m_Vehicles.size(); ++i)
		  {
			  m_Vehicles[i]->Steering()->SetOffset(m_Vehicles[i]->Steering()->GetOffset() / Prm.ChangeOffsetAgent); // Ancien offset divisé par le nouveau
		  }
	  }

	  break;


	  case ID_ADD_AGENT: // Ajoute un agent poursuivant
	  {
		  CreatePursuitAgent(m_Vehicles.size(), m_Vehicles.size() + 1);

		  if (isLeaderPlayer()) // Si le leader est guidé par le joueur, la cible devient le leader
		  {
			  m_Vehicles[m_Vehicles.size() - 1]->Steering()->SetTargetAgent1(m_Vehicles[0]);
			  // Donne le même offset précédant, si on a déjà modifié l'offset
			  m_Vehicles[m_Vehicles.size() - 1]->Steering()->SetOffset(m_Vehicles[m_Vehicles.size() - 2]->Steering()->GetOffset());
		  }
	  }

	  break;
      
  }//end switch
}


//------------------------------ Render ----------------------------------
//------------------------------------------------------------------------
void GameWorld::Render()
{
  gdi->TransparentText();

  //render any walls
  gdi->BlackPen();
  for (unsigned int w=0; w<m_Walls.size(); ++w)
  {
    m_Walls[w].Render(true);  //true flag shows normals
  }

  //render any obstacles
  gdi->BlackPen();
  
  for (unsigned int ob=0; ob<m_Obstacles.size(); ++ob)
  {
    gdi->Circle(m_Obstacles[ob]->Pos(), m_Obstacles[ob]->BRadius());
  }

  //render the agents
  for (unsigned int a=0; a<m_Vehicles.size(); ++a)
  {
    m_Vehicles[a]->Render();  
    
    //render cell partitioning stuff
    if (m_bShowCellSpaceInfo && a==0)
    {
      gdi->HollowBrush();
      InvertedAABBox2D box(m_Vehicles[a]->Pos() - Vector2D(Prm.ViewDistance, Prm.ViewDistance),
                           m_Vehicles[a]->Pos() + Vector2D(Prm.ViewDistance, Prm.ViewDistance));
      box.Render();

      gdi->RedPen();
      CellSpace()->CalculateNeighbors(m_Vehicles[a]->Pos(), Prm.ViewDistance);
      for (BaseGameEntity* pV = CellSpace()->begin();!CellSpace()->end();pV = CellSpace()->next())
      {
        gdi->Circle(pV->Pos(), pV->BRadius());
      }
      
      gdi->GreenPen();
      gdi->Circle(m_Vehicles[a]->Pos(), Prm.ViewDistance);
    }
  }  

//#define CROSSHAIR
#ifdef CROSSHAIR
  //and finally the crosshair
  gdi->RedPen();
  gdi->Circle(m_vCrosshair, 4);
  gdi->Line(m_vCrosshair.x - 8, m_vCrosshair.y, m_vCrosshair.x + 8, m_vCrosshair.y);
  gdi->Line(m_vCrosshair.x, m_vCrosshair.y - 8, m_vCrosshair.x, m_vCrosshair.y + 8);
  gdi->TextAtPos(5, cyClient() - 20, "Click to move crosshair");
#endif


  //gdi->TextAtPos(cxClient() -120, cyClient() - 20, "Press R to reset");

  gdi->TextColor(Cgdi::grey);
  if (RenderPath())
  {
     gdi->TextAtPos((int)(cxClient()/2.0f - 80), cyClient() - 20, "Press 'U' for random path");

     m_pPath->Render();
  }

  if (RenderFPS())
  {
    gdi->TextColor(Cgdi::grey);
    gdi->TextAtPos(5, cyClient() - 20, ttos(1.0 / m_dAvFrameTime));
  } 

  if (m_bShowCellSpaceInfo)
  {
    m_pCellSpace->RenderCells();
  }
}
