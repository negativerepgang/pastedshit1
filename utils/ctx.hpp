#pragma once

#include "..\includes.hpp"

class player_t;
class CUserCmd;

class ctx_t {
public:
	struct Globals {
		float w2s_matrix[ 4 ][ 4 ];
		float hitalpha;
		vec3_t real_angles;
		vec3_t fake_angles;
		bool aimbotting;
		bool hitgroundanimation;

		bool fakelagging;
		bool fakewalking;
		bool should_choke_packets;

		int shots[ 65 ];
		int missed_shots[ 65 ];

		float next_update[65];
		float add_time[65];

		void add_shot( );
		void remove_shot( IGameEvent * event );
	}m_globals;

	bool available( );

	player_t * m_local;

	matrix3x4_t backtack_matrix[ 64 ][ 128 ];

	vec3_t unpred_velocity;

	bool send_packet;
	bool backtrack_chams;

	void set_command( CUserCmd * cmd ) { m_pcmd = cmd; }
	CUserCmd * get_command( );
private:
	CUserCmd * m_pcmd;
};

extern ctx_t g_ctx;