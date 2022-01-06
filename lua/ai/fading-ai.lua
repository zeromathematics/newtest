--团长
sgs.ai_skill_invoke.mengxian = function(self, data)
	return self:willShowForDefence() or self:willShowForAttack()
end





--间宫明里
sgs.ai_skill_invoke.Takamakuri = function(self, data)
	local damage = data:toDamage()
	if not damage or not damage.to then return false end
	if self:isEnemy(damage.to) and not damage.to:hasSkills(sgs.lose_equip_skill) then return true end
	return false
end

sgs.ai_skill_invoke.Tobiugachi = function(self, data)
	if self:isFriend(self.room:getCurrent()) then
		if self.player:getHandcardNum() - self.player:getHp() + 1 <= 3 and (self:isWeak() or self:getCardsNum("Jink") == 0) then return true end
		return false
	end
	if self.player:getHandcardNum() - self.player:getHp() + 1 <= 5 then return true end
	return false
end

sgs.ai_skill_playerchosen.Tobiugachi = function(self, targets)
	local target = self:findPlayerToDiscard()
	if target then return target end
	return self.enemies[1]
end

sgs.ai_skill_invoke.Fukurouza = function(self, data)
	return true
end

sgs.ai_skill_invoke.FukurouzaTobi = function(self, data)
	if self:isEnemy(self.room:getCurrent()) and not self.room:getCurrent():hasSkills(sgs.immune_skill) then return true end
	return false
end

sgs.ai_skill_invoke.FukurouzaTaka = true