#ifndef CONSTANTS_H
#define CONSTANTS_H

// ─────────────────────────────────────────────────────────────────────────────
// Screen / Display
// ─────────────────────────────────────────────────────────────────────────────
#define SCREEN_WIDTH        128
#define SCREEN_HEIGHT       128

// ─────────────────────────────────────────────────────────────────────────────
// Tile / Map
// ─────────────────────────────────────────────────────────────────────────────
#define MAP_WIDTH           64
#define MAP_HEIGHT          64
#define TILE_SIZE           8
#define VIEWPORT_TILE_W     (SCREEN_WIDTH  / TILE_SIZE)         // 16
#define VIEWPORT_TILE_H     (SCREEN_HEIGHT / TILE_SIZE - 2)     // 14

// ─────────────────────────────────────────────────────────────────────────────
// Entity limits
// ─────────────────────────────────────────────────────────────────────────────
#define MAX_ENEMIES         30
#define MAX_PROJECTILES     30
#define MAX_PARTICLES       200
#define MAX_MELEE_TILES     20

// ─────────────────────────────────────────────────────────────────────────────
// Gameplay – player
// ─────────────────────────────────────────────────────────────────────────────
#define PLAYER_START_HP         100
#define PLAYER_START_FOOD       100
#define PLAYER_BASE_ATTACK      10
#define PLAYER_BASE_SPEED       0.1f
#define PLAYER_SPEED_PER_RING   0.05f
#define PLAYER_DIAG_FACTOR      0.7071f
#define PLAYER_MAX_HP           100

// ─────────────────────────────────────────────────────────────────────────────
// Gameplay – timing (in game ticks / frames at 20 ms each)
// ─────────────────────────────────────────────────────────────────────────────
#define FRAME_DELAY_MS          20UL

#define SPEED_TIMER_DEFAULT     500
#define SEE_ALL_TIMER_DEFAULT   1000
#define CONFUSION_TIMER_DEFAULT 1000
#define GLAMOUR_TIMER_DEFAULT   1000
#define BLINDNESS_TIMER_DEFAULT 700
#define PARALYSIS_TIMER_DEFAULT 1000
#define RIDICULE_DURATION       1000

#define HUNGER_TICK_NORMAL      700     // ticks between food drain when not starving
#define HUNGER_TICK_STARVING    70      // ticks between HP drain when starving
#define HUNGER_STARVE_DAMAGE    4
#define HUNGER_BASE_DRAIN       2

#define NOISE_DIFFUSE_TICKS     30      // ticks before ambientNoiseLevel decreases by 1

#define DAMSEL_HEAL_DELAY       200     // ticks between damsel carry heals
#define DAMSEL_WAIT_UP_COOLDOWN 200     // frames before "wait up" can repeat
#define DAMSEL_DIALOGUE_MIN     1000    // min ticks between damsel dialogue
#define DAMSEL_DIALOGUE_MAX     2000

#define REGEN_TICK_THRESHOLD    70      // ticks per regen ring HP tick
#define TELEPORT_TICK_THRESHOLD 130     // ticks per teleport ring warp

// ─────────────────────────────────────────────────────────────────────────────
// Gameplay – combat
// ─────────────────────────────────────────────────────────────────────────────
#define DEFAULT_ATTACK_DELAY        10
#define MELEE_DURATION_FRAMES       6
#define PROJECTILE_SPEED_PLAYER     0.5f
#define PROJECTILE_SPEED_ENEMY      0.25f
#define PROJECTILE_DAMAGE_ENEMY     4
#define STRENGTH_RING_DAMAGE_BONUS  5
#define WEAKNESS_RING_DAMAGE_BONUS  5   // NOTE: currently adds, not subtracts — keep same behaviour

// ─────────────────────────────────────────────────────────────────────────────
// Gameplay – dungeon
// ─────────────────────────────────────────────────────────────────────────────
#define BOSSFIGHT_LEVEL         12
#define MAX_ROOMS_MIN           15
#define MAX_ROOMS_MAX           25
#define MIN_ROOM_SIZE           4
#define MAX_ROOM_SIZE           8
#define CHEST_SPAWN_COUNT_MIN   2
#define CHEST_SPAWN_COUNT_MAX   5
#define ENEMY_SPAWN_MIN_DIST    10      // minimum tile distance from player when spawning
#define CHASE_RANGE_BASE        25      // gridDistSq threshold for enemy aggro
#define ENEMY_GIVE_UP_TICKS     600
#define REPEL_DISTANCE          0.5f
#define REPEL_STRENGTH          0.05f

// ─────────────────────────────────────────────────────────────────────────────
// Gameplay – boss
// ─────────────────────────────────────────────────────────────────────────────
#define BOSS_START_HP               400
#define BOSS_ENRAGE_HP              100
#define BOSS_PHASE_FLOATING         150     // bossStateTimer value for phase transitions
#define BOSS_PHASE_SHOOTING         1000
#define BOSS_PHASE_SUMMONING        1700
#define BOSS_PHASE_SHOOT2           2000
#define BOSS_PHASE_FLOAT2           2300
#define BOSS_CONTACT_DAMAGE_NORMAL  20
#define BOSS_CONTACT_DAMAGE_ENRAGED 40
#define BOSS_CONTACT_TICK           20      // every N ticks contact damage fires
#define BOSS_SHOOT_INTERVAL_FLOAT   100     // ticks between shots in Floating state
#define BOSS_SHOOT_INTERVAL_SHOOT   20      // ticks between shots in Shooting state
#define BOSS_SHOOT_INTERVAL_ENRAGE  10
#define BOSS_CHARGE_ALIGN_DOT       0.9f
#define BOSS_CHARGE_SPEED_MULT      2.5f
#define BOSS_CHARGE_SLOWDOWN_WALL   0.7f
#define BOSS_CHARGE_SLOWDOWN_MISS   0.9f
#define BOSS_CHARGE_STOP_SPEED      0.02f
#define BOSS_FLOAT_MOVE             0.05f
#define BOSS_ENRAGE_MOVE            0.1f
#define BOSS_SUMMON_INTERVAL        60      // ticks between minion spawns
#define BOSS_SUMMON_RADIUS_MIN      2
#define BOSS_SUMMON_RADIUS_MAX      5
#define BOSS_WANDER_RANGE_MIN       5
#define BOSS_WANDER_RANGE_MAX_X     (MAP_WIDTH  - 5)
#define BOSS_WANDER_RANGE_MAX_Y     (MAP_HEIGHT - 5)

// ─────────────────────────────────────────────────────────────────────────────
// Gameplay – inventory / items
// ─────────────────────────────────────────────────────────────────────────────
#define INVENTORY_SIZE          8
#define NUM_INVENTORY_PAGES     5
#define MAX_STACK_SIZE          7
#define NUM_POTIONS             20
#define NUM_SCROLLS             11
#define NUM_ITEMS               42
#define NUM_WEAPONS             9
#define NUM_RINGS               12
#define NUM_LORE_TEXTS          20
#define NUM_ITEM_COMBINATIONS   15
#define SHOP_MAX_ITEMS          7

// ─────────────────────────────────────────────────────────────────────────────
// Gameplay – audio
// ─────────────────────────────────────────────────────────────────────────────
#define MAX_AUDIO_DISTANCE      20.0f
#define MIN_AUDIO_VOLUME        0.01f
#define JUKEBOX_MAX_RADIUS      12.0f
#define JUKEBOX_NOISE_MAX       8
#define MASTER_VOLUME_DEFAULT   10
#define MASTER_VOLUME_MIN       1
#define MASTER_VOLUME_MAX       10

// ─────────────────────────────────────────────────────────────────────────────
// Gameplay – succubus pull
// ─────────────────────────────────────────────────────────────────────────────
#define SUCCUBUS_PULL_RANGE_SQ  40.0f
#define SUCCUBUS_PULL_STRENGTH  0.05f
// dialogue timer value that signals "succubus is talking" (was magic number 373)
#define SUCCUBUS_DIALOGUE_TIMER 373

// ─────────────────────────────────────────────────────────────────────────────
// UI / rendering
// ─────────────────────────────────────────────────────────────────────────────
#define UI_BOTTOM_BAR_Y         113
#define UI_BOTTOM_BAR_H         15
#define UI_TEXT_Y               117

#define SPLASH_LOOP_DURATION_MS     34350UL
#define SPLASH_TRANSITION_MS        100UL
#define SPLASH_SHAKE_FRAMES         6
#define SPLASH_SHAKE_MAGNITUDE      3
#define CREDITS_BRIGHTNESS_START    15
#define INTRO_MAX_FRAME             50
#define INTRO_MUSIC_CUTOFF          10      // introNum value at which we start checking if music ended

// ─────────────────────────────────────────────────────────────────────────────
// Hardware pins  (desktop build: these are dummies defined in Translation.h,
//                 but having them here keeps a single authoritative list)
// ─────────────────────────────────────────────────────────────────────────────
#define OLED_MOSI           11
#define OLED_CLK            13
#define OLED_DC             14
#define OLED_CS             10
#define OLED_RST            9
#define BUTTON_START_PIN    6
#define BUTTON_UP_PIN       2
#define BUTTON_DOWN_PIN     3
#define BUTTON_LEFT_PIN     4
#define BUTTON_RIGHT_PIN    5
#define BUTTON_B_PIN        1
#define BUTTON_A_PIN        0
#define SEED_PIN            A0

// ─────────────────────────────────────────────────────────────────────────────
// Pathfinding
// ─────────────────────────────────────────────────────────────────────────────
#define ASTAR_INF           999999
#define ASTAR_COST_STRAIGHT 10
#define ASTAR_COST_DIAGONAL 14
#define ASTAR_MAX_NODES     32      // max path nodes stored per enemy wander path

// ─────────────────────────────────────────────────────────────────────────────
// Puzzle
// ─────────────────────────────────────────────────────────────────────────────
#define PICROSS_SIZE        5
#define LIGHTSOUT_SIZE      5

// ─────────────────────────────────────────────────────────────────────────────
// Save / load
// ─────────────────────────────────────────────────────────────────────────────
#define SAVE_FILE_PATH      "savegame.dat"
#define SAVE_CHUNK_SIZE     256
#define SAVE_VERSION        4

// ─────────────────────────────────────────────────────────────────────────────
// Markov name generator
// ─────────────────────────────────────────────────────────────────────────────
#define MARKOV_LETTERS      26
#define NAME_BUFFER_SIZE    10
#define NAME_MIN_LEN        4

#endif // CONSTANTS_H