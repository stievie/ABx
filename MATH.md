## Drop Chance
### https://old.reddit.com/r/gamedev/comments/8z011r/item_rarity_in_rpgs/e2fp1b6/
The formula for calculating the chance of at least one item dropping for a given drop chance is:

Chance = 1 - (1 - DropChance) ^ NumTries

So if you want to say that there's a 80% chance that they'll get the drop after 10 tries, you can plug in those numbers to get:

.8 = 1 - (1 - DropChance) ^ 10

DropChance = .14866 (or 14.866% chance to drop for each try)

### https://www.reddit.com/r/Unity3D/comments/1y7aqf/random_drop_chance_concepts/