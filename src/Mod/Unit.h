#pragma once
/*
 * Copyright 2010-2016 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>
#include "../Engine/Script.h"

namespace OpenXcom
{

enum SpecialAbility { SPECAB_NONE, SPECAB_EXPLODEONDEATH, SPECAB_BURNFLOOR, SPECAB_BURN_AND_EXPLODE };
/**
 * This struct holds some plain unit attribute data together.
 */
struct UnitStats
{
	using Type = Sint16;
	using Ptr = Type UnitStats::*;

	Type tu, stamina, health, bravery, reactions, firing, throwing, strength, psiStrength, psiSkill, melee;

	UnitStats() : tu(0), stamina(0), health(0), bravery(0), reactions(0), firing(0), throwing(0), strength(0), psiStrength(0), psiSkill(0), melee(0) {};
	UnitStats(int tu_, int stamina_, int health_, int bravery_, int reactions_, int firing_, int throwing_, int strength_, int psiStrength_, int psiSkill_, int melee_) : tu(tu_), stamina(stamina_), health(health_), bravery(bravery_), reactions(reactions_), firing(firing_), throwing(throwing_), strength(strength_), psiStrength(psiStrength_), psiSkill(psiSkill_), melee(melee_) {};
	UnitStats& operator+=(const UnitStats& stats) { tu += stats.tu; stamina += stats.stamina; health += stats.health; bravery += stats.bravery; reactions += stats.reactions; firing += stats.firing; throwing += stats.throwing; strength += stats.strength; psiStrength += stats.psiStrength; psiSkill += stats.psiSkill; melee += stats.melee; return *this; }
	UnitStats operator+(const UnitStats& stats) const { return UnitStats(tu + stats.tu, stamina + stats.stamina, health + stats.health, bravery + stats.bravery, reactions + stats.reactions, firing + stats.firing, throwing + stats.throwing, strength + stats.strength, psiStrength + stats.psiStrength, psiSkill + stats.psiSkill, melee + stats.melee); }
	UnitStats& operator-=(const UnitStats& stats) { tu -= stats.tu; stamina -= stats.stamina; health -= stats.health; bravery -= stats.bravery; reactions -= stats.reactions; firing -= stats.firing; throwing -= stats.throwing; strength -= stats.strength; psiStrength -= stats.psiStrength; psiSkill -= stats.psiSkill; melee -= stats.melee; return *this; }
	UnitStats operator-(const UnitStats& stats) const { return UnitStats(tu - stats.tu, stamina - stats.stamina, health - stats.health, bravery - stats.bravery, reactions - stats.reactions, firing - stats.firing, throwing - stats.throwing, strength - stats.strength, psiStrength - stats.psiStrength, psiSkill - stats.psiSkill, melee - stats.melee); }
	UnitStats operator-() const { return UnitStats(-tu, -stamina, -health, -bravery, -reactions, -firing, -throwing, -strength, -psiStrength, -psiSkill, -melee); }
	void merge(const UnitStats& stats) { tu = (stats.tu ? stats.tu : tu); stamina = (stats.stamina ? stats.stamina : stamina); health = (stats.health ? stats.health : health); bravery = (stats.bravery ? stats.bravery : bravery); reactions = (stats.reactions ? stats.reactions : reactions); firing = (stats.firing ? stats.firing : firing); throwing = (stats.throwing ? stats.throwing : throwing); strength = (stats.strength ? stats.strength : strength); psiStrength = (stats.psiStrength ? stats.psiStrength : psiStrength); psiSkill = (stats.psiSkill ? stats.psiSkill : psiSkill); melee = (stats.melee ? stats.melee : melee); };

	template<typename Func>
	static void fieldLoop(Func f)
	{
		constexpr static Ptr allFields[] =
		{
			&UnitStats::tu, &UnitStats::stamina,
			&UnitStats::health, &UnitStats::bravery,
			&UnitStats::reactions, &UnitStats::firing,
			&UnitStats::throwing, &UnitStats::strength,
			&UnitStats::psiStrength, &UnitStats::psiSkill,
			&UnitStats::melee,
		};

		for (Ptr p : allFields)
		{
			f(p);
		}
	}

	static UnitStats percent(const UnitStats& base, const UnitStats& percent, int multipler = 1)
	{
		UnitStats r;
		fieldLoop(
			[&](Ptr p)
			{
				(r.*p) = (base.*p) * (percent.*p) * multipler / 100;
			}
		);
		return r;
	}

	static UnitStats scalar(int i)
	{
		UnitStats r;
		fieldLoop(
			[&](Ptr p)
			{
				(r.*p) = i;
			}
		);
		return r;
	}

	static UnitStats max(const UnitStats& a, const UnitStats& b)
	{
		UnitStats r;
		fieldLoop(
			[&](Ptr p)
			{
				(r.*p) = std::max((a.*p), (b.*p));
			}
		);
		return r;
	}

	static UnitStats min(const UnitStats& a, const UnitStats& b)
	{
		UnitStats r;
		fieldLoop(
			[&](Ptr p)
			{
				(r.*p) = std::min((a.*p), (b.*p));
			}
		);
		return r;
	}

	template<typename T, UnitStats T::*Stat, Ptr StatMax>
	struct getMaxStatScript
	{
		static RetEnum func(const T *t, int& val)
		{
			if (t)
			{
				val = ((t->*Stat).*StatMax);
			}
			else
			{
				val = 0;
			}
			return RetContinue;
		}
	};

	template<typename T, UnitStats T::*Stat, Ptr StatMax>
	struct setMaxStatScript
	{
		static RetEnum func(T *t, int val)
		{
			if (t)
			{
				val = std::min(std::max(val, 1), 1000);
				((t->*Stat).*StatMax) = val;
			}
			return RetContinue;
		}
	};

	template<typename T, UnitStats T::*Stat, int T::*Curr, Ptr StatMax>
	struct setMaxAndCurrStatScript
	{
		static RetEnum func(T *t, int val)
		{
			if (t)
			{
				val = std::min(std::max(val, 1), 1000);
				((t->*Stat).*StatMax) = val;

				//update current value
				if ((t->*Curr) > val)
				{
					(t->*Curr) = val;
				}
			}
			return RetContinue;
		}
	};

	template<typename T, UnitStats T::*Stat, typename TBind>
	static void addGetStatsScript(TBind& b, std::string prefix, bool skipResorcesStats = false)
	{
		if (!skipResorcesStats)
		{
			b.template addFunc<getMaxStatScript<T, Stat, &UnitStats::tu>>(prefix + "getTimeUnits");
			b.template addFunc<getMaxStatScript<T, Stat, &UnitStats::stamina>>(prefix + "getStamina");
			b.template addFunc<getMaxStatScript<T, Stat, &UnitStats::health>>(prefix + "getHealth");
		}

		b.template addFunc<getMaxStatScript<T, Stat, &UnitStats::bravery>>(prefix + "getBravery");
		b.template addFunc<getMaxStatScript<T, Stat, &UnitStats::reactions>>(prefix + "getReactions");
		b.template addFunc<getMaxStatScript<T, Stat, &UnitStats::firing>>(prefix + "getFiring");
		b.template addFunc<getMaxStatScript<T, Stat, &UnitStats::throwing>>(prefix + "getThrowing");
		b.template addFunc<getMaxStatScript<T, Stat, &UnitStats::strength>>(prefix + "getStrength");
		b.template addFunc<getMaxStatScript<T, Stat, &UnitStats::psiStrength>>(prefix + "getPsiStrength");
		b.template addFunc<getMaxStatScript<T, Stat, &UnitStats::psiSkill>>(prefix + "getPsiSkill");
		b.template addFunc<getMaxStatScript<T, Stat, &UnitStats::melee>>(prefix + "getMelee");
	}

	template<typename T, UnitStats T::*Stat, typename TBind>
	static void addSetStatsScript(TBind& b, std::string prefix, bool skipResorcesStats = false)
	{
		if (!skipResorcesStats)
		{
			b.template addFunc<setMaxStatScript<T, Stat, &UnitStats::tu>>(prefix + "setTimeUnits");
			b.template addFunc<setMaxStatScript<T, Stat, &UnitStats::stamina>>(prefix + "setStamina");
			b.template addFunc<setMaxStatScript<T, Stat, &UnitStats::health>>(prefix + "setHealth");
		}

		b.template addFunc<setMaxStatScript<T, Stat, &UnitStats::bravery>>(prefix + "setBravery");
		b.template addFunc<setMaxStatScript<T, Stat, &UnitStats::reactions>>(prefix + "setReactions");
		b.template addFunc<setMaxStatScript<T, Stat, &UnitStats::firing>>(prefix + "setFiring");
		b.template addFunc<setMaxStatScript<T, Stat, &UnitStats::throwing>>(prefix + "setThrowing");
		b.template addFunc<setMaxStatScript<T, Stat, &UnitStats::strength>>(prefix + "setStrength");
		b.template addFunc<setMaxStatScript<T, Stat, &UnitStats::psiStrength>>(prefix + "setPsiStrength");
		b.template addFunc<setMaxStatScript<T, Stat, &UnitStats::psiSkill>>(prefix + "setPsiSkill");
		b.template addFunc<setMaxStatScript<T, Stat, &UnitStats::melee>>(prefix + "setMelee");
	}

	template<typename T, UnitStats T::*Stat, int T::*CurrTu, int T::*CurrEnergy, int T::*CurrHealth, typename TBind>
	static void addSetStatsWithCurrScript(TBind& b, std::string prefix)
	{
		// when we change stats of BattleUnit its resoreces should be adjust.
		b.template addFunc<setMaxAndCurrStatScript<T, Stat, CurrTu, &UnitStats::tu>>(prefix + "setTimeUnits");
		b.template addFunc<setMaxAndCurrStatScript<T, Stat, CurrEnergy, &UnitStats::stamina>>(prefix + "setStamina");
		b.template addFunc<setMaxAndCurrStatScript<T, Stat, CurrHealth, &UnitStats::health>>(prefix + "setHealth");

		addSetStatsScript<T, Stat>(b, prefix, true);
	}
};

struct StatAdjustment
{
	/// Name of class used in script.
	static constexpr const char *ScriptName = "StatAdjustment";
	/// Register all useful function used by script.
	static void ScriptRegister(ScriptParserBase* parser);

	UnitStats statGrowth;
	int growthMultiplier;
	double aimAndArmorMultiplier;
};

class Mod;
class Armor;
class ModScript;

/**
 * Represents the static data for a unit that is generated on the battlescape, this includes: HWPs, aliens and civilians.
 * @sa Soldier BattleUnit
 */
class Unit
{
private:
	std::string _type, _civilianRecoveryType;
	std::string _race;
	int _showFullNameInAlienInventory;
	std::string _rank;
	UnitStats _stats;
	std::string _armorName;
	Armor* _armor;
	int _standHeight, _kneelHeight, _floatHeight;
	std::vector<int> _deathSound, _panicSound, _berserkSound;
	int _value, _moraleLossWhenKilled, _aggroSound, _moveSound;
	int _intelligence, _aggression, _spotter, _sniper, _energyRecovery;
	SpecialAbility _specab;
	std::string _spawnUnit;
	bool _livingWeapon;
	std::string _meleeWeapon, _psiWeapon;
	std::vector<std::vector<std::string> > _builtInWeapons;
	bool _capturable;
	bool _canSurrender, _autoSurrender;
	bool _isLeeroyJenkins;
	bool _waitIfOutsideWeaponRange;
	int _pickUpWeaponsMoreActively;

public:
	/// Creates a blank unit ruleset.
	Unit(const std::string &type);
	/// Cleans up the unit ruleset.
	~Unit();
	/// Loads the unit data from YAML.
	void load(const YAML::Node& node, Mod *mod);
	/// Cross link with other rules.
	void afterLoad(const Mod* mod);

	/// Gets the unit's type.
	std::string getType() const;
	/// Gets the type of staff (soldier/engineer/scientists) or type of item to be recovered when a civilian is saved.
	std::string getCivilianRecoveryType() const;
	/// Gets the unit's stats.
	UnitStats *getStats();
	/// Gets the unit's height when standing.
	int getStandHeight() const;
	/// Gets the unit's height when kneeling.
	int getKneelHeight() const;
	/// Gets the unit's float elevation.
	int getFloatHeight() const;
	/// Gets the armor type.
	Armor* getArmor() const;
	/// Gets the alien race type.
	std::string getRace() const;
	/// Gets the alien rank.
	std::string getRank() const;
	/// Gets the value - for score calculation.
	int getValue() const;
	/// Percentage modifier for morale loss when this unit is killed.
	int getMoraleLossWhenKilled() { return _moraleLossWhenKilled; };
	/// Gets the death sound id.
	const std::vector<int> &getDeathSounds() const;
	/// Gets the unit's panic sounds.
	const std::vector<int> &getPanicSounds() const;
	/// Gets the unit's berserk sounds.
	const std::vector<int> &getBerserkSounds() const;
	/// Gets the move sound id.
	int getMoveSound() const;
	/// Gets the intelligence. This is the number of turns AI remembers your troop positions.
	int getIntelligence() const;
	/// Gets the aggression. Determines the chance of revenge and taking cover.
	int getAggression() const;
	/// Gets the spotter score. This is the number of turns sniper AI units can use spotting info from this unit.
	int getSpotterDuration() const;
	/// Gets the sniper score. Determines chance of acting on information gained by spotter units.
	int getSniperPercentage() const;
	/// Gets the alien's special ability.
	int getSpecialAbility() const;
	/// Gets the unit's spawn unit.
	std::string getSpawnUnit() const;
	/// Gets the unit's war cry.
	int getAggroSound() const;
	/// Gets how much energy this unit recovers per turn.
	int getEnergyRecovery() const;
	/// Checks if this unit has a built in weapon.
	bool isLivingWeapon() const;
	/// Gets the name of any melee weapon that may be built in to this unit.
	const std::string &getMeleeWeapon() const;
	/// Gets the name of any psi weapon that may be built in to this unit.
	const std::string &getPsiWeapon() const;
	/// Gets a vector of integrated items this unit has available.
	const std::vector<std::vector<std::string> > &getBuiltInWeapons() const;
	/// Gets whether the alien can be captured alive.
	bool getCapturable() const;
	/// Checks if this unit can surrender.
	bool canSurrender() const;
	/// Checks if this unit surrenders automatically, if all other units surrendered too.
	bool autoSurrender() const;
	bool isLeeroyJenkins() const { return _isLeeroyJenkins; };
	/// Should the unit get "stuck" trying to fire from outside of weapon range? Vanilla bug, that may serve as "feature" in rare cases.
	bool waitIfOutsideWeaponRange() { return _waitIfOutsideWeaponRange; };
	/// Should the unit try to pick up weapons more actively?
	bool pickUpWeaponsMoreActively(Mod *mod) const;
	/// Should alien inventory show full name (e.g. Sectoid Leader) or just the race (e.g. Sectoid)?
	bool getShowFullNameInAlienInventory(Mod *mod) const;
};

}

namespace YAML
{
	template<>
	struct convert<OpenXcom::UnitStats>
	{
		static Node encode(const OpenXcom::UnitStats& rhs)
		{
			Node node;
			node["tu"] = rhs.tu;
			node["stamina"] = rhs.stamina;
			node["health"] = rhs.health;
			node["bravery"] = rhs.bravery;
			node["reactions"] = rhs.reactions;
			node["firing"] = rhs.firing;
			node["throwing"] = rhs.throwing;
			node["strength"] = rhs.strength;
			node["psiStrength"] = rhs.psiStrength;
			node["psiSkill"] = rhs.psiSkill;
			node["melee"] = rhs.melee;
			return node;
		}

		static bool decode(const Node& node, OpenXcom::UnitStats& rhs)
		{
			if (!node.IsMap())
				return false;

			rhs.tu = node["tu"].as<int>(rhs.tu);
			rhs.stamina = node["stamina"].as<int>(rhs.stamina);
			rhs.health = node["health"].as<int>(rhs.health);
			rhs.bravery = node["bravery"].as<int>(rhs.bravery);
			rhs.reactions = node["reactions"].as<int>(rhs.reactions);
			rhs.firing = node["firing"].as<int>(rhs.firing);
			rhs.throwing = node["throwing"].as<int>(rhs.throwing);
			rhs.strength = node["strength"].as<int>(rhs.strength);
			rhs.psiStrength = node["psiStrength"].as<int>(rhs.psiStrength);
			rhs.psiSkill = node["psiSkill"].as<int>(rhs.psiSkill);
			rhs.melee = node["melee"].as<int>(rhs.melee);
			return true;
		}
	};
}
