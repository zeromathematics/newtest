--团长
sgs.ai_skill_invoke.mengxian = function(self, data)
	return self:willShowForDefence() or self:willShowForAttack()
end

sgs.ai_skill_invoke.yuanwang = function(self, data)
	return self:willShowForDefence() or self:willShowForAttack()
end

sgs.ai_skill_choice.yuanwang = function(self, choices, data)
	local haruhi = self.room:findPlayerBySkillName("yuanwang")
	if not haruhi then return "cancel" end
	if self:isFriend(haruhi) then return "yuanwang_accept" end
	return "yuanwang_accept"
end

sgs.ai_skill_playerchosen.yuanwang = function(self, targets)
	for _,p in sgs.qlist(targets) do
		if self:isFriend(p) then return p end
	end
	return
end

sgs.ai_skill_invoke.yuanwangsos = function(self, data)
    local can
    for _,c in sgs.qlist(self.player:getHandcards()) do
	   if c:isBlack() then
	      can = true
	   end 
	end
	return can
end

sgs.ai_skill_askforag.yuanwang = function(self, card_ids)
   for _,c in sgs.qlist(self.player:getHandcards()) do
	   if c:isBlack() then
	      return c:getEffectiveId()
	   end 
   end
end

--创真
sgs.ai_skill_invoke.pengtiao = function(self, data)
	return self:willShowForDefence() or self:willShowForAttack()
end

sgs.ai_skill_use["@@pengtiao"] = function(self, prompt)
    local current = self.room:getCurrent()
	if current and not self:isFriend(current) then
		return "."
	end
	if (self.player:isRemoved()) then
	    return "."
	end
    for _,f in ipairs(self.friends_noself) do
	  if f:objectName()~=current:objectName() and not self.player:isWounded() and f:isWounded() and not f:hasFlag("pengtiao_cancel") and not f:isRemoved() then
	    return "."
	  end
	end
	local cards=sgs.QList2Table(self.player:getHandcards())
	local equips=sgs.QList2Table(self.player:getEquips())
	local needed = {}
	for _,acard in ipairs(cards) do
		if #needed == 0 and (acard:isKindOf("Peach") or acard:isKindOf("Analeptic") or acard:getNumber()==13 or acard:getSubtype()=="food_card") then
			table.insert(needed, acard:getEffectiveId())
		end
	end
	for _,acard in ipairs(equips) do
		if #needed == 0 and (acard:isKindOf("Peach") or acard:isKindOf("Analeptic") or acard:getNumber()==13 or acard:getSubtype()=="food_card") then
			table.insert(needed, acard:getEffectiveId())
		end
	end
	if #needed==1 then
		return ("@PengtiaoCard="..table.concat(needed, "+").."&")
	end
	return "."
end

sgs.ai_skill_invoke.shiji = function(self, data)
	local use = data:toCardUse()
	local cards=sgs.QList2Table(self.player:getHandcards())
	local OK = false
	for _,card in ipairs(cards) do
		if card:getNumber() > 6 then
			OK =true
		end
	end
	if (self:isEnemy(use.from)) then
        return OK or self.player:getHp() <= use.from:getHp()     
    end

    return false	
end

sgs.ai_skill_invoke.shiji_recover = function(self, data)
	return true
end

sgs.ai_skill_invoke.pengtiao_recover = function(self, data)
	local dest = data:toPlayer()
	if (dest and self:isFriend(dest)) then
       return true	
	end
	return false
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

--艾拉
sgs.ai_skill_invoke.kioku = function(self, data)
	return self:willShowForDefence() or self:willShowForAttack()
end

sgs.ai_skill_playerchosen.kioku = function(self, targets, max_num, min_num)
    for _, target in sgs.qlist(targets) do	     
		if self.player:isFriendWith(target) then return target end
	end
	for _, target in sgs.qlist(targets) do	     
		if self:isFriend(target) then return target end
	end
end

local xiangsui_skill={}
xiangsui_skill.name="xiangsui"
table.insert(sgs.ai_skills,xiangsui_skill)
xiangsui_skill.getTurnUseCard=function(self,inclusive)
    if self.player:hasUsed("XiangsuiCard") or self.player:getPile("memory"):length()==0 then return false end
	return sgs.Card_Parse("@XiangsuiCard=.&xiangsui")
end

sgs.ai_skill_use_func.XiangsuiCard = function(card,use,self)
	local target
	local n=-1
	for _,f in ipairs(self.friends) do
		if f:getLostHp()>n then
			n = f:getLostHp()
		end
	end
	for _,f in sgs.list(self.friends) do
		if f:getLostHp()==n then
			target = f
		end
	end
	local needed
	--local cards = sgs.QList2Table(self.player:getPile("memory"))
	needed = self.player:getPile("memory"):at(0)
	if target and needed then
		use.card = sgs.Card_Parse("@XiangsuiCard="..needed.."&xiangsui")
		if use.to then use.to:append(target) end
		return
	end
end

--未来
sgs.ai_skill_invoke.zhouxue = function(self, data)
	return true
end

sgs.ai_skill_invoke.caoxue = function(self, data)
    local damage = data:toDamage()
	return (self:isEnemy(damage.to) and not damage.to:isNude()) or (self:isFriend(damage.to) and damage.to:getJudgingArea():length()>0) 
end

local xueren_skill = {}
xueren_skill.name = "xueren"
table.insert(sgs.ai_skills, xueren_skill)
xueren_skill.getTurnUseCard = function(self)
	if self.player:getPile("zhouxue_blood"):isEmpty() or self.player:hasFlag("xueren_used") then
		return
	end
	self:sort(self.enemies, "defense")
	local useAll = false
	for _, enemy in ipairs(self.enemies) do
		if enemy:getHp() == 1 and not enemy:hasArmorEffect("EightDiagram") and self.player:distanceTo(enemy) <= self.player:getAttackRange() and self:isWeak(enemy)
			and getCardsNum("Jink", enemy, self.player) + getCardsNum("Peach", enemy, self.player) + getCardsNum("Analeptic", enemy, self.player) == 0 then
			useAll = true
			break
		end
	end

	local disCrossbow = false
	if self:getCardsNum("Slash") < 2 or self.player:hasSkill("paoxiao") then disCrossbow = true end
	
	local can_use = false
	local cards = {}
	for i = 0, self.player:getPile("zhouxue_blood"):length() - 1, 1 do
		local slash = sgs.Sanguosha:getCard(self.player:getPile("zhouxue_blood"):at(i))
		local slash_str = ("slash:xueren[%s:%s]=%d&xueren"):format(slash:getSuitString(), slash:getNumberString(), self.player:getPile("zhouxue_blood"):at(i))
		local xuerenslash = sgs.Card_Parse(slash_str)
		assert(xuerenslash)
        if self:slashIsAvailable(self.player, xuerenslash) then
			table.insert(cards, xuerenslash)
		end
	end
	if #cards == 0 then return end
	return cards[1]
end

sgs.ai_view_as.xueren = function(card, player, card_place)
    if player:hasFlag("xueren_used") then
		return
	end
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local ask = sgs.Sanguosha:getCurrentCardUsePattern()
	if card_place == sgs.Player_PlaceSpecial and player:getPileName(card_id) == "zhouxue_blood" and ask == "slash" then
		return ("slash:xueren[%s:%s]=%d%s"):format(suit, number, card_id, "&xueren")
	end
	if card_place == sgs.Player_PlaceSpecial and player:getPileName(card_id) == "zhouxue_blood" and ask == "jink" then
		return ("jink:xueren[%s:%s]=%d%s"):format(suit, number, card_id, "&xueren")
	end
end

sgs.ai_skill_choice["zhouxue"] = function(self, choices, data)
	return "zhouxue_yes"
end

--艾露莎
sgs.ai_skill_invoke.huanzhuang = function(self, data)
	return self:willShowForDefence() or self:willShowForAttack()
end

--饼
function SmartAI:useCardTacos(card, use)
	--[[for _,p in ipairs(self.enemies) do
		if p:hasSkill("eastfast") then return end
	end
	for _,p in ipairs(self.friends) do
		if p:hasSkill("SE_Jiawu") then return end
	end]]
	use.card = card
end

sgs.ai_card_intention.Tacos = -40

sgs.ai_keep_value.Tacos = 2.5
sgs.ai_use_value.Tacos = 8
sgs.ai_use_priority.Tacos = 4

sgs.dynamic_value.benefit.Tacos = true

--辣
function SmartAI:useCardMapoTofu(card, use)
	local targets = {}
	for _,p in sgs.list(self.room:getAlivePlayers()) do
		if self.player:distanceTo(p) <= 1 then table.insert(targets, p) end
	end
	if #targets == 0 then return end
	local f_target
	for _,target in ipairs(targets) do
		if self:isFriend(target) then
			if target:hasSkills(sgs.masochism_by_self_skill) then f_target = target end
			if target:hasSkills("tianhuo") and target:getLostHp() > 0 then return target end
		else
			if self.player:hasSkills(sgs.weak_killer_skill) and target:getLostHp() == 0 then f_target = target end
		end
	end
	if not f_target then
		for _,target in ipairs(targets) do
			if self:isFriend(target) and target:getLostHp() > 0 then
				f_target = target
			end
		end
	end
	if f_target then
		for _,v in ipairs(self.enemies) do
			if v:objectName() == f_target:objectName() then
				use.card = card
				if use.to and not (self.room:isProhibited(self.player, v, card) or v:isRemoved()) then use.to:append(v) end
				return
			end
		end
	end
end
sgs.ai_use_priority.MapoTofu = 10
sgs.ai_use_value.MapoTofu = 8
sgs.ai_keep_value.MapoTofu = 1.0
sgs.ai_card_intention.MapoTofu = 0

--永琳
local penglai_skill={}
penglai_skill.name="penglai"
table.insert(sgs.ai_skills,penglai_skill)
penglai_skill.getTurnUseCard=function(self,inclusive)
    if self.player:hasUsed("PenglaiCard") then return false end
	return sgs.Card_Parse("@PenglaiCard=.&penglai")
end

sgs.ai_skill_use_func.PenglaiCard = function(card,use,self)
	local target
	for _,f in ipairs(self.friends_noself) do
		if f:getLostHp()>0 and f:getHp() == 1 and not f:isKongcheng() then
			target = p
			break
		end
	end
	if not target then
	   for _,f in sgs.list(self.friends_noself) do
		if f:getLostHp()>0 and f:getHandcardNum()>1 then
			target = p
			break
		end
	   end
	end
	if not target then
	   for _,e in sgs.list(self.enemies) do
		if e:getLostHp()==0 and not e:isKongcheng() then
			target = e
			break
		end
	   end
	end
	if target then
		use.card = sgs.Card_Parse("@PenglaiCard=.&penglai")
		if use.to then use.to:append(target) end
		return
	end
end

sgs.ai_skill_invoke.jiansi = function(self, data)
	return self:willShowForDefence() or self:willShowForAttack()
end

sgs.ai_skill_use["@@jiansi"] = function(self, prompt)
   local pattern = self.player:property("jiansi_card"):toString()
   local id = self.player:property("jiansi_number"):toInt()
   local card = sgs.Sanguosha:getCard(id)
   return sgs.Card_Parse(pattern..":jiansi["..card:getSuitString()..":"..card:getNumberString().."]=" .. id .."&jiansi")
end

sgs.ai_skill_askforag.jiansi = function(self, card_ids)
   for _,id in ipairs(card_ids) do
       local card = sgs.Sanguosha:getCard(id)
	   if card:isAvailable(self.player) then
	      return id
	   end 
   end
end