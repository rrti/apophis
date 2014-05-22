#ifndef _ENUMS_HDR_
#define _ENUMS_HDR_

enum {
	OWNER_NEUTRAL = 0,
	OWNER_ALLIED  = 1,
	OWNER_ENEMY   = 2,
	NUM_OWNERS    = 3,
};

inline unsigned int OWNER_OPPONENT(unsigned int owner) {
	if ((owner) == OWNER_ALLIED) { return OWNER_ENEMY;  }
	if ((owner) == OWNER_ENEMY ) { return OWNER_ALLIED; }
	return OWNER_NEUTRAL;
}

/*
enum {
	WEIGHT_DISTANCE   = 0,
	WEIGHT_NUMSHIPS   = 1,
	WEIGHT_GROWTHRATE = 2,
	NUM_WEIGHTS       = 3,
};
*/

enum {
	PLANET_CENTER_NEUTRAL_BIT = 1,
	PLANET_CENTER_ENEMY_BIT   = 2,
	PLANET_INNER_NEUTRAL_BIT  = 4, // neutral planets closer to OWNER_ALLIED than to OWNER_ENEMY
	PLANET_INNER_ENEMY_BIT    = 8, // enemy planets closer to OWNER_ENEMY than to OWNER_ALLIED
};
enum {
	PLANET_SAFETY_LEVEL_MIN = 0,
	PLANET_SAFETY_LEVEL_MID = 1,
	PLANET_SAFETY_LEVEL_MAX = 2,
};
enum {
	MAP_REGION_INNER  = 0, // position in the inner map-region quad (0.000 - 0.333)
	MAP_REGION_MIDDLE = 1, // position in the middle map-region quad (0.333 - 0.666)
	MAP_REGION_OUTER  = 2, // position in the outer map-region quad (0.666 - 0.999)
};

enum {
	SORTMODE_GROWTHRATE_DECR   =  0,
	SORTMODE_GROWTHRATE_INCR   =  1,
	SORTMODE_NUMSHIPS_DECR     =  2,
	SORTMODE_NUMSHIPS_INCR     =  3,
	SORTMODE_NUMSHIPSREM_DECR  =  4,
	SORTMODE_NUMSHIPSREM_INCR  =  5,
	SORTMODE_TMPVALUE_DECR     =  6,
	SORTMODE_TMPVALUE_INCR     =  7,
	SORTMODE_TMPDISTANCE_DECR  =  8,
	SORTMODE_TMPDISTANCE_INCR  =  9,
	SORTMODE_TMPTHREAT_DECR    = 10,
	SORTMODE_TMPTHREAT_INCR    = 11,
	SORTMODE_TMPROITIME_DECR   = 12,
	SORTMODE_TMPROITIME_INCR   = 13,
};

#endif
