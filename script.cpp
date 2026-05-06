#include "script.h"
#include "keyboard.h"
#include <string>

void PRINT_TEXT(char* text, int duration)
{
	UI::_SET_TEXT_ENTRY_2("STRING");
	UI::_ADD_TEXT_COMPONENT_STRING(text);
	UI::_DRAW_SUBTITLE_TIMED(duration, 1);
}

void setPlayerPropertiesEveryFrame(Player player) {
	PLAYER::SET_PLAYER_INVINCIBLE(player, TRUE);
	GAMEPLAY::SET_SUPER_JUMP_THIS_FRAME(player);
}

void flameForce(Ped playerPed, bool isUsingController, Player player) {
	char* dict = "core";
	GRAPHICS::_SET_PTFX_ASSET_NEXT_CALL(dict);
	int explosionParticle = GRAPHICS::START_PARTICLE_FX_LOOPED_ON_ENTITY(
		"fire_petroltank_truck",
		playerPed,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		2.0f,
		FALSE, FALSE, FALSE
	);

	int duration = 2000;

	int startTime = GAMEPLAY::GET_GAME_TIMER();
	while (TRUE) {
		WAIT(0);

		setPlayerPropertiesEveryFrame(player);

		int elapsedTime = GAMEPLAY::GET_GAME_TIMER();
		elapsedTime -= startTime;
		if (elapsedTime > duration) break;

		if (isUsingController) CONTROLS::DISABLE_CONTROL_ACTION(0, 37, TRUE);  // L1 (Weapon Wheel)
	}

	auto base = ENTITY::GET_ENTITY_COORDS(playerPed, TRUE);
	FIRE::ADD_EXPLOSION(base.x, base.y, base.z, ExplosionTypePlaneRocket, 1.0f, TRUE, FALSE, 3.0f);
	if (GRAPHICS::DOES_PARTICLE_FX_LOOPED_EXIST(explosionParticle)) {
		GRAPHICS::STOP_PARTICLE_FX_LOOPED(explosionParticle, FALSE);
	}
}

void throwFireBall(Ped playerPed, bool isUsingController, Player player) {
	Vector3 fireBallCoords = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(playerPed, 0.0f, 2.0f, -0.5f);

	STREAMING::REQUEST_MODEL(1840863642);
	STREAMING::LOAD_ALL_OBJECTS_NOW();
	do {
		WAIT(0);
	} while (!STREAMING::HAS_MODEL_LOADED(1840863642));

	char* animDict = "weapons@first_person@aim_idle@generic@assault_rifle@advanced_rifle@";
	char* animName = "aim_med_loop";
	if (STREAMING::DOES_ANIM_DICT_EXIST(animDict)) {
		STREAMING::REQUEST_ANIM_DICT(animDict);
		do {
			WAIT(0);
		} while (!STREAMING::HAS_ANIM_DICT_LOADED(animDict));

		AI::CLEAR_PED_TASKS(playerPed);
		AI::TASK_PLAY_ANIM(
			playerPed,
			animDict,
			animName,
			8.0f, -8.0f,
			500,
			49,
			1.0f,
			FALSE, FALSE, FALSE
		);
	}

	Object fireBall = OBJECT::CREATE_OBJECT(1840863642, fireBallCoords.x, fireBallCoords.y, fireBallCoords.z, FALSE, TRUE, TRUE);

	char* dict = "core";
	GRAPHICS::_SET_PTFX_ASSET_NEXT_CALL(dict);
	int fireBallParticle = GRAPHICS::START_PARTICLE_FX_LOOPED_ON_ENTITY(
		"ent_amb_torch_fire",
		fireBall,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		5.0f,
		FALSE, FALSE, FALSE
	);

	float travelSpeed = 100.0;
	float upwardForce = 0.0;

	float zAngle = ENTITY::GET_ENTITY_HEADING(playerPed);
	float xComponent = SYSTEM::SIN(zAngle);
	float yComponent = SYSTEM::COS(zAngle);
	xComponent *= travelSpeed;
	yComponent *= travelSpeed;
	xComponent *= -1.0;

	ENTITY::SET_ENTITY_VELOCITY(fireBall, xComponent, yComponent, upwardForce);

	int startTime = GAMEPLAY::GET_GAME_TIMER();

	while (TRUE) {
		WAIT(0);

		if (isUsingController) CONTROLS::DISABLE_CONTROL_ACTION(0, 37, TRUE);  // L1 (Weapon Wheel)

		setPlayerPropertiesEveryFrame(player);

		int elapsedTime = GAMEPLAY::GET_GAME_TIMER();
		elapsedTime -= startTime;

		// Disable gravity
		ENTITY::SET_ENTITY_VELOCITY(fireBall, xComponent, yComponent, upwardForce);

		if (ENTITY::HAS_ENTITY_COLLIDED_WITH_ANYTHING(fireBall) || elapsedTime > 1000) {
			Vector3 fireBallCoordsAfterCollision = ENTITY::GET_ENTITY_COORDS(fireBall, FALSE);
			FIRE::ADD_EXPLOSION(
				fireBallCoordsAfterCollision.x,
				fireBallCoordsAfterCollision.y,
				fireBallCoordsAfterCollision.z,
				ExplosionTypeGrenadeL,
				1.0f,
				TRUE,
				FALSE,
				1.5f
			);

			if (GRAPHICS::DOES_PARTICLE_FX_LOOPED_EXIST(fireBallParticle)) {
				GRAPHICS::STOP_PARTICLE_FX_LOOPED(fireBallParticle, FALSE);
			}

			break;
		}
	}

	OBJECT::DELETE_OBJECT(&fireBall);
	STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(1840863642);
	STREAMING::REMOVE_ANIM_DICT(animDict);
}

void fireSpark(Ped playerPed, bool isUsingController, Player player) {
	char* dict = "core";
	int headIndex = PED::GET_PED_BONE_INDEX(playerPed, SKEL_Head);
	const int numParticles = 5;

	char* animDict = "anim@mp_point";
	char* animName = "1st_person_med";
	int duration = 2000;
	if (STREAMING::DOES_ANIM_DICT_EXIST(animDict)) {
		STREAMING::REQUEST_ANIM_DICT(animDict);
		while (!STREAMING::HAS_ANIM_DICT_LOADED(animDict)) {
			WAIT(0);
		}
		AI::CLEAR_PED_TASKS(playerPed);
		AI::TASK_PLAY_ANIM(playerPed, animDict, animName, 8.0f, 1.0f, duration, 49, 1.0f, FALSE, FALSE, FALSE);
	}

	Vector3 headCoords = ENTITY::GET_WORLD_POSITION_OF_ENTITY_BONE(playerPed, headIndex);
	Vector3 forwardVec = ENTITY::GET_ENTITY_FORWARD_VECTOR(playerPed);
	for (int i = 0; i < numParticles; ++i) {
		float distance = 1.0f + ((float)i * 2.0f);
		float fx = headCoords.x + (forwardVec.x * distance);
		float fy = headCoords.y + (forwardVec.y * distance);
		float fz = headCoords.z;

		float groundZ;
		if (GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD(fx, fy, fz, &groundZ, FALSE)) {
			fz = groundZ;
		}
		else {
			fz = headCoords.z + (forwardVec.z * distance);
		}
				
		FIRE::ADD_EXPLOSION(
			fx, fy, fz,
			ExplosionTypeRocket,
			0.5f,
			(i == numParticles - 1) ? TRUE : FALSE,
			FALSE,
			2.0f
		);
	}
}

void fireGrenade(Ped playerPed, bool isUsingController, Player player) {
	STREAMING::REQUEST_MODEL(1840863642);
	STREAMING::LOAD_ALL_OBJECTS_NOW();
	do {
		WAIT(0);
	} while (!STREAMING::HAS_MODEL_LOADED(1840863642));

	char* animDict = "weapons@projectile@aim_throw_mg";
	char* animName = "aim_throw_m";
	if (STREAMING::DOES_ANIM_DICT_EXIST(animDict)) {
		STREAMING::REQUEST_ANIM_DICT(animDict);
		do {
			WAIT(0);
		} while (!STREAMING::HAS_ANIM_DICT_LOADED(animDict));

		AI::CLEAR_PED_TASKS(playerPed);
		AI::TASK_PLAY_ANIM(
			playerPed,
			animDict,
			animName,
			5.0f, 1.0f,
			500,
			49,
			1.0f,
			FALSE, FALSE, FALSE
		);
	}

	Vector3 fireGrenadeCoords = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(playerPed, 0.0f, 2.0f, 0.0f);
	Object fireGrenade = OBJECT::CREATE_OBJECT(1840863642, fireGrenadeCoords.x, fireGrenadeCoords.y, fireGrenadeCoords.z, FALSE, TRUE, TRUE);
	
	char* dict = "core";
	GRAPHICS::_SET_PTFX_ASSET_NEXT_CALL(dict);
	int fireGrenadeParticle = GRAPHICS::START_PARTICLE_FX_LOOPED_ON_ENTITY(
		"ent_amb_torch_fire",
		fireGrenade,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		5.0f,
		FALSE, FALSE, FALSE
	);

	float travelSpeed = 20.0;
	float upwardForce = 10.0;

	float zAngle = ENTITY::GET_ENTITY_HEADING(playerPed);
	float xComponent = SYSTEM::SIN(zAngle);
	float yComponent = SYSTEM::COS(zAngle);
	xComponent *= travelSpeed;
	yComponent *= travelSpeed;
	xComponent *= -1.0;

	ENTITY::SET_ENTITY_VELOCITY(fireGrenade, xComponent, yComponent, upwardForce);

	Vector3 fireGrenadeCurrentCoords;
	int startTime = GAMEPLAY::GET_GAME_TIMER();
	int lastFireDrop = 0;
	const int duration = 5000;
	const int interval = 200;
	const int MAX_FIRES = duration / interval + 10;
	int fireScriptsSpawned[MAX_FIRES];
	for (int i = 0; i < MAX_FIRES; ++i) {
		fireScriptsSpawned[i] = -1;
	}
	int fireScriptSpawnCounter = 0;
	while (TRUE) {
		WAIT(0);

		if (isUsingController) CONTROLS::DISABLE_CONTROL_ACTION(0, 37, TRUE);  // L1 (Weapon Wheel)

		setPlayerPropertiesEveryFrame(player);

		int elapsedTime = GAMEPLAY::GET_GAME_TIMER();
		elapsedTime -= startTime;

		if (elapsedTime > duration) break;

		fireGrenadeCurrentCoords = ENTITY::GET_ENTITY_COORDS(fireGrenade, FALSE);
		if (elapsedTime - lastFireDrop > interval) {
			int fireScript = FIRE::START_SCRIPT_FIRE(fireGrenadeCurrentCoords.x, fireGrenadeCurrentCoords.y, fireGrenadeCurrentCoords.z, 10, FALSE);
			fireScriptsSpawned[fireScriptSpawnCounter++] = fireScript;
			lastFireDrop = elapsedTime;
		}

		if (ENTITY::HAS_ENTITY_COLLIDED_WITH_ANYTHING(fireGrenade) || ENTITY::IS_ENTITY_IN_WATER(fireGrenade)) {
			FIRE::ADD_EXPLOSION(
				fireGrenadeCurrentCoords.x,
				fireGrenadeCurrentCoords.y,
				fireGrenadeCurrentCoords.z,
				ExplosionTypeGrenade,
				1.0f,
				FALSE,
				FALSE,
				1.5f
			);

			FIRE::ADD_EXPLOSION(
				fireGrenadeCurrentCoords.x + 3.0f,
				fireGrenadeCurrentCoords.y,
				fireGrenadeCurrentCoords.z,
				ExplosionTypeGrenade,
				1.0f,
				FALSE,
				FALSE,
				0.5f
			);

			FIRE::ADD_EXPLOSION(
				fireGrenadeCurrentCoords.x - 3.0f,
				fireGrenadeCurrentCoords.y,
				fireGrenadeCurrentCoords.z,
				ExplosionTypeGrenade,
				1.0f,
				FALSE,
				FALSE,
				0.5f
			);

			FIRE::ADD_EXPLOSION(
				fireGrenadeCurrentCoords.x + 6.0f,
				fireGrenadeCurrentCoords.y,
				fireGrenadeCurrentCoords.z,
				ExplosionTypeGrenade,
				1.0f,
				FALSE,
				FALSE,
				0.5f
			);

			FIRE::ADD_EXPLOSION(
				fireGrenadeCurrentCoords.x - 6.0f,
				fireGrenadeCurrentCoords.y,
				fireGrenadeCurrentCoords.z,
				ExplosionTypeGrenade,
				1.0f,
				FALSE,
				FALSE,
				0.5f
			);

			FIRE::ADD_EXPLOSION(
				fireGrenadeCurrentCoords.x + 9.0f,
				fireGrenadeCurrentCoords.y,
				fireGrenadeCurrentCoords.z,
				ExplosionTypeGrenade,
				1.0f,
				FALSE,
				FALSE,
				0.5f
			);

			FIRE::ADD_EXPLOSION(
				fireGrenadeCurrentCoords.x - 9.0f,
				fireGrenadeCurrentCoords.y,
				fireGrenadeCurrentCoords.z,
				ExplosionTypeGrenade,
				1.0f,
				FALSE,
				FALSE,
				0.5f
			);

			FIRE::ADD_EXPLOSION(
				fireGrenadeCurrentCoords.x,
				fireGrenadeCurrentCoords.y + 3.0f,
				fireGrenadeCurrentCoords.z,
				ExplosionTypeGrenade,
				1.0f,
				FALSE,
				FALSE,
				0.5f
			);

			FIRE::ADD_EXPLOSION(
				fireGrenadeCurrentCoords.x,
				fireGrenadeCurrentCoords.y - 3.0f,
				fireGrenadeCurrentCoords.z,
				ExplosionTypeGrenade,
				1.0f,
				FALSE,
				FALSE,
				0.5f
			);

			FIRE::ADD_EXPLOSION(
				fireGrenadeCurrentCoords.x,
				fireGrenadeCurrentCoords.y + 6.0f,
				fireGrenadeCurrentCoords.z,
				ExplosionTypeGrenade,
				1.0f,
				FALSE,
				FALSE,
				0.5f
			);

			FIRE::ADD_EXPLOSION(
				fireGrenadeCurrentCoords.x,
				fireGrenadeCurrentCoords.y - 6.0f,
				fireGrenadeCurrentCoords.z,
				ExplosionTypeGrenade,
				1.0f,
				FALSE,
				FALSE,
				0.5f
			);

			FIRE::ADD_EXPLOSION(
				fireGrenadeCurrentCoords.x,
				fireGrenadeCurrentCoords.y + 9.0f,
				fireGrenadeCurrentCoords.z,
				ExplosionTypeGrenade,
				1.0f,
				FALSE,
				FALSE,
				0.5f
			);

			FIRE::ADD_EXPLOSION(
				fireGrenadeCurrentCoords.x,
				fireGrenadeCurrentCoords.y - 9.0f,
				fireGrenadeCurrentCoords.z,
				ExplosionTypeGrenade,
				1.0f,
				TRUE,
				FALSE,
				0.5f
			);

			if (GRAPHICS::DOES_PARTICLE_FX_LOOPED_EXIST(fireGrenadeParticle)) {
				GRAPHICS::STOP_PARTICLE_FX_LOOPED(fireGrenadeParticle, FALSE);
			}
			for (int i = 0; i < fireScriptSpawnCounter; ++i) {
				FIRE::REMOVE_SCRIPT_FIRE(fireScriptsSpawned[i]);
			}

			break;
		}
	}

	OBJECT::DELETE_OBJECT(&fireGrenade);
	STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(1840863642);
	STREAMING::REMOVE_ANIM_DICT(animDict);
}

int rightHandParticle = 0;
int leftHandParticle = 0;
BOOL isFirebendingActive = FALSE;
BOOL wasInWater = FALSE;

void CleanupFirebending(Player player, Ped playerPed) {
	if (GRAPHICS::DOES_PARTICLE_FX_LOOPED_EXIST(rightHandParticle))
		GRAPHICS::STOP_PARTICLE_FX_LOOPED(rightHandParticle, FALSE);
	if (GRAPHICS::DOES_PARTICLE_FX_LOOPED_EXIST(leftHandParticle))
		GRAPHICS::STOP_PARTICLE_FX_LOOPED(leftHandParticle, FALSE);

	PLAYER::SET_PLAYER_INVINCIBLE(player, FALSE);
	isFirebendingActive = FALSE;
	wasInWater = FALSE;
}

bool shouldDisableFirebending(Player player, Ped playerPed) {
	return PED::IS_PED_DEAD_OR_DYING(playerPed, TRUE) ||
		PLAYER::IS_PLAYER_BEING_ARRESTED(player, TRUE) ||
		!PLAYER::IS_PLAYER_CONTROL_ON(player) ||
		CUTSCENE::IS_CUTSCENE_PLAYING();
}

bool canFirebend(bool currentlyInWater, Ped playerPed) {
	return !currentlyInWater &&
		!PED::IS_PED_RAGDOLL(playerPed) &&
		!PED::IS_PED_IN_ANY_VEHICLE(playerPed, TRUE) &&
		!PED::IS_PED_FALLING(playerPed) &&
		!PED::IS_PED_CLIMBING(playerPed) &&
		!PED::IS_PED_DIVING(playerPed) &&
		!PED::IS_PED_JUMPING(playerPed) &&
		!PED::IS_PED_IN_COVER(playerPed, TRUE);
}

void main() {
	WAIT(2000);
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	char* coreDict = "core";

	int lastInput = CONTROLS::_GET_LAST_INPUT_METHOD(0);
	bool isUsingController = (lastInput == 0);

	while (true) {
		WAIT(0);
		playerPed = PLAYER::PLAYER_PED_ID();

		if (!isFirebendingActive) {
			if (IsKeyJustUp('J') || (isUsingController && CONTROLS::IS_CONTROL_PRESSED(0, 37) && CONTROLS::IS_CONTROL_JUST_RELEASED(0, 24))) {
				isFirebendingActive = TRUE;
				STREAMING::REQUEST_NAMED_PTFX_ASSET(coreDict);
				while (!STREAMING::HAS_NAMED_PTFX_ASSET_LOADED(coreDict)) WAIT(0);
			}
		}
		else {
			if (IsKeyJustUp('J') || (isUsingController && CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, 37) && CONTROLS::IS_CONTROL_JUST_RELEASED(0, 24))) {
				CleanupFirebending(player, playerPed);
				continue;
			}
		}

		if (isFirebendingActive) {
			if (shouldDisableFirebending(player, playerPed)) {
				CleanupFirebending(player, playerPed);
				continue;
			}

			if (isUsingController) CONTROLS::DISABLE_CONTROL_ACTION(0, 37, TRUE);  // L1 (Weapon Wheel)

			setPlayerPropertiesEveryFrame(player);

			BOOL currentlyInWater = PED::IS_PED_SWIMMING(playerPed) || PED::IS_PED_SWIMMING_UNDER_WATER(playerPed);

			if (currentlyInWater != wasInWater || !GRAPHICS::DOES_PARTICLE_FX_LOOPED_EXIST(rightHandParticle)) {
				if (GRAPHICS::DOES_PARTICLE_FX_LOOPED_EXIST(rightHandParticle)) GRAPHICS::STOP_PARTICLE_FX_LOOPED(rightHandParticle, FALSE);
				if (GRAPHICS::DOES_PARTICLE_FX_LOOPED_EXIST(leftHandParticle)) GRAPHICS::STOP_PARTICLE_FX_LOOPED(leftHandParticle, FALSE);

				char* effectName = currentlyInWater ? "ent_amb_fbi_smoke_edge_lip" : "ent_amb_torch_fire";
				int rHand = PED::GET_PED_BONE_INDEX(playerPed, SKEL_R_Finger21);
				int lHand = PED::GET_PED_BONE_INDEX(playerPed, SKEL_L_Finger21);

				GRAPHICS::_SET_PTFX_ASSET_NEXT_CALL(coreDict);
				rightHandParticle = GRAPHICS::_START_PARTICLE_FX_LOOPED_ON_ENTITY_BONE(effectName, playerPed, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, rHand, 0.75f, FALSE, FALSE, FALSE);

				GRAPHICS::_SET_PTFX_ASSET_NEXT_CALL(coreDict);
				leftHandParticle = GRAPHICS::_START_PARTICLE_FX_LOOPED_ON_ENTITY_BONE(effectName, playerPed, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, lHand, 0.75f, FALSE, FALSE, FALSE);

				wasInWater = currentlyInWater;
			}

			if (canFirebend(currentlyInWater, playerPed)) {

				if (isUsingController && CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, 37)) {
					CONTROLS::DISABLE_CONTROL_ACTION(0, 21, TRUE);  // Cross (Sprint)
					CONTROLS::DISABLE_CONTROL_ACTION(0, 22, TRUE);  // Square (Jump)
					CONTROLS::DISABLE_CONTROL_ACTION(0, 23, TRUE);  // Triangle (Enter Car)
					CONTROLS::DISABLE_CONTROL_ACTION(0, 140, TRUE);  // Circle (Melee)

					if (CONTROLS::IS_DISABLED_CONTROL_JUST_RELEASED(0, 21)) flameForce(playerPed, isUsingController, player);
					if (CONTROLS::IS_DISABLED_CONTROL_JUST_RELEASED(0, 22)) throwFireBall(playerPed, isUsingController, player);
					if (CONTROLS::IS_DISABLED_CONTROL_JUST_RELEASED(0, 23)) fireSpark(playerPed, isUsingController, player);
					if (CONTROLS::IS_DISABLED_CONTROL_JUST_RELEASED(0, 140)) fireGrenade(playerPed, isUsingController, player);
				}

				if (IsKeyJustUp('T')) flameForce(playerPed, isUsingController, player);
				if (IsKeyJustUp('Y')) throwFireBall(playerPed, isUsingController, player);
				if (IsKeyJustUp('U')) fireSpark(playerPed, isUsingController, player);
				if (IsKeyJustUp('I')) fireGrenade(playerPed, isUsingController, player);
			}
		}
	}
}

void ScriptMain()
{
	srand(GetTickCount());
	main();
}
