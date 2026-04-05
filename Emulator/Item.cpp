#include "Common.h"
#include "GameState.h"
#include "Item.h"
#include "Inventory.h"
#include "HelperFunctions.h"
#include "Player.h"

#include <cstring>
#include <cctype>

// ─────────────────────────────────────────────────────────────────────────────
// Item.cpp  — item data tables, randomisation, combination logic
// All logic identical to original; only includes have changed.
// ─────────────────────────────────────────────────────────────────────────────

// ── Items that live outside GameState (circular-dep workaround) ──────────────
GameItem equippedArmor   = {};
GameItem equippedWeapon  = {};
GameItem combiningItem1  = {};
GameItem combiningItem2  = {};

// ── Lore scrap texts ──────────────────────────────────────────────────────────
const char* loreTexts[NUM_LORE_TEXTS] = {
    "...er of this dungeon trapped down here, unable to esca...",
    "...more to scrolls than meets the untrained...",
    "...lie. Cake...",
    "...pin. Then shalt thou count to three, no more, no...",
    "...locked me in a room, a ru...",
    "...food, I shall die here if I...",
    "...nd so this will be my final resting pl...",
    "...at cursed washer... I can't get it off my fin...",
    "...is place, it never en...",
    "...t first his song was nice, I realized too la...",
    "...the light...",
    "...hopeless...",
    "...regret...",
    "...was a fool! Why di...",
    "...f what folly brought me to this place, I do not kn...",
    "...LL I EVER GET OU...",
    "...no way home, no w...",
    "...urn back if thou art readi...",
    "...nd yet it seems She was curious ab...",
    "...rhaps I could be Her frie...",
};

// ── itemList ──────────────────────────────────────────────────────────────────
GameItem itemList[] = {
  { RedPotion,       PotionCategory, "Red Potion",       0,0,0,0,0,"Drink it to find out.","Red Potion",       "Nothing happens."},
  { GreenPotion,     PotionCategory, "Green Potion",     0,0,0,0,0,"Drink it to find out.","Green Potion",     "Nothing happens."},
  { BluePotion,      PotionCategory, "Blue Potion",      0,0,0,0,0,"Drink it to find out.","Blue Potion",      "Nothing happens."},
  { BlackPotion,     PotionCategory, "Black Potion",     0,0,0,0,0,"Drink it to find out.","Black Potion",     "Nothing happens."},
  { WhitePotion,     PotionCategory, "White Potion",     0,0,0,0,0,"Drink it to find out.","White Potion",     "Nothing happens."},
  { YellowPotion,    PotionCategory, "Yellow Potion",    0,0,0,0,0,"Drink it to find out.","Yellow Potion",    "Nothing happens."},
  { OrangePotion,    PotionCategory, "Orange Potion",    0,0,0,0,0,"Drink it to find out.","Orange Potion",    "Nothing happens."},
  { PurplePotion,    PotionCategory, "Purple Potion",    0,0,0,0,0,"Drink it to find out.","Purple Potion",    "Nothing happens."},
  { CyanPotion,      PotionCategory, "Cyan Potion",      0,0,0,0,0,"Drink it to find out.","Cyan Potion",      "Nothing happens."},
  { MaroonPotion,    PotionCategory, "Maroon Potion",    0,0,0,0,0,"Drink it to find out.","Maroon Potion",    "Nothing happens."},
  { DarkGreenPotion, PotionCategory, "DarkGreen Potion", 0,0,0,0,0,"Drink it to find out.","DarkGreen Potion", "Nothing happens."},
  { LimePotion,      PotionCategory, "Lime Potion",      0,0,0,0,0,"Drink it to find out.","Lime Potion",      "Nothing happens."},
  { GreyPotion,      PotionCategory, "Grey Potion",      0,0,0,0,0,"Drink it to find out.","Grey Potion",      "Nothing happens."},
  { OlivePotion,     PotionCategory, "Olive Potion",     0,0,0,0,0,"Drink it to find out.","Olive Potion",     "Nothing happens."},
  { CreamPotion,     PotionCategory, "Cream Potion",     0,0,0,0,0,"Drink it to find out.","Cream Potion",     "Nothing happens."},
  { NavyPotion,      PotionCategory, "Navy Potion",      0,0,0,0,0,"Drink it to find out.","Navy Potion",      "Nothing happens."},
  { AzurePotion,     PotionCategory, "Azure Potion",     0,0,0,0,0,"Drink it to find out.","Azure Potion",     "Nothing happens."},
  { MintPotion,      PotionCategory, "Mint Potion",      0,0,0,0,0,"Drink it to find out.","Mint Potion",      "Nothing happens."},
  { SalmonPotion,    PotionCategory, "Salmon Potion",    0,0,0,0,0,"Drink it to find out.","Salmon Potion",    "Nothing happens."},
  { BrownPotion,     PotionCategory, "Brown Potion",     0,0,0,0,0,"Drink it to find out.","Brown Potion",     "Nothing happens."},
  { Mushroom,  FoodCategory,"Mushroom",  0,20,0,0,0,"It is edible.",                              "Mushroom",  "You become less hungry.",                          1},
  { Bread,     FoodCategory,"Bread",     0,30,0,0,0,"Old bread. How long has it been down here?",                          "Bread",     "You eat the bread. It tastes stale.",       2},
  { Cheese,    FoodCategory,"Cheese",    0,15,0,0,0,"Smells terrible, but some people like that.",             "Cheese",    "You eat the cheese, it tastes quite nice.", 2},
  { StaleMeat, FoodCategory,"Stale Meat",-5,40,0,0,0,"Should be fine. Should be.",                  "Stale Meat","You eat the meat. It tastes terrible.",         2,true,StaleMeatEffect},
  { Berries,   FoodCategory,"Berries",   0,10,0,0,0,"A sweet treat.", "Berries",   "You eat the berries, and remember what nice things taste like.",       4},
  { EmptyBottle,   PotionCategory,   "Empty Bottle",    0,0,0,0,0,"It is an empty bottle.",                                              "Empty Bottle",   "Nothing happens. It's an empty bottle.",          4,false},
  { RiddleStone,   EquipmentCategory,"Riddle Stone",    0,0,0,0,0,"Looks like it could be used for many things...",                      "Riddle Stone",   "Solve this riddle!",                              5,true,DefaultEffect,0,false,false,2,false},
  { Scroll,        ScrollsCategory,  "Scroll",          0,0,0,0,0,"Read it to find out.",                                                "Scroll",         "Nothing happens.",                                4},
  { WetScroll,     ScrollsCategory,  "Wet Scroll",      0,0,0,0,0,"A scroll that is too wet to read.",                                   "Wet Scroll",     "The scroll is too wet to read. Nothing happens.", 3,false},
  { Ring,          EquipmentCategory,"Ring",            0,0,0,0,0,"Put it on to find out.",                                              "Ring",           "You equip the ring.",                             3,false},
  { LeatherArmor,  EquipmentCategory,"Leather Armor",   0,0,0,0,0,"Basic leather armor. Reduces damage taken.",                          "Leather Armor",  "You equip the leather armor.",                    3,false,ArmorEffect, 2,false,false,2,false},
  { IronArmor,     EquipmentCategory,"Iron Armor",      0,0,0,0,0,"Sturdy iron armor. Reduces damage taken.",                            "Iron Armor",     "You equip the iron armor.",                       4,false,ArmorEffect, 4,false,false,2,true},
  { MagicRobe,     EquipmentCategory,"Magic Robe",      0,0,0,0,0,"Enchanted robe. Reduces damage and magic resistance.",                "Magic Robe",     "You equip the magic robe.",                       4,false,ArmorEffect, 1,false,false,2,false},
  { Cloak,         EquipmentCategory,"Cloak",           0,0,0,0,0,"A simple cloth cloak. Provides no protection but keeps you warm.",    "Cloak",          "You equip the cloak.",                            4,false,ArmorEffect, 0,false,false,2,false},
  { Trenchcoat,    EquipmentCategory,"Trenchcoat",      0,0,0,0,0,"A trenchcoat and fedora. Provides no protection but looks cool.",     "Trenchcoat",     "You equip the trenchcoat.",                       3,false,ArmorEffect, 0,false,false,2,false},
  { DenimJacket,   EquipmentCategory,"Denim Jacket",    0,0,0,0,0,"A simple denim jacket. Provides no protection but looks cool.",       "Denim Jacket",   "You equip the denim jacket.",                     3,false,ArmorEffect, 0,false,false,2,false},
  { RingMailArmor, EquipmentCategory,"Ring Mail Armor", 0,0,0,0,0,"Ring mail armor. Reduces damage taken.",                              "Ring Mail Armor","You equip the ring mail armor.",                   4,false,ArmorEffect, 3,false,false,2,true},
  { ChaosArmor,    EquipmentCategory,"Chaos Armor",     0,0,0,0,0,"This physical state of this armor is not stable.",                    "Chaos Armor",    "The chaos armor clings to you.",                  5,false,ArmorEffect, 0,false,true, 2,false},
  { SpikyArmor,    EquipmentCategory,"Spiky Armor",     0,0,0,0,0,"This armor hurts all who touch it.",                                  "Spiky Armor",    "You equip the spiky armor.",                      4,false,ArmorEffect,-1,false,true, 2,true},
  { KingArmor,     EquipmentCategory,"King Armor",      0,0,0,0,0,"Armor fit for a king. Bestowed upon you by unnatural means.",         "King Armor",     "You equip the king armor.",                       5,false,ArmorEffect,50,false,false,0,false},
  { Weapon,        WeaponCategory,   "Weapon",          0,0,0,0,0,"Weapon description",                                                  "Weapon",         "",                                                4,false,WeaponEffect,0,false,false,2,false},
  { Null,          PotionCategory,   "Null",            0,0,0,0,0,"",                                                                    "Null",           "",                                                5,false},
};

// ── weaponList ────────────────────────────────────────────────────────────────
WeaponItem weaponList[] = {
    { Sword,          MagicSword,      "Sword",           "A standard sword.",                 true,  15, 25, 1 },
    { LongSword,      MagicLongSword,  "Longsword",       "A longsword. Increased reach.",      true,  15, 35, 2 },
    { Staff,          MagicStaff,      "Staff",           "A trusty wooden staff.",             false, 10, 25, 1 },
    { Dagger,         MagicDagger,     "Dagger",          "A small dagger for fast attacks.",   true,   3, 15, 1 },
    { MagicStaff,     MagicStaff,      "Magic Staff",     "A staff enfused with magic.",        false,  7, 15, 1 },
    { MagicDagger,    MagicDagger,     "Magic Dagger",    "A dagger enfused with magic.",       false, 15, 20, 1 },
    { MagicSword,     MagicSword,      "Magic Sword",     "A sword enfused with magic.",        false, 25, 25, 1 },
    { MagicLongSword, MagicLongSword,  "Magic Longsword", "A longsword enfused with magic.",    false, 30, 30, 2 },
    { NoWeapon,       NoWeapon,        "No Weapon",       "Not a weapon.",                      false,  0, 10, 1 },
};

// ── Scroll names ──────────────────────────────────────────────────────────────
char scrollNames[NUM_SCROLLS][20] = {
    "Protect scroll","Identify scroll","Enchant scroll","Uncurse scroll",
    "Blank scroll","Mapping scroll","Aggravate scroll","Destroy scroll",
    "Teleport scroll","Amnesia scroll","Lore scrap",
};
char scrollNamesRevealed[NUM_SCROLLS][20] = {
    "Protect scroll","Identify scroll","Enchant scroll","Uncurse scroll",
    "Blank scroll","Mapping scroll","Aggravate scroll","Destroy scroll",
    "Teleport scroll","Amnesia scroll","Lore scrap",
};

// ── Potion effects ────────────────────────────────────────────────────────────
PotionEffect potionEffects[] = {
  { 20,0,0,0,      "Healing Potion",  "Healing. Heals 20 HP.",                             "You feel better.",                                           HealingEffect   },
  {-20,0,0,0,      "Diluted Poison",  "Deducts 20 HP. Don't drink.",                       "You feel a bit sick.",                                       PoisonEffect    },
  {  0,4,40,0,     "Explosion Potion","Bomb. Deals 40 damage around you.",                 "The enemies around you lose 40 HP.",                         ExplosionEffect },
  { 40,4,-30,0,    "Buffing Potion",  "Heals 40 HP, but also heals nearby enemies.",       "You feel better, but so do nearby enemies.",                  BuffingEffect   },
  { 70,0,0,0,      "Mega Heal Potion","Healing, but mega. Heals 70 HP.",                   "You feel much better.",                                      MegaHealEffect  },
  {-50,4,-20,0,    "Bad Potion",      "Deducts 50 HP and gives nearby enemies 20 HP.",     "You lose 50 HP, nearby enemies gain 20 HP.",                  BadEffect       },
  {  0,0,0,1,      "Speed Potion",    "Drink this, and you'll go twice as fast.",          "You are faster now, for a limited time.",                     SpeedEffect     },
  {  0,0,0,-0.4f,  "Slowing Potion",  "Drink this, and you'll go half as fast.",           "You are slower now, for a limited time.",                     SlowEffect      },
  {  0,0,0,0,      "Hunger Potion",   "Makes you more hungry.",                            "You are now more hungry.",                                   HungerEffect    },
  {  0,0,0,0,      "See-all Potion",  "Opens your eyes to the unseen.",                   "You can now see the unseen for a limited time.",              SeeAllEffect    },
  {  0,0,0,0,      "Confusion Potion","You go opposite to where you want.",               "What is going on?",                                          ConfusionEffect },
  {  0,0,0,0,      "Ridicule Potion", "Drinking this makes you feel stupid.",              "You feel stupid.",                                           RidiculeEffect  },
  {  0,0,0,0,      "Bland Potion",    "Colored liquid that does nothing.",                 "Nothing happens.",                                           NoEffect        },
  {  0,0,0,0,      "Glamour Potion",  "Drinking this makes you feel awesome.",             "You feel fabulous!",                                         GlamourEffect   },
  {  0,0,0,0,      "Chaos Potion",    "The effect of this potion is random.",              "A lot happens.",                                             ChaosEffect     },
  {  0,0,0,0,      "Blindness Potion","Makes you blind for a time.",                       "A cloak of darkness falls around you.",                      BlindnessEffect },
  {  0,0,0,0,      "Strength Potion", "Makes you do more damage.",                        "You feel stronger.",                                         StrengthEffect  },
  {  0,0,0,0,      "Restore Potion",  "Cures all your ailments.",                         "You feel restored!",                                         RestoreEffect   },
  {  0,0,0,0,      "Paralysis Potion","Paralyzes you for a time.",                         "You can't move!",                                            ParalysisEffect },
  {-40,0,0,0,      "Poison",          "Deducts 40 HP.",                                   "You feel very sick.",                                        VeryPoisonEffect},
};

// ── Scroll effects ────────────────────────────────────────────────────────────
ScrollEffect scrollEffects[NUM_SCROLLS] = {
    {"Protect scroll", "Protects your armor from rusting and raises its damage reduction.",
     "Your armor is covered by a shimmering gold shield!",        ScrollProtectionEffect, true },
    {"Identify scroll","Reveals the true name of an item and sees if it is cursed.",
     "Select an item to identify.",                               ScrollIdentifyEffect,   true },
    {"Enchant scroll", "Makes you do more damage.",
     "You do more damage.",                                       ScrollEnchantEffect,    true },
    {"Uncurse scroll", "Removes curses from all equipped items.",
     "You feel as if someone is watching over you.",              ScrollUncurseEffect,    true },
    {"Blank scroll",   "It looks like it's just a blank piece of paper.",
     "You look at the blank scroll.",                             ScrollEmptyEffect,      true },
    {"Mapping scroll", "It has a map on it.",
     "You study the map on the scroll.",                          ScrollMapEffect,        true },
    {"Amnesia scroll", "Makes you forget all discovered items.",
     "You feel as if you've forgotten something...",              ScrollAmnesiaEffect,    true },
    {"Aggravate scroll","Makes all enemies target you.",
     "You hear a high-pitched humming noise.",                    ScrollAggravateEffect,  true },
    {"Destroy scroll", "Destroys the integrity of your armor.",
     "Your armor now feels like paper.",                          ScrollDestroyEffect,    true },
    {"Teleport scroll","Teleports you to a random location.",
     "You feel a wrenching sensation.",                           ScrollTeleportEffect,   true },
    {"Lore scrap",     "A scrap of parchment with someone's writing on it.",
     "",                                                          ScrollLoreEffect,       false},
};

// ── Item combinations ─────────────────────────────────────────────────────────
ItemCombination itemCombinations[] = {
    {BluePotion,  YellowPotion, GreenPotion    },
    {RedPotion,   GreenPotion,  YellowPotion   },
    {RedPotion,   YellowPotion, OrangePotion   },
    {RedPotion,   BluePotion,   PurplePotion   },
    {GreenPotion, BluePotion,   CyanPotion     },
    {RedPotion,   BlackPotion,  MaroonPotion   },
    {GreenPotion, BlackPotion,  DarkGreenPotion},
    {GreenPotion, YellowPotion, LimePotion     },
    {WhitePotion, BlackPotion,  GreyPotion     },
    {YellowPotion,BlackPotion,  OlivePotion    },
    {YellowPotion,WhitePotion,  CreamPotion    },
    {BluePotion,  BlackPotion,  NavyPotion     },
    {BluePotion,  WhitePotion,  AzurePotion    },
    {GreenPotion, WhitePotion,  MintPotion     },
    {RedPotion,   WhitePotion,  SalmonPotion   },
};

// ── Ring data ─────────────────────────────────────────────────────────────────
char ringTypes[NUM_RINGS][20] = {
    "Wooden Ring","Emerald Ring","Diamond Ring","Clay Ring",
    "Gold Ring",  "Ruby ring",   "Washer",      "Azure ring",
    "Stone ring", "Opal ring",   "Copper ring", "Silver ring"
};
char ringEffects[NUM_RINGS][100] = {
    "Ring of Swiftness","Ring of Strength",    "Ring of Weakness",    "Ring of Hunger",
    "Ring of Regeneration","Ring",             "Ring of Sickness",    "Ring of Aggravation",
    "Ring of Armor",    "Ring of Indigestion", "Ring of Teleport",    "Ring of Invisibility"
};
bool ringCursed[NUM_RINGS] = {
    false,false,true,true,false,false,true,true,false,false,true,true
};
char ringDescriptions[NUM_RINGS][100] = {
    "While wearing this ring, you move faster.",
    "While wearing this ring, you deal more damage.",
    "While wearing this ring, you deal less damage.",
    "Makes you starve quicker.",
    "Passively heals you.",
    "It's just a ring.",
    "Lowers your maximum HP.",
    "Makes enemies never stop chasing you.",
    "Prevents armor degrading while slightly boosting defence.",
    "Makes you get hungry slower.",
    "Makes you randomly teleport all over the map.",
    "Makes you invisible to all, including yourself.",
};

// ─────────────────────────────────────────────────────────────────────────────
// Functions — logic identical to original
// ─────────────────────────────────────────────────────────────────────────────

void randomizePotionEffects() {
    for (int i = NUM_POTIONS - 1; i > 0; i--) {
        int j = random(i + 1);
        PotionEffect tmp = potionEffects[i];
        potionEffects[i] = potionEffects[j];
        potionEffects[j] = tmp;
    }
    for (int i = 0; i < NUM_POTIONS; i++) {
        itemList[i].healthRecoverAmount = potionEffects[i].healthChange;
        itemList[i].AOEsize             = potionEffects[i].AOEsize;
        itemList[i].AOEdamage           = potionEffects[i].AOEdamage;
        itemList[i].SpeedMultiplier     = potionEffects[i].SpeedMultiplier;
        snprintf(itemList[i].itemResult, sizeof(itemList[i].itemResult),
                 "%s", potionEffects[i].effectResult);
        itemList[i].effectType = potionEffects[i].effectType;
    }
}

GameItem getItem(GameItems item) {
    GameItem newItem = itemList[item];

    if (item == Scroll) {
        int idx = random(0, NUM_SCROLLS);
        newItem.scrollEffectIndex = idx;
        newItem.isScrollRevealed  = false;
        snprintf(newItem.name,         sizeof(newItem.name),         "%s", scrollNamesRevealed[idx]);
        snprintf(newItem.originalName, sizeof(newItem.originalName), "%s", scrollNames[idx]);
        if (scrollEffects[idx].effectType == ScrollLoreEffect) {
            newItem.oneTimeUse = false;
            snprintf(newItem.description, sizeof(newItem.description),
                     "%s", "A scrap of parchment with writing on it.");
            int li = random(0, NUM_LORE_TEXTS);
            const char* txt = loreTexts[li];
            snprintf(newItem.itemResult, sizeof(newItem.itemResult),
                     "%s", (txt && txt[0]) ? txt : "The writing is too faded to make out.");
        } else {
            newItem.oneTimeUse = true;
            snprintf(newItem.description, sizeof(newItem.description), "%s", "Read it to find out.");
        }
    }

    if (item == Ring) {
        int ti = random(0, NUM_RINGS);
        newItem.ringTypeIndex    = ti;
        newItem.ringEffectIndex  = -1;
        newItem.isRingIdentified = false;
        newItem.isCursed         = false;
        snprintf(newItem.name,         sizeof(newItem.name),         "%s", ringTypes[ti]);
        snprintf(newItem.originalName, sizeof(newItem.originalName), "%s", ringTypes[ti]);
    }
    return newItem;
}

void updatePotionName(GameItem& potion) {
    for (auto& effect : potionEffects) {
        if (potion.effectType != effect.effectType) continue;
        for (int i = 0; i < NUM_POTIONS; i++) {
            if (itemList[i].item != potion.item) continue;
            snprintf(itemList[i].name,        sizeof(itemList[i].name),        "%s", effect.effectName);
            snprintf(itemList[i].description, sizeof(itemList[i].description), "%s", effect.effectDescription);
        }
        for (int i = 0; i < INVENTORY_SIZE; i++) {
            if (inventoryPages[0].items[i].item != potion.item) continue;
            snprintf(inventoryPages[0].items[i].name,        sizeof(inventoryPages[0].items[i].name),        "%s", effect.effectName);
            snprintf(inventoryPages[0].items[i].description, sizeof(inventoryPages[0].items[i].description), "%s", effect.effectDescription);
        }
        break;
    }
}

void resetPotionNames() {
    for (int i = 0; i < NUM_POTIONS; i++)
        snprintf(itemList[i].name, sizeof(itemList[i].name), "%s", itemList[i].originalName);
}

GameItems getRandomPotion(int randInt, bool primaryColors) {
    if (primaryColors) {
        GameItems primaries[] = { RedPotion, GreenPotion, BluePotion,
                                  BlackPotion, WhitePotion, YellowPotion };
        return primaries[randInt % 6];
    }
    GameItems pool[NUM_POTIONS]; int count = 0;
    for (size_t i = 0; i < sizeof(itemList) / sizeof(itemList[0]); i++)
        if (itemList[i].category == PotionCategory &&
            itemList[i].item != EmptyBottle && itemList[i].item != Null)
            pool[count++] = itemList[i].item;
    return pool[randInt % count];
}

void applyAOEEffect(float cx, float cy, int aoeRadius, int aoeDamage) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].hp <= 0) continue;
        int dx = round(cx) - round(enemies[i].x);
        int dy = round(cy) - round(enemies[i].y);
        if (dx * dx + dy * dy <= aoeRadius * aoeRadius && strcmp(enemies[i].name, "shopkeeper") != 0 && !enemies[i].isFriend) {
            enemies[i].hp -= aoeDamage;
            if (enemies[i].hp <= 0) kills++;
            if (enemies[i].hp <= 0 && random(0, 100) < 15 && dungeonMap[(int)round(enemies[i].y)][(int)round(enemies[i].x)] == Floor) dungeonMap[(int)round(enemies[i].y)][(int)round(enemies[i].x)] = MushroomTile;
        }
    }
}

void renderItemResult() {
    display.clearDisplay();
    u8g2_for_adafruit_gfx.setFont(u8g2_font_profont12_tr);
    display.setCursor(15, 65);
    display.print(itemResultMessage);
    display.display();
}

bool areItemsEqual(GameItem a, GameItem b) {
    return a.item == b.item && a.category == b.category &&
           a.healthRecoverAmount == b.healthRecoverAmount &&
           a.AOEsize == b.AOEsize && a.AOEdamage == b.AOEdamage &&
           a.SpeedMultiplier == b.SpeedMultiplier &&
           strcmp(a.name, b.name) == 0 && strcmp(a.description, b.description) == 0 &&
           strcmp(a.originalName, b.originalName) == 0 &&
           strcmp(a.itemResult, b.itemResult) == 0;
}

GameItem combineItems(GameItem item1, GameItem item2) {
    GameItems primaryPotions[] = { RedPotion,GreenPotion,BluePotion,
                                   YellowPotion,WhitePotion,BlackPotion };
    auto isPrimary = [&](GameItems it) {
        for (auto p : primaryPotions) if (it == p) return true;
        return false;
    };

    if (item1.category == PotionCategory && item2.category == PotionCategory) {
        if (item1.item == EmptyBottle && !item2.item != EmptyBottle) return getItem(item2.item);
        if (!item1.item != EmptyBottle && item2.item == EmptyBottle) return getItem(item1.item);
        if (item1.item == EmptyBottle && item2.item == EmptyBottle)  return getItem(EmptyBottle);
        if (item1.item == item2.item) return getItem(Null);
        if (!isPrimary(item1.item) || !isPrimary(item2.item)) return getItem(BrownPotion);
    }

    bool i1Equip = (item1.category == EquipmentCategory || item1.category == WeaponCategory);
    bool i2Equip = (item2.category == EquipmentCategory || item2.category == WeaponCategory);

    if ((item1.category == PotionCategory && i2Equip) ||
        (i1Equip && item2.category == PotionCategory)) {
        if (item1.item == EmptyBottle || item2.item == EmptyBottle) {
            GameItem it = (item1.item == EmptyBottle) ? item2 : item1;
            snprintf(it.itemResult, sizeof(it.itemResult),
                     "The %s cannot fit inside the bottle.", it.name);
            return it;
        }
        bool washer = (strcmp(item1.name,"Washer")==0 || strcmp(item2.name,"Washer")==0);
        if (washer) {
            GameItem it = (item1.item == Ring) ? item1 : item2;
            snprintf(it.name,        sizeof(it.name),        "%s", "Wet Washer");
            snprintf(it.originalName,sizeof(it.originalName),"%s", "Wet Washer");
            snprintf(it.itemResult,  sizeof(it.itemResult),  "%s",
                     "The washer is wet now, making it easy to remove.");
            snprintf(it.description, sizeof(it.description), "%s",
                     "A wet washer. It being wet prevents it from getting stuck on your finger.");
            return it;
        }
        GameItem equip = (i1Equip) ? item1 : item2;
        GameItem pot   = (item1.category == PotionCategory) ? item1 : item2;
        if (equip.canRust) {
            bool cursed = (random(0,10) < 3);
            GameItem rusted = equip;
            rusted.AOEdamage = 123;
            if (equip.category == EquipmentCategory) rusted.armorValue    = equip.armorValue    - (equip.armorValue    < 0 ? 0 : 1);
            else                                     rusted.weapon.damage = equip.weapon.damage - (equip.weapon.damage < 0 ? 0 : 1);
            rusted.isCursed = cursed;
            rusted.canRust  = true;
            snprintf(rusted.description, sizeof(rusted.description),
                     "A Rusty %s. It looks degraded%s.", equip.name,
                     cursed ? ", and you feel a sense of unease around it." : "");
            snprintf(rusted.name, sizeof(rusted.name), "Rusty %s", equip.name);
            snprintf(rusted.itemResult, sizeof(rusted.itemResult),
                     "You pour the %s over the %s. It rusts%s.",
                     pot.name, equip.name,
                     cursed ? ", becomes less durable, and shimmers slightly red" : " and becomes less durable");
            if (equip.isEquipped) {
                if (equip.category == EquipmentCategory) equippedArmor  = rusted;
                else                                     equippedWeapon = rusted;
            }
            return rusted;
        } else {
            GameItem same = equip;
            snprintf(same.itemResult, sizeof(same.itemResult),
                     "You pour the %s over the %s, but nothing happens.", pot.name, equip.name);
            return same;
        }
    }

    if ((item1.category == PotionCategory && item2.category == FoodCategory) ||
        (item2.category == PotionCategory && item1.category == FoodCategory)) {
        if (item1.item == EmptyBottle || item2.item == EmptyBottle) {
            GameItem it = (item1.item == EmptyBottle) ? item2 : item1;
            snprintf(it.itemResult, sizeof(it.itemResult),
                     "The %s cannot fit inside the bottle.", it.name);
            return it;
        }
        GameItem pot  = (item1.category == PotionCategory) ? item1 : item2;
        GameItem food = (item1.category == FoodCategory)   ? item1 : item2;
        food.healthRecoverAmount = pot.healthRecoverAmount;
        food.hungerRecoverAmount = pot.hungerRecoverAmount;
        food.AOEsize             = pot.AOEsize;
        food.AOEdamage           = pot.AOEdamage;
        food.SpeedMultiplier     = pot.SpeedMultiplier;
        snprintf(food.name, sizeof(food.name), "Odd %s",
                 (item1.category == FoodCategory) ? item1.name : item2.name);
        return food;
    }

    for (int i = 0; i < NUM_ITEM_COMBINATIONS; i++) {
        const ItemCombination& c = itemCombinations[i];
        if ((item1.item == c.ingredient1 && item2.item == c.ingredient2) ||
            (item1.item == c.ingredient2 && item2.item == c.ingredient1))
            return getItem(c.result);
    }
    return getItem(Null);
}

GameItem CombineTwoItemsToGetItem(GameItem item1, GameItem item2) {
    GameItem result = combineItems(item1, item2);
    if (result.item != Null) return result;
    if ((item1.category == PotionCategory && item2.category == ScrollsCategory) ||
        (item2.category == PotionCategory && item1.category == ScrollsCategory))
        return getItem(WetScroll);
    GameItem nullItem = getItem(Null);
    snprintf(nullItem.name, sizeof(nullItem.name), "%s", "Null");
    return nullItem;
}

void generateScrollName(char* name, size_t nameSize) {
    static const char* consonants[] = {
        "b","c","d","f","g","h","j","k","l","m","n","p","r","s","t","v","w","y","z"
    };
    static const char* vowels[] = {"a","e","i","o","u"};
    const int nc = sizeof(consonants) / sizeof(consonants[0]);
    const int nv = sizeof(vowels)     / sizeof(vowels[0]);

    char base[20] = "";
    int  len = random(4, 8);
    for (int i = 0; i < len && (int)strlen(base) < 18; i++) {
        strcat(base, (i % 2 == 0) ? consonants[random(0, nc)] : vowels[random(0, nv)]);
    }
    if (strlen(base) > 0) base[0] = (char)toupper((unsigned char)base[0]);
    snprintf(name, nameSize, "Scroll: %s", base);
}

void randomizeScrollEffects() {
    const int NM = NUM_SCROLLS - 1;   // leave lore scrap (index 10) in place
    for (int i = 0; i < NM; i++) {
        generateScrollName(scrollNames[i],         sizeof(scrollNames[i]));
        generateScrollName(scrollNamesRevealed[i], sizeof(scrollNamesRevealed[i]));
    }
    for (int i = NM - 1; i > 0; i--) {
        int j = random(i + 1);
        ScrollEffect tmp = scrollEffects[i];
        scrollEffects[i] = scrollEffects[j];
        scrollEffects[j] = tmp;
    }
}

void updateScrollName(GameItem& scroll) {
    if (scroll.scrollEffectIndex < 0 || scroll.scrollEffectIndex >= NUM_SCROLLS) return;
    ScrollEffect& eff = scrollEffects[scroll.scrollEffectIndex];

    if (eff.effectType == ScrollLoreEffect) {
        scroll.isScrollRevealed = true;
        return;
    }

    snprintf(scroll.name,        sizeof(scroll.name),        "%s", eff.effectName);
    snprintf(scroll.description, sizeof(scroll.description), "%s", eff.effectDescription);
    snprintf(scroll.itemResult,  sizeof(scroll.itemResult),  "%s", eff.effectResult);
    scroll.effectType       = eff.effectType;
    scroll.isScrollRevealed = true;
    snprintf(scrollNamesRevealed[scroll.scrollEffectIndex],
             sizeof(scrollNamesRevealed[0]), "%s", eff.effectName);

    for (int p = 0; p < numInventoryPages; p++) {
        for (int i = 0; i < INVENTORY_SIZE; i++) {
            GameItem& it = inventoryPages[p].items[i];
            if (it.item != Scroll || it.scrollEffectIndex != scroll.scrollEffectIndex) continue;
            snprintf(it.name,        sizeof(it.name),        "%s", eff.effectName);
            snprintf(it.description, sizeof(it.description), "%s", eff.effectDescription);
            snprintf(it.itemResult,  sizeof(it.itemResult),  "%s", eff.effectResult);
            it.effectType       = eff.effectType;
            it.isScrollRevealed = true;
        }
    }
}

void randomizeRingEffects() {
    for (int i = NUM_RINGS - 1; i > 0; i--) {
        int j = random(i + 1);
        char tmp[100];
        memcpy(tmp, ringEffects[i], 100); memcpy(ringEffects[i], ringEffects[j], 100); memcpy(ringEffects[j], tmp, 100);
        bool tb = ringCursed[i]; ringCursed[i] = ringCursed[j]; ringCursed[j] = tb;
    }
}

void updateRingName(GameItem& ring) {
    if (ring.ringEffectIndex < 0 || ring.ringEffectIndex >= NUM_RINGS) {
        ring.ringEffectIndex = random(0, NUM_RINGS);
        ring.isCursed        = ringCursed[ring.ringEffectIndex];
    }
    int idx = ring.ringEffectIndex;
    snprintf(ring.name,        sizeof(ring.name),        "%s", ringEffects[idx]);
    snprintf(ring.description, sizeof(ring.description), "%s", ringDescriptions[idx]);
    ring.isRingIdentified = true;
}

GameItems getRandomItemByRarity(ItemCategory category, int maxRarity) {
    GameItems cands[50]; int weights[50]; int count = 0;
    for (size_t i = 0; i < sizeof(itemList)/sizeof(itemList[0]); i++) {
        if (itemList[i].category == category && itemList[i].rarity <= maxRarity && itemList[i].item != Null) {
            cands[count]   = itemList[i].item;
            weights[count] = max(1, 6 - itemList[i].rarity);
            count++;
        }
    }
    if (count == 0 || random(0,100) > 70) return Null;
    int total = 0; for (int i = 0; i < count; i++) total += weights[i];
    int r = random(0, total), cum = 0;
    for (int i = 0; i < count; i++) { cum += weights[i]; if (r < cum) return cands[i]; }
    return cands[0];
}

GameItems getRandomItemByRarityAnyCategory(int maxRarity) {
    GameItems cands[50]; int weights[50]; int count = 0;
    for (size_t i = 0; i < sizeof(itemList)/sizeof(itemList[0]); i++) {
        if (itemList[i].rarity <= maxRarity && itemList[i].item != Null && itemList[i].item != WetScroll) {
            cands[count]   = itemList[i].item;
            weights[count] = max(1, 6 - itemList[i].rarity);
            count++;
        }
    }
    if (count == 0) return Null;
    int total = 0; for (int i = 0; i < count; i++) total += weights[i];
    int r = random(0, total), cum = 0;
    for (int i = 0; i < count; i++) { cum += weights[i]; if (r < cum) return cands[i]; }
    return cands[0];
}

TileTypes getRandomLootTile(int maxRarity) {
    ItemCategory cats[] = { PotionCategory, FoodCategory,
                             ScrollsCategory, EquipmentCategory, WeaponCategory };
    GameItems ri = getRandomItemByRarity(cats[random(0, 5)], maxRarity);
    if (ri == Null) return GoldTile;
    switch (getItem(ri).category) {
        case PotionCategory:    return Potion;
        case ScrollsCategory:   return ScrollTile;
        case EquipmentCategory:
            if (ri == Ring)        return RingTile;
            if (ri == RiddleStone) return RiddleStoneTile;
            return ArmorTile;
        case FoodCategory:      return MushroomTile;
        case WeaponCategory:    return WeaponTile;
        default:                return GoldTile;
    }
}