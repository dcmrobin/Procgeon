#include "Item.h"
#include "Inventory.h"
#include "HelperFunctions.h"
#include "Player.h"
#include "Translation.h"

extern int numInventoryPages;

// ── Lore scrap texts ──────────────────────────────────────────────────────────
// Fill these strings with your dungeon's story fragments.
// They are assigned randomly when a lore scrap is created.
const char* loreTexts[NUM_LORE_TEXTS] = {
    "...er of this dungeon trapped down here, unable to esca...",  // 0
    "...more to scrolls than meets the untrained...",  // 1
    "...lie. Cake...",  // 2
    "...pin. Then shalt thou count to three, no more, no...",  // 3
    "...locked me in a room, a ru...",  // 4
    "...food, I shall die here if I...",  // 5
    "...nd so this will be my final resting pl...",  // 6
    "...at cursed washer... I can't get it off my fin...",  // 7
    "...is place, it never en...",  // 8
    "...t first his song was nice, I realized too la...",  // 9
    "...the light...",  // 10
    "...hopeless...",  // 11
    "...regret...",  // 12
    "...was a fool! Why di...",  // 13
    "...f what folly brought me to this place, I do not kn...",  // 14
    "...LL I EVER GET OU...",  // 15
    "...no way home, no w...",  // 16
    "...urn back if thou art readi...",  // 17
    "...nd yet it seems She was curious ab...",  // 18
    "...rhaps I could be Her frie...",  // 19
};

// ── itemList ──────────────────────────────────────────────────────────────────
GameItem itemList[] = {
  // ── Potions (indices 0-19) ────────────────────────────────────────────
  { RedPotion,       PotionCategory, "Red Potion",       0, 0, 0, 0, 0, "Drink it to find out.", "Red Potion",       "Nothing happens."},
  { GreenPotion,     PotionCategory, "Green Potion",     0, 0, 0, 0, 0, "Drink it to find out.", "Green Potion",     "Nothing happens."},
  { BluePotion,      PotionCategory, "Blue Potion",      0, 0, 0, 0, 0, "Drink it to find out.", "Blue Potion",      "Nothing happens."},
  { BlackPotion,     PotionCategory, "Black Potion",     0, 0, 0, 0, 0, "Drink it to find out.", "Black Potion",     "Nothing happens."},
  { WhitePotion,     PotionCategory, "White Potion",     0, 0, 0, 0, 0, "Drink it to find out.", "White Potion",     "Nothing happens."},
  { YellowPotion,    PotionCategory, "Yellow Potion",    0, 0, 0, 0, 0, "Drink it to find out.", "Yellow Potion",    "Nothing happens."},
  { OrangePotion,    PotionCategory, "Orange Potion",    0, 0, 0, 0, 0, "Drink it to find out.", "Orange Potion",    "Nothing happens."},
  { PurplePotion,    PotionCategory, "Purple Potion",    0, 0, 0, 0, 0, "Drink it to find out.", "Purple Potion",    "Nothing happens."},
  { CyanPotion,      PotionCategory, "Cyan Potion",      0, 0, 0, 0, 0, "Drink it to find out.", "Cyan Potion",      "Nothing happens."},
  { MaroonPotion,    PotionCategory, "Maroon Potion",    0, 0, 0, 0, 0, "Drink it to find out.", "Maroon Potion",    "Nothing happens."},
  { DarkGreenPotion, PotionCategory, "DarkGreen Potion", 0, 0, 0, 0, 0, "Drink it to find out.", "DarkGreen Potion", "Nothing happens."},
  { LimePotion,      PotionCategory, "Lime Potion",      0, 0, 0, 0, 0, "Drink it to find out.", "Lime Potion",      "Nothing happens."},
  { GreyPotion,      PotionCategory, "Grey Potion",      0, 0, 0, 0, 0, "Drink it to find out.", "Grey Potion",      "Nothing happens."},
  { OlivePotion,     PotionCategory, "Olive Potion",     0, 0, 0, 0, 0, "Drink it to find out.", "Olive Potion",     "Nothing happens."},
  { CreamPotion,     PotionCategory, "Cream Potion",     0, 0, 0, 0, 0, "Drink it to find out.", "Cream Potion",     "Nothing happens."},
  { NavyPotion,      PotionCategory, "Navy Potion",      0, 0, 0, 0, 0, "Drink it to find out.", "Navy Potion",      "Nothing happens."},
  { AzurePotion,     PotionCategory, "Azure Potion",     0, 0, 0, 0, 0, "Drink it to find out.", "Azure Potion",     "Nothing happens."},
  { MintPotion,      PotionCategory, "Mint Potion",      0, 0, 0, 0, 0, "Drink it to find out.", "Mint Potion",      "Nothing happens."},
  { SalmonPotion,    PotionCategory, "Salmon Potion",    0, 0, 0, 0, 0, "Drink it to find out.", "Salmon Potion",    "Nothing happens."},
  { BrownPotion,     PotionCategory, "Brown Potion",     0, 0, 0, 0, 0, "Drink it to find out.", "Brown Potion",     "Nothing happens."},

  // ── Food (indices 20-24) ──────────────────────────────────────────────
  { Mushroom,  FoodCategory, "Mushroom",   0, 20,  0, 0, 0,
    "It is edible.",                               "Mushroom",   "You become less hungry.",                            1},
  { Bread,     FoodCategory, "Bread",      0, 30,  0, 0, 0,
    "Stale but edible.",                           "Bread",      "You eat the bread. Not great, but filling.",         2},
  { Cheese,    FoodCategory, "Cheese",     0, 15,  0, 0, 0,
    "Smells suspicious. Tastes fine.",             "Cheese",     "You nibble the cheese. A little goes a long way.",   2},
  { StaleMeat, FoodCategory, "Stale Meat", -5, 40, 0, 0, 0,
    "Probably fine. Probably.",                    "Stale Meat", "You eat the meat. Your stomach protests.",           2, true, StaleMeatEffect},
  { Berries,   FoodCategory, "Berries",    0, 10,  0, 0, 0,
    "Small but sweet. Hard to find underground.",  "Berries",    "You eat the berries. Delicious, if scarce.",         4},

  // ── Other items (indices 25+) ─────────────────────────────────────────
  { EmptyBottle,   PotionCategory,    "Empty Bottle",   0,  0, 0, 0, 0, "It is an empty bottle.",                                               "Empty Bottle",   "Nothing happens. It's an empty bottle.",           4, false},
  { RiddleStone,   EquipmentCategory, "Riddle Stone",   0,  0, 0, 0, 0, "Looks like it could be used for many things...",                       "Riddle Stone",   "Solve this riddle!",                               5, true, DefaultEffect, 0, false, false, 2, false},
  { Scroll,        ScrollsCategory,   "Scroll",         0,  0, 0, 0, 0, "Read it to find out.",                                                 "Scroll",         "Nothing happens.",                                 4},
  { WetScroll,     ScrollsCategory,   "Wet Scroll",     0,  0, 0, 0, 0, "A scroll that is too wet to read.",                                    "Wet Scroll",     "The scroll is too wet to read. Nothing happens.",  3, false},
  { Ring,          EquipmentCategory, "Ring",           0,  0, 0, 0, 0, "Put it on to find out.",                                               "Ring",           "You equip the ring.",                              3, false},
  { LeatherArmor,  EquipmentCategory, "Leather Armor",  0,  0, 0, 0, 0, "Basic leather armor. Reduces damage taken.",                           "Leather Armor",  "You equip the leather armor.",                     3, false, ArmorEffect,  2,  false, false, 2, false},
  { IronArmor,     EquipmentCategory, "Iron Armor",     0,  0, 0, 0, 0, "Sturdy iron armor. Reduces damage taken.",                             "Iron Armor",     "You equip the iron armor.",                        4, false, ArmorEffect,  4,  false, false, 2, true},
  { MagicRobe,     EquipmentCategory, "Magic Robe",     0,  0, 0, 0, 0, "Enchanted robe. Reduces damage taken and increases magic resistance.", "Magic Robe",     "You equip the magic robe.",                        4, false, ArmorEffect,  1,  false, false, 2, false},
  { Cloak,         EquipmentCategory, "Cloak",          0,  0, 0, 0, 0, "A simple cloth cloak. Provides no protection but keeps you warm.",     "Cloak",          "You equip the cloak.",                             4, false, ArmorEffect,  0,  false, false, 2, false},
  { Trenchcoat,    EquipmentCategory, "Trenchcoat",     0,  0, 0, 0, 0, "A trenchcoat and fedora. Provides no protection but looks cool.",      "Trenchcoat",     "You equip the trenchcoat.",                        3, false, ArmorEffect,  0,  false, false, 2, false},
  { DenimJacket,   EquipmentCategory, "Denim Jacket",   0,  0, 0, 0, 0, "A simple denim jacket. Provides no protection but looks cool.",        "Denim Jacket",   "You equip the denim jacket.",                      3, false, ArmorEffect,  0,  false, false, 2, false},
  { RingMailArmor, EquipmentCategory, "Ring Mail Armor",0,  0, 0, 0, 0, "Ring mail armor. Reduces damage taken.",                               "Ring Mail Armor","You equip the ring mail armor.",                   4, false, ArmorEffect,  3,  false, false, 2, true},
  { ChaosArmor,    EquipmentCategory, "Chaos Armor",    0,  0, 0, 0, 0, "This physical state of this armor is not stable.",                     "Chaos Armor",    "The chaos armor clings to you.",                   5, false, ArmorEffect,  0,  false, true,  2, false},
  { SpikyArmor,    EquipmentCategory, "Spiky Armor",    0,  0, 0, 0, 0, "This armor hurts all who touch it.",                                   "Spiky Armor",    "You equip the spiky armor.",                       4, false, ArmorEffect, -1,  false, true,  2, true},
  { KingArmor,     EquipmentCategory, "King Armor",     0,  0, 0, 0, 0, "Armor fit for a king. Bestowed upon you by unnatural means.",          "King Armor",     "You equip the king armor.",                        5, false, ArmorEffect, 50,  false, false, 0, false},
  { Weapon,        WeaponCategory,    "Weapon",         0,  0, 0, 0, 0, "Weapon description",                                                   "Weapon",         "",                                                 4, false, WeaponEffect, 0, false, false, 2, false},
  { Null,          PotionCategory,    "Null",           0,  0, 0, 0, 0, "",                                                                     "Null",           "",                                                 5, false}
};

// ── weaponList ────────────────────────────────────────────────────────────────
WeaponItem weaponList[] = {
    { Sword,          MagicSword,      "Sword",           "A standard sword.",                  true,  15, 25, 1 },
    { LongSword,      MagicLongSword,  "Longsword",       "A longsword. Increased reach.",       true,  15, 35, 2 },
    { Staff,          MagicStaff,      "Staff",           "A trusty wooden staff.",              false,  10, 25, 1 },
    { Dagger,         MagicDagger,     "Dagger",          "A small dagger for fast attacks.",    true,   3, 15, 1 },
    { MagicStaff,     MagicStaff,      "Magic Staff",     "A staff enfused with magic.",         false,  7, 15, 1 },
    { MagicDagger,    MagicDagger,     "Magic Dagger",    "A dagger enfused with magic.",        false, 15, 20, 1 },
    { MagicSword,     MagicSword,      "Magic Sword",     "A sword enfused with magic.",         false, 25, 25, 1 },
    { MagicLongSword, MagicLongSword,  "Magic Longsword", "A longsword enfused with magic.",     false, 30, 30, 2 },
    { NoWeapon,       NoWeapon,        "No Weapon",       "Not a weapon.",                       false,  0, 10, 1 }
};

// ── Scroll names ──────────────────────────────────────────────────────────────
// Index 10 = lore scrap — keeps its plain name, never randomised.
char scrollNames[NUM_SCROLLS][20] = {
    "Protect scroll",  "Identify scroll", "Enchant scroll",  "Uncurse scroll",
    "Blank scroll",    "Mapping scroll",  "Aggravate scroll","Destroy scroll",
    "Teleport scroll", "Amnesia scroll",  "Lore scrap",
};
char scrollNamesRevealed[NUM_SCROLLS][20] = {
    "Protect scroll",  "Identify scroll", "Enchant scroll",  "Uncurse scroll",
    "Blank scroll",    "Mapping scroll",  "Aggravate scroll","Destroy scroll",
    "Teleport scroll", "Amnesia scroll",  "Lore scrap",
};

// ── Potion effects ────────────────────────────────────────────────────────────
PotionEffect potionEffects[] = {
  { 20,  0,  0,  0,     "Healing Potion",   "Healing. Heals 20 of your HP.",                         "You feel better.",                                           HealingEffect  },
  { -20, 0,  0,  0,     "Diluted Poison",   "Deducts 20 of your HP. Don't drink.",                   "You feel a bit sick.",                                       PoisonEffect   },
  { 0,   4, 40,  0,     "Explosion Potion", "Bomb. Deals 40 damage to enemies around you.",           "The enemies around you lose 40 HP.",                         ExplosionEffect},
  { 40,  4, -30, 0,     "Buffing Potion",   "Heals 40 HP, but also heals nearby enemies.",            "You feel better, but so do the enemies close to you.",       BuffingEffect  },
  { 70,  0,  0,  0,     "Mega Heal Potion", "Healing, but mega. Heals 70 of your HP.",               "You feel much better.",                                      MegaHealEffect },
  { -50, 4, -20, 0,     "Bad Potion",       "Deducts 50 HP and gives nearby enemies 20 HP.",         "You lose 50 HP, and the enemies around you gain 20 HP.",     BadEffect      },
  { 0,   0,  0,  1,     "Speed Potion",     "Drink this, and you'll go twice as fast.",               "Your are faster now, but only for a limited amount of time.",SpeedEffect    },
  { 0,   0,  0, -0.4f,  "Slowing Potion",   "Drink this, and you'll go half as fast.",               "Your are slower now, but only for a limited amount of time.",SlowEffect     },
  { 0,   0,  0,  0,     "Hunger Potion",    "Makes you more hungry.",                                "You are now more hungry.",                                   HungerEffect   },
  { 0,   0,  0,  0,     "See-all Potion",   "Opens your eyes to the unseen.",                        "You can now see that which was unseen for a limited time.",  SeeAllEffect   },
  { 0,   0,  0,  0,     "Confusion Potion", "You go in the opposite direction to where you want.",   "What is going on?",                                          ConfusionEffect},
  { 0,   0,  0,  0,     "Ridicule Potion",  "Drinking this makes you feel stupid.",                  "You feel stupid.",                                           RidiculeEffect },
  { 0,   0,  0,  0,     "Bland Potion",     "Colored liquid that does nothing.",                     "Nothing happens.",                                           NoEffect       },
  { 0,   0,  0,  0,     "Glamour Potion",   "Drinking this makes you feel awesome.",                 "You feel fabulous!",                                         GlamourEffect  },
  { 0,   0,  0,  0,     "Chaos Potion",     "The effect of this potion is random.",                  "A lot happens.",                                             ChaosEffect    },
  { 0,   0,  0,  0,     "Blindness Potion", "Makes you blind for a time. Do not drink this.",        "A cloak of darkness falls around you.",                      BlindnessEffect},
  { 0,   0,  0,  0,     "Strength Potion",  "Makes you do more damage.",                             "You feel stronger.",                                         StrengthEffect },
  { 0,   0,  0,  0,     "Restore Potion",   "Cures all your ailments.",                              "You feel restored!",                                         RestoreEffect  },
  { 0,   0,  0,  0,     "Paralysis Potion", "Paralyzes you for a time.",                             "You can't move!",                                            ParalysisEffect},
  { -40, 0,  0,  0,     "Poison",           "Deducts 40 of your HP.",                               "You feel very sick.",                                        VeryPoisonEffect},
};

// ── Scroll effects ────────────────────────────────────────────────────────────
ScrollEffect scrollEffects[NUM_SCROLLS] = {
    {"Protect scroll",  "Protects your armor from rusting and raises its damage reduction.",
     "Your armor is covered by a shimmering gold shield!",          ScrollProtectionEffect, true },
    {"Identify scroll", "Reveals the true name of an item and sees if it is cursed.",
     "Select an item to identify.",                                  ScrollIdentifyEffect,   true },
    {"Enchant scroll",  "Makes you do more damage.",
     "You do more damage.",                                          ScrollEnchantEffect,    true },
    {"Uncurse scroll",  "Removes curses from all equipped items.",
     "You feel as if someone is watching over you.",                 ScrollUncurseEffect,    true },
    {"Blank scroll",    "It looks like it's just a blank piece of paper.",
     "You look at the blank scroll.",                                ScrollEmptyEffect,      true },
    {"Mapping scroll",  "It has a map on it.",
     "You study the map on the scroll.",                             ScrollMapEffect,        true },
    {"Amnesia scroll",  "Makes you forget all discovered items.",
     "You feel as if you've forgotten something...",                 ScrollAmnesiaEffect,    true },
    {"Aggravate scroll","Makes all enemies on the current level target you.",
     "You hear a high-pitched humming noise.",                       ScrollAggravateEffect,  true },
    {"Destroy scroll",  "Destroys the integrity of your armor.",
     "Your armor now feels like paper.",                             ScrollDestroyEffect,    true },
    {"Teleport scroll", "Teleports you to a random location on the map.",
     "You feel a wrenching sensation.",                              ScrollTeleportEffect,   true },
    {"Lore scrap",      "A scrap of parchment with someone's writing on it.",
     "",                                                             ScrollLoreEffect,       false},
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
const int NUM_ITEM_COMBINATIONS = sizeof(itemCombinations) / sizeof(itemCombinations[0]);

// ── Ring data ─────────────────────────────────────────────────────────────────
char ringTypes[NUM_RINGS][20] = {
  "Wooden Ring","Emerald Ring","Diamond Ring","Clay Ring",
  "Gold Ring",  "Ruby ring",   "Washer",      "Azure ring",
  "Stone ring", "Opal ring",   "Copper ring", "Silver ring"
};
char ringEffects[NUM_RINGS][100] = {
  "Ring of Swiftness","Ring of Strength",   "Ring of Weakness",   "Ring of Hunger",
  "Ring of Regeneration","Ring",            "Ring of Sickness",   "Ring of Aggravation",
  "Ring of Armor",    "Ring of Indigestion","Ring of Teleport",   "Ring of Invisibility"
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
  "Prevents your armor from degrading while also slightly boosting its defence.",
  "Makes you get hungry slower.",
  "Makes you randomly teleport all over the map.",
  "Makes you invisible to all, including yourself, meaning you become partially blind."
};

// ─────────────────────────────────────────────────────────────────────────────

void randomizePotionEffects() {
  for (int i = NUM_POTIONS - 1; i > 0; i--) {
    int j = random(i + 1);
    PotionEffect temp = potionEffects[i];
    potionEffects[i]  = potionEffects[j];
    potionEffects[j]  = temp;
  }
  for (int i = 0; i < NUM_POTIONS; i++) {
    itemList[i].healthRecoverAmount = potionEffects[i].healthChange;
    itemList[i].AOEsize             = potionEffects[i].AOEsize;
    itemList[i].AOEdamage           = potionEffects[i].AOEdamage;
    itemList[i].SpeedMultiplier     = potionEffects[i].SpeedMultiplier;
    snprintf(itemList[i].itemResult, sizeof(itemList[i].itemResult), "%s", potionEffects[i].effectResult);
    itemList[i].effectType = potionEffects[i].effectType;
  }
}

GameItem getItem(GameItems item) {
  GameItem newItem = itemList[item];

  if (item == Scroll) {
    int effectIndex = random(0, NUM_SCROLLS);
    newItem.scrollEffectIndex = effectIndex;
    newItem.isScrollRevealed  = false;
    snprintf(newItem.name,         sizeof(newItem.name),         "%s", scrollNamesRevealed[effectIndex]);
    snprintf(newItem.originalName, sizeof(newItem.originalName), "%s", scrollNames[effectIndex]);

    if (scrollEffects[effectIndex].effectType == ScrollLoreEffect) {
      newItem.oneTimeUse = false;
      snprintf(newItem.description, sizeof(newItem.description), "%s",
               "A scrap of parchment with writing on it.");
      int loreIndex   = random(0, NUM_LORE_TEXTS);
      const char* txt = loreTexts[loreIndex];
      snprintf(newItem.itemResult, sizeof(newItem.itemResult), "%s",
               (txt && txt[0] != '\0') ? txt : "The writing is too faded to make out.");
    } else {
      newItem.oneTimeUse = true;
      snprintf(newItem.description, sizeof(newItem.description), "%s", "Read it to find out.");
    }
  }

  if (item == Ring) {
    int typeIndex = random(0, NUM_RINGS);
    newItem.ringTypeIndex    = typeIndex;
    newItem.ringEffectIndex  = -1;
    newItem.isRingIdentified = false;
    newItem.isCursed         = false;
    snprintf(newItem.name,         sizeof(newItem.name),         "%s", ringTypes[typeIndex]);
    snprintf(newItem.originalName, sizeof(newItem.originalName), "%s", ringTypes[typeIndex]);
  }

  return newItem;
}

void updatePotionName(GameItem &potion) {
  for (PotionEffect effect : potionEffects) {
    if (potion.effectType == effect.effectType) {
      for (int i = 0; i < NUM_POTIONS; i++) {
        if (itemList[i].item == potion.item) {
          snprintf(itemList[i].name,        sizeof(itemList[i].name),        "%s", effect.effectName);
          snprintf(itemList[i].description, sizeof(itemList[i].description), "%s", effect.effectDescription);
        }
      }
      for (int i = 0; i < inventorySize; i++) {
        if (inventoryPages[0].items[i].item == potion.item) {
          snprintf(inventoryPages[0].items[i].name,        sizeof(inventoryPages[0].items[i].name),        "%s", effect.effectName);
          snprintf(inventoryPages[0].items[i].description, sizeof(inventoryPages[0].items[i].description), "%s", effect.effectDescription);
        }
      }
      break;
    }
  }
}

void resetPotionNames() {
  for (int i = 0; i < NUM_POTIONS; i++)
    snprintf(itemList[i].name, sizeof(itemList[i].name), "%s", itemList[i].originalName);
}

GameItems getRandomPotion(int randInt, bool primaryColors) {
  if (primaryColors) {
    GameItems potions[] = { RedPotion, GreenPotion, BluePotion, BlackPotion, WhitePotion, YellowPotion };
    return potions[randInt % 6];
  }
  GameItems potions[NUM_POTIONS];
  int count = 0;
  for (size_t i = 0; i < sizeof(itemList)/sizeof(itemList[0]); i++) {
    if (itemList[i].category == PotionCategory && itemList[i].item != EmptyBottle && itemList[i].item != Null)
      potions[count++] = itemList[i].item;
  }
  return potions[randInt % count];
}

void applyAOEEffect(float centerX, float centerY, int aoeRadius, int aoeDamage) {
  for (int i = 0; i < maxEnemies; i++) {
    if (enemies[i].hp > 0) {
      int dx = round(centerX) - round(enemies[i].x);
      int dy = round(centerY) - round(enemies[i].y);
      if (dx * dx + dy * dy <= aoeRadius * aoeRadius) {
        enemies[i].hp -= aoeDamage;
        if (enemies[i].hp <= 0) kills += 1;
      }
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

bool areItemsEqual(GameItem item1, GameItem item2) {
  return (item1.item == item2.item) &&
         (item1.category == item2.category) &&
         (item1.healthRecoverAmount == item2.healthRecoverAmount) &&
         (item1.AOEsize == item2.AOEsize) &&
         (item1.AOEdamage == item2.AOEdamage) &&
         (item1.SpeedMultiplier == item2.SpeedMultiplier) &&
         (strcmp(item1.name,         item2.name)         == 0) &&
         (strcmp(item1.description,  item2.description)  == 0) &&
         (strcmp(item1.originalName, item2.originalName) == 0) &&
         (strcmp(item1.itemResult,   item2.itemResult)   == 0);
}

GameItem combineItems(GameItem item1, GameItem item2) {
  GameItems primaryPotions[] = { RedPotion, GreenPotion, BluePotion, YellowPotion, WhitePotion, BlackPotion };
  auto isPrimaryPotion = [&](GameItems item) {
    for (int i = 0; i < 6; i++) if (item == primaryPotions[i]) return true;
    return false;
  };

  if (item1.category == PotionCategory && item2.category == PotionCategory) {
    bool item1Primary = isPrimaryPotion(item1.item);
    bool item2Primary = isPrimaryPotion(item2.item);
    bool item1Empty   = (item1.item == EmptyBottle);
    bool item2Empty   = (item2.item == EmptyBottle);
    if (item1Empty && !item2Empty)  return getItem(item2.item);
    if (!item1Empty && item2Empty)  return getItem(item1.item);
    if (item1Empty && item2Empty)   return getItem(EmptyBottle);
    if (item1.item == item2.item)   return getItem(Null);
    if (!item1Primary || !item2Primary) return getItem(BrownPotion);
  } else if ((item1.category == PotionCategory && (item2.category == EquipmentCategory || item2.category == WeaponCategory)) ||
             ((item1.category == EquipmentCategory || item1.category == WeaponCategory) && item2.category == PotionCategory)) {
    if (item1.item == EmptyBottle || item2.item == EmptyBottle) {
      GameItem item = item1.item == EmptyBottle ? item2 : item1;
      snprintf(item.itemResult, sizeof(item.itemResult), "The %s cannot fit inside the bottle.", item.name);
      return item;
    } else if (strcmp(item1.name, "Washer") == 0 || strcmp(item2.name, "Washer") == 0) {
      GameItem item = item1.item == Ring ? item1 : item2;
      snprintf(item.name,         sizeof(item.name),         "%s", "Wet Washer");
      snprintf(item.originalName, sizeof(item.originalName), "%s", "Wet Washer");
      snprintf(item.itemResult,   sizeof(item.itemResult),   "%s", "The washer is wet now, making it easy to remove.");
      snprintf(item.description,  sizeof(item.description),  "%s", "A wet washer. It being wet prevents it from getting stuck on your finger.");
      return item;
    } else if (((item1.category == EquipmentCategory || item1.category == WeaponCategory) ? item1.canRust : item2.canRust) &&
               strcmp(item1.name, "Washer") != 0 && strcmp(item2.name, "Washer") != 0) {
      GameItem item      = (item1.category == EquipmentCategory || item1.category == WeaponCategory) ? item1 : item2;
      bool     cursed    = random(0, 10) < 3;
      GameItem rustedItem = item;
      rustedItem.AOEdamage = 123;
      if (item.category == EquipmentCategory) rustedItem.armorValue     = item.armorValue     - (item.armorValue     < 0 ? 0 : 1);
      else                                    rustedItem.weapon.damage  = item.weapon.damage  - (item.weapon.damage  < 0 ? 0 : 1);
      rustedItem.isCursed = cursed;
      rustedItem.canRust  = true;
      snprintf(rustedItem.description, sizeof(rustedItem.description), "A Rusty %s. It looks degraded%s.", item.name, cursed ? ", and you feel a sense of unease around it." : "");
      snprintf(rustedItem.name,        sizeof(rustedItem.name),        "Rusty %s", item.name);
      snprintf(rustedItem.itemResult,  sizeof(rustedItem.itemResult),  "You pour the %s over the %s. It rusts%s.",
               (item1.category == PotionCategory ? item1.name : item2.name), item.name,
               cursed ? ", becomes less durable, and shimmers slightly red for a moment" : " and becomes less durable");
      if (item.isEquipped) {
        if (item.category == EquipmentCategory) equippedArmor  = rustedItem;
        else                                    equippedWeapon = rustedItem;
      }
      return rustedItem;
    } else if (!((item1.category == EquipmentCategory || item1.category == WeaponCategory) ? item1.canRust : item2.canRust) &&
               strcmp(item1.name, "Washer") != 0 && strcmp(item2.name, "Washer") != 0) {
      GameItem item = (item1.category == EquipmentCategory || item1.category == WeaponCategory) ? item1 : item2;
      GameItem unrustedItem = item;
      snprintf(unrustedItem.itemResult, sizeof(unrustedItem.itemResult), "You pour the %s over the %s, but nothing happens.",
               (item1.category == PotionCategory ? item1.name : item2.name), item.name);
      return unrustedItem;
    }
  } else if ((item1.category == PotionCategory && item2.category == FoodCategory) ||
             (item2.category == PotionCategory && item1.category == FoodCategory)) {
    if (item1.item == EmptyBottle || item2.item == EmptyBottle) {
      GameItem item = item1.item == EmptyBottle ? item2 : item1;
      snprintf(item.itemResult, sizeof(item.itemResult), "The %s cannot fit inside the bottle.", item.name);
      return item;
    }
    GameItem potion = (item1.category == PotionCategory) ? item1 : item2;
    GameItem food   = (item1.category == FoodCategory)   ? item1 : item2;
    food.healthRecoverAmount = potion.healthRecoverAmount;
    food.hungerRecoverAmount = potion.hungerRecoverAmount;
    food.AOEsize             = potion.AOEsize;
    food.AOEdamage           = potion.AOEdamage;
    food.SpeedMultiplier     = potion.SpeedMultiplier;
    snprintf(food.name, sizeof(food.name), "Odd %s", (item1.category == FoodCategory ? item1.name : item2.name));
    return food;
  }

  for (int i = 0; i < NUM_ITEM_COMBINATIONS; i++) {
    const ItemCombination& combo = itemCombinations[i];
    if ((item1.item == combo.ingredient1 && item2.item == combo.ingredient2) ||
        (item1.item == combo.ingredient2 && item2.item == combo.ingredient1))
      return getItem(combo.result);
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

void generateScrollName(char *name, size_t nameSize) {
  const char* consonants[] = {"b","c","d","f","g","h","j","k","l","m","n","p","q","r","s","t","v","w","x","y","z"};
  const char* vowels[]     = {"a","e","i","o","u"};
  char baseName[20] = "";
  int length = random(4, 8);
  for (int i = 0; i < length && i < (int)(sizeof(baseName) - 1); i++) {
    if (i % 2 == 0) strcat(baseName, consonants[random(0, (int)(sizeof(consonants)/sizeof(consonants[0])))]);
    else            strcat(baseName, vowels[random(0, (int)(sizeof(vowels)/sizeof(vowels[0])))]);
  }
  if (strlen(baseName) > 0) baseName[0] = toupper(baseName[0]);
  snprintf(name, nameSize, "Scroll: %s", baseName);
}

void randomizeScrollEffects() {
  // Only randomise the 10 magic scrolls; lore scrap (index 10) stays unchanged.
  const int NUM_MAGIC_SCROLLS = NUM_SCROLLS - 1;
  for (int i = 0; i < NUM_MAGIC_SCROLLS; i++) {
    generateScrollName(scrollNames[i],         sizeof(scrollNames[i]));
    generateScrollName(scrollNamesRevealed[i], sizeof(scrollNamesRevealed[i]));
  }
  for (int i = NUM_MAGIC_SCROLLS - 1; i > 0; i--) {
    int j = random(i + 1);
    ScrollEffect temp = scrollEffects[i];
    scrollEffects[i]  = scrollEffects[j];
    scrollEffects[j]  = temp;
  }
}

void updateScrollName(GameItem &scroll) {
  if (scroll.scrollEffectIndex < 0 || scroll.scrollEffectIndex >= NUM_SCROLLS) return;
  ScrollEffect effect = scrollEffects[scroll.scrollEffectIndex];

  // Lore scraps: just mark revealed, don't overwrite the lore text
  if (effect.effectType == ScrollLoreEffect) {
    scroll.isScrollRevealed = true;
    return;
  }

  snprintf(scroll.name,        sizeof(scroll.name),        "%s", effect.effectName);
  snprintf(scroll.description, sizeof(scroll.description), "%s", effect.effectDescription);
  snprintf(scroll.itemResult,  sizeof(scroll.itemResult),  "%s", effect.effectResult);
  scroll.effectType       = effect.effectType;
  scroll.isScrollRevealed = true;
  snprintf(scrollNamesRevealed[scroll.scrollEffectIndex],
           sizeof(scrollNamesRevealed[scroll.scrollEffectIndex]), "%s", effect.effectName);

  for (int p = 0; p < numInventoryPages; p++) {
    for (int i = 0; i < inventorySize; i++) {
      if (inventoryPages[p].items[i].item == Scroll &&
          inventoryPages[p].items[i].scrollEffectIndex == scroll.scrollEffectIndex) {
        snprintf(inventoryPages[p].items[i].name,        sizeof(inventoryPages[p].items[i].name),        "%s", effect.effectName);
        snprintf(inventoryPages[p].items[i].description, sizeof(inventoryPages[p].items[i].description), "%s", effect.effectDescription);
        snprintf(inventoryPages[p].items[i].itemResult,  sizeof(inventoryPages[p].items[i].itemResult),  "%s", effect.effectResult);
        inventoryPages[p].items[i].effectType       = effect.effectType;
        inventoryPages[p].items[i].isScrollRevealed = true;
      }
    }
  }
}

void randomizeRingEffects() {
  for (int i = NUM_RINGS - 1; i > 0; i--) {
    int j = random(i + 1);
    std::swap(ringEffects[i], ringEffects[j]);
    std::swap(ringCursed[i],  ringCursed[j]);
  }
}

void updateRingName(GameItem &ring) {
  if (ring.ringEffectIndex < 0 || ring.ringEffectIndex >= NUM_RINGS) {
    ring.ringEffectIndex = random(0, NUM_RINGS);
    ring.isCursed        = ringCursed[ring.ringEffectIndex];
  }
  if (ring.ringEffectIndex >= 0 && ring.ringEffectIndex < NUM_RINGS) {
    snprintf(ring.name,        sizeof(ring.name),        "%s", ringEffects[ring.ringEffectIndex]);
    snprintf(ring.description, sizeof(ring.description), "%s", ringDescriptions[ring.ringEffectIndex]);
    ring.isRingIdentified = true;
  }
}

GameItems getRandomItemByRarity(ItemCategory category, int maxRarity) {
  GameItems candidates[50]; int weights[50]; int count = 0;
  for (size_t i = 0; i < sizeof(itemList)/sizeof(itemList[0]); i++) {
    GameItem item = itemList[i];
    if (item.category == category && item.rarity <= maxRarity && item.item != Null) {
      candidates[count] = item.item;
      weights[count]    = max(1, 6 - item.rarity);
      count++;
    }
  }
  if (count == 0) return Null;
  if (random(0, 100) > 70) return Null; // chance for just gold
  int total = 0; for (int i = 0; i < count; i++) total += weights[i];
  int r = random(0, total), cur = 0;
  for (int i = 0; i < count; i++) { cur += weights[i]; if (r < cur) return candidates[i]; }
  return candidates[0];
}

GameItems getRandomItemByRarityAnyCategory(int maxRarity) {
  GameItems candidates[50]; int weights[50]; int count = 0;
  for (size_t i = 0; i < sizeof(itemList)/sizeof(itemList[0]); i++) {
    GameItem item = itemList[i];
    if (item.rarity <= maxRarity && item.item != Null && item.item != WetScroll) {
      candidates[count] = item.item;
      weights[count]    = max(1, 6 - item.rarity);
      count++;
    }
  }
  if (count == 0) return Null;
  int total = 0; for (int i = 0; i < count; i++) total += weights[i];
  int r = random(0, total), cur = 0;
  for (int i = 0; i < count; i++) { cur += weights[i]; if (r < cur) return candidates[i]; }
  return candidates[0];
}

TileTypes getRandomLootTile(int maxRarity) {
  ItemCategory categories[] = {PotionCategory, FoodCategory, ScrollsCategory, EquipmentCategory, WeaponCategory};
  GameItems randomItem = getRandomItemByRarity(categories[random(0, 5)], maxRarity);
  if (randomItem == Null) return GoldTile;

  switch (getItem(randomItem).category) {
    case PotionCategory:   return Potion;
    case ScrollsCategory:  return ScrollTile;
    case EquipmentCategory:
      if (randomItem == Ring)        return RingTile;
      if (randomItem == RiddleStone) return RiddleStoneTile;
      return ArmorTile;
    case FoodCategory:     return MushroomTile; // blanket food tile — random food on pickup
    case WeaponCategory:   return WeaponTile;
    default:               return GoldTile;
  }
}