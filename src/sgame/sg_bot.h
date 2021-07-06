/*
===========================================================================

Unvanquished GPL Source Code
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of the Unvanquished GPL Source Code (Unvanquished Source Code).

Unvanquished is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Unvanquished is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Unvanquished; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

===========================================================================
*/

#ifndef __BOT_HEADER
#define __BOT_HEADER

#define UNNAMED_BOT "[bot] Bot"

struct botEntityAndDistance_t
{
	gentity_t const *ent;
	float distance;
};

struct botTarget_t
{
	gentity_t const *ent;
	vec3_t coord;
	bool inuse;
	void SetTarget( gentity_t const* ent );
	void SetTarget( vec3_t pos );
	void SetTarget( void );
};

#define MAX_ENEMY_QUEUE 32
struct enemyQueueElement_t
{
	gentity_t *ent;
	int        timeFound;
};

struct enemyQueue_t
{
	enemyQueueElement_t enemys[ MAX_ENEMY_QUEUE ];
	int front;
	int back;
};

struct botSkill_t
{
	int level;
	float aimSlowness;
	float aimShake;
};

#include "sg_bot_ai.h"
#define MAX_NODE_DEPTH 20

struct botMemory_t
{
	enemyQueue_t enemyQueue;
	int enemyLastSeen;

	//team the bot is on when added
	team_t botTeam;

private:
	friend bool BotChangeGoal( gentity_t *self, botTarget_t target );
	botTarget_t m_goal;
public:
	botTarget_t const& goal( void ) const { return m_goal; }
	inline void SetTarget( gentity_t const* ent ) { m_goal.SetTarget( ent ); }
	inline void SetTarget( vec3_t pos ) { m_goal.SetTarget( pos ); }
	inline void SetTarget( void ) { m_goal.SetTarget(); }

	botSkill_t botSkill;
	botEntityAndDistance_t bestEnemy;
	botEntityAndDistance_t closestDamagedBuilding;
	botEntityAndDistance_t closestBuildings[ BA_NUM_BUILDABLES ];

	AIBehaviorTree_t *behaviorTree;
	AIGenericNode_t  *currentNode;
	AIGenericNode_t  *runningNodes[ MAX_NODE_DEPTH ];
	int              numRunningNodes;

	int         futureAimTime;
	int         futureAimTimeInterval;
	vec3_t      futureAim;
private:
	friend void G_BotThink( gentity_t *self );
	friend void BotWalk( gentity_t *self, bool enable );
	usercmd_t   m_cmdBuffer;
public:
	//sub-optimal solution to give bots a better press button API
	//such an enum should be provided by Daemon, and maybe it will
	//be propagated at some point, but for now let's try to improve
	//AI situation.
	//This might even allow to find a more robust solution for AIs
	//than pressing buttons explicitly.
	enum FakeButton
	{
		FB_BUTTON_ATTACK   = BUTTON_ATTACK,
		FB_BUTTON_ATTACK2  = BUTTON_ATTACK2,
		FB_BUTTON_ATTACK3  = BUTTON_ATTACK3,
		FB_BUTTON_WALKING  = BUTTON_WALKING,
		FB_BUTTON_SPRINT   = BUTTON_SPRINT,
		FB_BUTTON_GESTURE  = BUTTON_GESTURE,
		FB_BUTTON_ACTIVATE = BUTTON_ACTIVATE,
	};
	inline void PressButton( FakeButton btn ) { usercmdPressButton( m_cmdBuffer.buttons, btn ); }
	inline void ReleaseButton( FakeButton btn ) { usercmdReleaseButton( m_cmdBuffer.buttons, btn ); }
	inline bool ButtonPressed( FakeButton btn ) const { return usercmdButtonPressed( m_cmdBuffer.buttons, btn ); }
	inline void SetForwardSpeed( signed char speed ) { m_cmdBuffer.forwardmove = speed; }
	inline void SetLateralSpeed( signed char speed ) { m_cmdBuffer.rightmove = speed; }
	inline void SetVerticalSpeed( signed char speed ) { m_cmdBuffer.upmove = speed; }
	inline void ReverseLateralSpeed( void ) { m_cmdBuffer.rightmove = -m_cmdBuffer.rightmove; }
	inline void StopMoves( void ) { m_cmdBuffer.forwardmove = 0; m_cmdBuffer.rightmove = 0; m_cmdBuffer.upmove = 0; }
	inline void FireWeapon( weaponMode_t mode )
	{
		switch ( mode )
		{
			case WPM_PRIMARY:
				PressButton( FB_BUTTON_ATTACK );
				break;
			case WPM_SECONDARY:
				PressButton( FB_BUTTON_ATTACK2 );
				break;
			case WPM_TERTIARY:
				PressButton( FB_BUTTON_ATTACK3 );
				break;
			default:
				ASSERT( false && "Wrong mode used to FireWeapon" );
				break;
		}
	}
	inline void AimAt( vec3_t angle )
	{
		m_cmdBuffer.angles[0] = ANGLE2SHORT( angle[0] );
		m_cmdBuffer.angles[1] = ANGLE2SHORT( angle[1] );
		m_cmdBuffer.angles[2] = ANGLE2SHORT( angle[2] );
	}
	inline void AimAtPitch( short angle ) { m_cmdBuffer.angles[PITCH] = angle; }

	botNavCmd_t nav;

	int lastThink;
	int stuckTime;
	vec3_t stuckPosition;

	int spawnTime;
};

constexpr int BOT_DEFAULT_SKILL = 5;
const char BOT_DEFAULT_BEHAVIOR[] = "default";
const char BOT_NAME_FROM_LIST[] = "*";

bool G_BotAdd( const char *name, team_t team, int skill, const char *behavior, bool filler = false );
void G_BotChangeBehavior( int clientNum, const char* behavior );
bool G_BotSetBehavior( botMemory_t *botMind, const char* behavior );
bool G_BotSetDefaults( int clientNum, team_t team, int skill, const char* behavior );
void     G_BotDel( int clientNum );
void     G_BotDelAllBots();
void     G_BotThink( gentity_t *self );
void     G_BotSpectatorThink( gentity_t *self );
void     G_BotIntermissionThink( gclient_t *client );
void     G_BotListNames( gentity_t *ent );
bool G_BotClearNames();
int      G_BotAddNames(team_t team, int arg, int last);
void     G_BotDisableArea( vec3_t origin, vec3_t mins, vec3_t maxs );
void     G_BotEnableArea( vec3_t origin, vec3_t mins, vec3_t maxs );
void     G_BotInit();
void     G_BotCleanup();
void G_BotFill( bool immediately );
#endif
