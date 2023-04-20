# ProjectBravo
A first-person shooter with an over-engineered weapon system.

https://youtu.be/RiWiFti6vYg

Player character:
The damage that the player receives is divided between armor and life health.
In addition, damage is reduced in several cases:
-When the player takes damage from the back.
-When armor points are greater than 1.
-When health points are low.
-When the current weapon has a shield.
Damage reductions can be combined.
Enemies drop health pickups when the player is low on health.
The player has a dodge whose distance and duration can be freely configured.

Player Weapon System: (doesn't use GAS)
Weapon attack pattern can be freely configured to create unique attacks.
You can create simple setups with one bullet attack, bursts of several bullets, or fire multiple bullets in a horizontal line.
You can also create a more complicated pattern, e.g. the first two attacks are a series of three bullets, and the third one is five bullets in a horizontal line.
The attack pattern can be reset when reloading or switching weapons if necessary.
Weapon projectiles are divided into three categories: standard, ricocheting, which have a chance to bounce off an enemy by hitting another, and grenades that deal area damage.
Grenades can have additional effects such as slow, ignite, or corrosive acid.
When the player manually reloads the weapon, missing bullets in the magazine will deal more damage after reloading.
When reloading a weapon, the player can switch to another weapon. A weapon that is reloaded activates a reload in the pocket. The weapon will complete its reload, but it will take longer. The player will be able to select this weapon once it has finished reloading in the pocket.
The weapon may have the option to reload with a grenade. The grenade deals damage based on the bullets in the magazine.
The weapon may have a shield that absorbs damage. Absorbed damage subtracts ammo from the magazine.

Enemies Description: (doesn't use BT)
Enemies can drop resource pickups when they die. What they drop depends on the weapon they died with.
When an enemy takes damage from a player, it takes the weapon type's damage value from its damage table and multiplies it by the scale. This allows to design enemies that are immune or vulnerable to a player's specific weapon.
Each enemy can have any number of skills divided into three types (ranged, melee and scanning). In addition, each skill can be combined with another to create a combo. Skills also have movement modes such as following the player, moving to a random point in the arena or within the player's radius.

Ranged skills use projectiles to deal damage.

Melee abilities deal damage when the hitbox hits the player. The hitbox travels along the path defined by the Spline Component. This gives the ability to design any attack pattern. During the duration of the attack, damage is dealt only once, regardless of how much time the player is in the attack path.

Scanning abilities deal damage with Line Trace, ignoring obstacles.
