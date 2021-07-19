sgs.ai_skill_choice.transform = function(self, choices, data)
    if math.random(1,2) == 1 or self.player:getGeneral2Name()=="sujiang" or self.player:getGeneral2Name()=="sujiangf" then
	 return "transform"
	else
	 return "cancel"
	end
end

function SmartAI:useCardKey(card, use)
    local room=self.room
	for _,v in ipairs(self.friends) do
	    local keys=0
	    for _,c in sgs.qlist(v:getJudgingArea()) do
		  if c:isKindOf("Key") then
		    keys=keys+1
		  end
		end
		if v:getLostHp() > 0 and not v:containsTrick("keyCard") then
			use.card = card
			if use.to then use.to:append(v) end
			return
		end
	end
	for _,v in ipairs(self.friends) do
	    local keys=0
	    for _,c in sgs.qlist(v:getJudgingArea()) do
		  if c:isKindOf("Key") then
		    keys=keys+1
		  end
		end
		if not v:containsTrick("keyCard") then
			use.card = card
			if use.to then use.to:append(v) end
			return
		end
	end
end
sgs.ai_use_priority.Key = 3
sgs.ai_use_value.Key = 2
sgs.ai_keep_value.Key = 2
sgs.ai_card_intention.Key = -50

function needToRemoveKey(player)
  return player:containsTrick("keyCard") and player:isWounded()
end

function noNeedToRemoveJudgeArea(player)
  local noothers = true
  for _,c in sgs.qlist(player:getJudgingArea()) do
    if c:objectName()~=("keyCard") then noothers =false end
  end
  return player:containsTrick("keyCard") and not player:isWounded() and noothers
end

zhuren_skill={}
zhuren_skill.name="zhuren"
table.insert(sgs.ai_skills,zhuren_skill)
zhuren_skill.getTurnUseCard=function(self,inclusive)
	if #self.friends <= 1 then return end
	local source = self.player
	if source:isNude() then return end
	if source:hasUsed("ZhurenCard") then return end
	return sgs.Card_Parse("@ZhurenCard=.&zhuren")
end

sgs.ai_skill_use_func.ZhurenCard = function(card,use,self)
	local target
	local source = self.player
	local keys = 0
	for _,card in sgs.qlist(source:getJudgingArea()) do
		if card:isKindOf("Key") then
			keys = keys + 1
		end
	end
	local max_num = source:getMaxHp() - source:getHp() + keys
	local max_x = 0
	for _,friend in ipairs(self.friends) do
		local x = 5 - friend:getHandcardNum()

		if x > max_x and friend:objectName() ~= source:objectName() then
			max_x = x
			target = friend
		end
	end
	local cards=sgs.QList2Table(self.player:getHandcards())
	local equips=sgs.QList2Table(self.player:getEquips())
	local needed = {}
	for _,acard in ipairs(cards) do
		if #needed < max_num then
			table.insert(needed, acard:getEffectiveId())
		end
	end
	for _,acard in ipairs(equips) do
		if #needed < max_num then
			table.insert(needed, acard:getEffectiveId())
		end
	end
	if target and #needed>0 then
		use.card = sgs.Card_Parse("@ZhurenCard="..table.concat(needed,"+").."&zhuren")
		if use.to then use.to:append(target) end
		return
	end
end

sgs.ai_use_value.ZhurenCard = 4
sgs.ai_use_priority.ZhurenCard  = 2.4
sgs.ai_card_intention.ZhurenCard  = -60

sgs.ai_skill_invoke.newshenzhi = function(self, data)
	if not self:willShowForDefence() and not self:willShowForAttack() and self.player:getJudgingArea():isEmpty() then
		return false
	end
	return true
end

gonglue_skill={}
gonglue_skill.name="gonglue"
table.insert(sgs.ai_skills,gonglue_skill)
gonglue_skill.getTurnUseCard=function(self,inclusive)
	if #self.enemies < 1 or self.player:hasUsed("GonglueCard") then return end
	if not self:willShowForAttack() and not self:willShowForDefence() then return end
	return sgs.Card_Parse("@GonglueCard=.&gonglue")
end

sgs.ai_skill_use_func.GonglueCard = function(card,use,self)
	local target
	local card
	local cards = sgs.QList2Table(self.player:getHandcards())
	for _,acard in ipairs(cards) do
		if self:getKeepValue(acard)<5 then
			card = acard
		end
    end
	for _,acard in ipairs(sgs.QList2Table(self.player:getEquips())) do
		if self:getKeepValue(acard)<5 then
			card = acard
		end
    end
	self:sort(self.enemies, "defense")
		for _,enemy in ipairs(self.enemies) do
			if not enemy:isKongcheng() then
				target = enemy
			end
		end
	if target and card then
		use.card = sgs.Card_Parse("@GonglueCard="..card:getEffectiveId().."&gonglue")
		if use.to then use.to:append(target) end
		return
	end
end

sgs.ai_use_value.GonglueCard = 8
sgs.ai_use_priority.GonglueCard = 8
sgs.ai_card_intention.GonglueCard = 60

sgs.ai_skill_invoke.lichang = function(self, data)
	return true
end

sgs.ai_skill_invoke.kongni = function(self, data)
	return true
end

sgs.ai_skill_invoke.wucun = function(self, data)
	if self:willShowForDefence() then 
		return true 
	end
	return false
end



--手刃实验

shouren_skill={}
shouren_skill.name="shouren"
table.insert(sgs.ai_skills,shouren_skill)
shouren_skill.getTurnUseCard=function(self,inclusive)
	local source = self.player -- 来源
	if not self:willShowForAttack() then return end
	if source:hasUsed("ShourenCard") then return end -- 如果来源用过了这个技能，就不能再用了（因为出限1）
	if #self.enemies == 0 then return end -- 没有敌人的话不必用了
	if source:isKongcheng() then return end
	--关于敌人的判断，self.enemies是一个table，对于单个角色用self:isEnemy(dest)来判断
	-- 这里经常会判断的还有，如果需要手牌的技能，那么没手牌就别用了之类的

	--万事俱备，可以走下一个函数了
	return sgs.Card_Parse("@ShourenCard=.&shouren")
end

sgs.ai_skill_use_func.ShourenCard = function(card,use,self)
	local target --预备一个目标
	local minHp = 100  -- 当前场上的最低体力
	for _,enemy in ipairs(self.enemies) do
		local hp = enemy:getHp()
		if self:hasSkills(sgs.masochism_skill, enemy) then hp = hp - 1 end  -- 如果这人有卖血技能，因为是体力流失，所以价值更高，少算这个人一血
		if hp < minHp and enemy:getHp()>=self.player:getHp() then --如果这人比最低体力少
			minHp = hp --更新最低体力
			target = enemy  --目标换成这个人
		end
	end
	-- 这样就获得了场上最适合被手刃的人
    local card
	local cards = sgs.QList2Table(self.player:getHandcards())
	for _,acard in ipairs(cards) do
		if self:getKeepValue(acard)<5 then
			card = acard
		end
    end
	-- 注意：这是老版本的手刃，现在的版本翻人会失去立场，应该考虑重写这个判断
	if target and card then --如果找到了目标
		use.card = sgs.Card_Parse("@ShourenCard="..card:getEffectiveId().."&shouren")   --用的卡是手刃
		if use.to then use.to:append(target) end  --把目标加入用的角色，就可以了
		return
	end
end

sgs.ai_use_value.ShourenCard = 8  --这个技能卡的价值
sgs.ai_use_priority.ShourenCard = 2  --回合内使用这个牌的优先度，手刃应该属于比较靠后的，10到0
sgs.ai_card_intention.ShourenCard  = 60 --这个牌的敌意，-100到100，负数是好意，正数是敌意

qiyuan_skill={}
qiyuan_skill.name="qiyuan"
table.insert(sgs.ai_skills,qiyuan_skill)
qiyuan_skill.getTurnUseCard=function(self,inclusive)
	local source = self.player
	if source:getMark("@se_qiyuan") == 0 then return end
	local room = source:getRoom()
	local deathplayer = {}
	for _,p in sgs.qlist(room:getPlayers()) do
		if p:isDead() and p:isFriendWith(source) then
			table.insert(deathplayer,p:getGeneralName())
		end
	end
	if #deathplayer==0 then return end
	if #deathplayer>0 then
		return sgs.Card_Parse("@QiyuanCard=.&qiyuan")
	end
end

sgs.ai_skill_use_func.QiyuanCard = function(card,use,self)
	use.card = sgs.Card_Parse("@QiyuanCard=.&qiyuan")
	return
end

sgs.ai_use_value.QiyuanCard  = 8
sgs.ai_use_priority.QiyuanCard = 7

sgs.ai_skill_playerchosen.lichang = function(self, targets)
  return targets[math.random(1,#targets)]
end

--逢坂大河（有待提高）
sgs.ai_skill_invoke.zhudao = function(self , data)
    if not (self:willShowForAttack() or self:willShowForDefence()) then
		 return false
	end
    return true
end
--来源
sgs.ai_skill_playerchosen.Laiyuan = function(self, targets)
	local source = self.player

	for _,player in ipairs(self.friends) do
		if player:isAlive() and player:getJudgingArea():length() > 0 then
			return player
		end
	end

	for _,player in ipairs(self.enemies) do
		if player:isAlive() and (player:hasShownSkill("liegong") or player:hasShownSkill("Zhena")) and player:getWeapon() then
			return player
		end
	end

	if not source:getArmor() then
		for _,player in ipairs(self.enemies) do
			if player:isAlive() and player:getArmor() and not player:hasShownSkill("xuanfeng|xiaoji") then
				return player
			end
		end
	end
	if not source:getDefensiveHorse() then
		for _,player in ipairs(self.enemies) do
			if player:isAlive() and player:getDefensiveHorse() and not player:hasShownSkill("xuanfeng|xiaoji") then
				return player
			end
		end
	end
	if not source:getWeapon() then
		for _,player in ipairs(self.enemies) do
			if player:isAlive() and player:getWeapon() and not player:hasShownSkill("xuanfeng|xiaoji") then
				return player
			end
		end
	end
	if not source:getOffensiveHorse() then
		for _,player in ipairs(self.enemies) do
			if player:isAlive() and player:getOffensiveHorse() and not player:hasShownSkill("xuanfeng|xiaoji") then
				return player
			end
		end
	end

	if #self.enemies == 1 then
		for _,badpeople in ipairs(self.enemies) do
			if badpeople:isAlive() and not badpeople:hasShownSkill("kongcheng") then
				return badpeople
			end
		end
	end


	for _,player in ipairs(self.friends) do
		if player:isAlive() and not player:getWeapon() then
			for _,badpeople in ipairs(self.enemies) do
				if badpeople:isAlive() and badpeople:getWeapon() then
					return badpeople
				end
			end
		end
	end
	for _,player in ipairs(self.friends) do
		if player:isAlive() and not player:getOffensiveHorse() then
			for _,badpeople in ipairs(self.enemies) do
				if badpeople:isAlive() and badpeople:getOffensiveHorse() then
					return badpeople
				end
			end
		end
	end
	for _,player in ipairs(self.friends) do
		if player:isAlive() and not player:getArmor() then
			for _,badpeople in ipairs(self.enemies) do
				if badpeople:isAlive() and badpeople:getArmor() then
					return badpeople
				end
			end
		end
	end
	for _,player in ipairs(self.friends) do
		if player:isAlive() and not player:getDefensiveHorse() then
			for _,badpeople in ipairs(self.enemies) do
				if badpeople:isAlive() and badpeople:getDefensiveHorse() then
					return badpeople
				end
			end
		end
	end

	for _,player in ipairs(self.enemies) do
		if player:isAlive() and not player:isKongcheng() then
			return player
		end
	end
	return
end
--竹刀的选牌
sgs.ai_skill_cardchosen.zhudao = function(self, who, flags)
	local source = self.player

	if self:isFriend(who) and who:getJudgingArea():length() > 0 then
		local cards = who:getJudgingArea()
		return cards[1]
	end

	if self:isEnemy(who) and who:hasShownSkill("liegong|Zhena") and who:isAlive() and who:getWeapon() and not who:hasShownSkill("xuanfeng|xiaoji") then
		for _,player in ipairs(self.friends) do
			if player:isAlive() and not player:getWeapon() then
				local card = who:getWeapon()
				return card
			end
		end
	end

	if not ((not source:getArmor() and who:getArmor()) or (not source:getTreasure() and who:getTreasure()) or (not source:getDefensiveHorse() and who:getDefensiveHorse()) or (not source:getWeapon() and who:getWeapon()) or (not source:getOffensiveHorse() and who:getOffensiveHorse())) then
		if self:isEnemy(who) and who:isAlive() and not who:hasShownSkill("kongcheng") and not who:isKongcheng() and #self.enemies==1 then
			local cards = who:getHandcards()
			return cards[1]
		end
	end



	if self:isEnemy(who) and who:isAlive() and who:getArmor() and not who:hasShownSkill("xuanfeng|xiaoji") then
		for _,player in ipairs(self.friends) do
			if player:isAlive() and not player:getArmor() then
				local card = who:getArmor()
				return card
			end
		end
	end
	if self:isEnemy(who) and who:isAlive() and who:getTreasure() and not who:hasShownSkill("xuanfeng|xiaoji") then
		for _,player in ipairs(self.friends) do
			if player:isAlive() and not player:getTreasure() then
				local card = who:getTreasure()
				return card
			end
		end
	end
	if self:isEnemy(who) and who:isAlive() and who:getDefensiveHorse() and not who:hasShownSkill("xuanfeng|xiaoji") then
		for _,player in ipairs(self.friends) do
			if player:isAlive() and not player:getDefensiveHorse() then
				local card = who:getDefensiveHorse()
				return card
			end
		end
	end
	if self:isEnemy(who) and who:isAlive() and who:getWeapon() and not who:hasShownSkill("xuanfeng|xiaoji") then
		for _,player in ipairs(self.friends) do
			if player:isAlive() and not player:getWeapon() then
				local card = who:getWeapon()
				return card
			end
		end
	end
	if self:isEnemy(who) and who:isAlive() and who:getOffensiveHorse() and not who:hasShownSkill("xuanfeng|xiaoji") then
		for _,player in ipairs(self.friends) do
			if player:isAlive() and not player:getOffensiveHorse() then
				local card = who:getOffensiveHorse()
				return card
			end
		end
	end

	if self:isEnemy(who) and who:isAlive() and not who:isKongcheng() then
		local cards = who:getHandcards()
		return cards[1]
	end
	return nil
end

--给予
sgs.ai_skill_playerchosen.Quxiang = function(self, targets)
	local source = self.player

	for _,player in ipairs(self.friends) do
		if player:isAlive() and player:getJudgingArea():length() > 0 then
			for _, target in sgs.qlist(targets) do
				if self:isEnemy(target) and target:getJudgingArea():length() == 0 then
					return target
				end
			end
		end
	end

	for _,player in ipairs(self.enemies) do
		if player:isAlive() and (player:hasShownSkill("liegong") or player:hasShownSkill("Zhena")) and player:getWeapon() then
			for _,goodguy in sgs.qlist(targets) do
				if self:isFriend(goodguy) and not goodguy:getWeapon() then
					return goodguy
				end
			end
		end
	end

	if not source:getArmor() then
		for _,player in sgs.qlist(targets) do
			if self:isEnemy(player) and player:getArmor() and not player:hasShownSkill("xuanfeng|xiaoji") then
				return source
			end
		end
	end
	if not source:getTreasure() then
		for _,player in sgs.qlist(targets) do
			if self:isEnemy(player) and player:getTreasure() and not player:hasShownSkill("xuanfeng|xiaoji") then
				return source
			end
		end
	end
	if not source:getDefensiveHorse() then
		for _,player in sgs.qlist(targets) do
			if self:isEnemy(player) and player:getDefensiveHorse() and not player:hasShownSkill("xuanfeng|xiaoji") then
				return source
			end
		end
	end
	if not source:getWeapon() then
		for _,player in sgs.qlist(targets) do
			if self:isEnemy(player) and player:getWeapon() and not player:hasShownSkill("xuanfeng|xiaoji") then
				return source
			end
		end
	end
	if not source:getOffensiveHorse() then
		for _,player in sgs.qlist(targets) do
			if self:isEnemy(player) and player:getOffensiveHorse() and not player:hasShownSkill("xuanfeng|xiaoji") then
				return source
			end
		end
	end

	if #self.enemies == 1 then
		for _,badpeople in ipairs(self.enemies) do
			if badpeople:isAlive() and not badpeople:hasShownSkill("kongcheng") then
				return source
			end
		end
	end

	for _,badpeople in ipairs(self.enemies) do
		if badpeople:isAlive() and badpeople:getWeapon() then
			for _,player in sgs.qlist(targets) do
				if self:isFriend(player) and not player:getWeapon() then
					if player:hasShownSkill("xuanfeng|xiaoji|zhudao") then
						return player
					end
				end
			end
			for _,player in sgs.qlist(targets) do
				if self:isFriend(player) and not player:getWeapon() then
					return player
				end
			end
		end
	end

	for _,badpeople in ipairs(self.enemies) do
		if badpeople:isAlive() and badpeople:getOffensiveHorse() then
			for _,player in sgs.qlist(targets) do
				if self:isFriend(player) and not player:getOffensiveHorse() then
					if player:hasShownSkill("xuanfeng|xiaoji|zhudao") then
						return player
					end
				end
			end
			for _,player in sgs.qlist(targets) do
				if self:isFriend(player) and not player:getOffensiveHorse() then
					return player
				end
			end
		end
	end
	for _,badpeople in ipairs(self.enemies) do
		if badpeople:isAlive() and badpeople:getArmor() then
			for _,player in sgs.qlist(targets) do
				if self:isFriend(player) and not player:getArmor() then
					if player:hasShownSkill("xuanfeng|xiaoji|zhudao") then
						return player
					end
				end
			end
			for _,player in sgs.qlist(targets) do
				if self:isFriend(player) and not player:getArmor() then
					return player
				end
			end
		end
	end
	for _,badpeople in ipairs(self.enemies) do
		if badpeople:isAlive() and badpeople:getDefensiveHorse() then
			for _,player in sgs.qlist(targets) do
				if self:isFriend(player) and not player:getDefensiveHorse() then
					if player:hasShownSkill("xuanfeng|xiaoji|zhudao") then
						return player
					end
				end
			end
			for _,player in sgs.qlist(targets) do
				if self:isFriend(player) and not player:getDefensiveHorse() then
					return player
				end
			end
		end
	end

	local cardNumMin = 100
	local bestguy = source
	for _,player in ipairs(self.enemies) do
		if player:isAlive() and not player:isKongcheng() then
			for _,goodguy in sgs.qlist(targets) do
				local cardNum = goodguy:getHandcardNum()
				if cardNum < cardNumMin and self:isFriend(goodguy) then
					cardNumMin = cardNum
					bestguy = goodguy
				end
			end
			return bestguy
		end
	end
	return source
end

--思绪，准备复制粘贴就好。。

sgs.ai_skill_cardchosen.sixu = function(self, who, flags)
	local source = self.player

	if self:isFriend(who) and who:getJudgingArea():length() > 0 then
		local cards = who:getJudgingArea()
		return cards[1]
	end

	if self:isEnemy(who) and who:hasShownSkill("liegong|Zhena") and who:isAlive() and who:getWeapon() and not who:hasShownSkill("xuanfeng|xiaoji") then
		for _,player in ipairs(self.friends) do
			if player:isAlive() and not player:getWeapon() then
				local card = who:getWeapon()
				return card
			end
		end
	end

	if not ((not source:getArmor() and who:getArmor()) or (not source:getTreasure() and who:getTreasure()) or (not source:getDefensiveHorse() and who:getDefensiveHorse()) or (not source:getWeapon() and who:getWeapon()) or (not source:getOffensiveHorse() and who:getOffensiveHorse())) then
		if self:isEnemy(who) and who:isAlive() and not who:hasShownSkill("kongcheng") and not who:isKongcheng() and #self.enemies==1 then
			local cards = who:getHandcards()
			return cards[1]
		end
	end



	if self:isEnemy(who) and who:isAlive() and who:getArmor() and not who:hasShownSkill("xuanfeng|xiaoji") then
		for _,player in ipairs(self.friends) do
			if player:isAlive() and not player:getArmor() then
				local card = who:getArmor()
				return card
			end
		end
	end
	if self:isEnemy(who) and who:isAlive() and who:getTreasure() and not who:hasShownSkill("xuanfeng|xiaoji") then
		for _,player in ipairs(self.friends) do
			if player:isAlive() and not player:getTreasure() then
				local card = who:getTreasure()
				return card
			end
		end
	end
	if self:isEnemy(who) and who:isAlive() and who:getDefensiveHorse() and not who:hasShownSkill("xuanfeng|xiaoji") then
		for _,player in ipairs(self.friends) do
			if player:isAlive() and not player:getDefensiveHorse() then
				local card = who:getDefensiveHorse()
				return card
			end
		end
	end
	if self:isEnemy(who) and who:isAlive() and who:getWeapon() and not who:hasShownSkill("xuanfeng|xiaoji") then
		for _,player in ipairs(self.friends) do
			if player:isAlive() and not player:getWeapon() then
				local card = who:getWeapon()
				return card
			end
		end
	end
	if self:isEnemy(who) and who:isAlive() and who:getOffensiveHorse() and not who:hasShownSkill("xuanfeng|xiaoji") then
		for _,player in ipairs(self.friends) do
			if player:isAlive() and not player:getOffensiveHorse() then
				local card = who:getOffensiveHorse()
				return card
			end
		end
	end

	if self:isEnemy(who) and who:isAlive() and not who:isKongcheng() then
		local cards = who:getHandcards()
		return cards[1]
	end
	return nil
end

sgs.ai_skill_invoke.sixu = function(self, data)
	local Can_get_Card_Num = 0
	local tool = 0
	if self.player:getWeapon() then
		tool = tool -1
	end
	if self.player:getArmor() then
		tool = tool -1
	end
	if self.player:getOffensiveHorse() then
		tool = tool -1
	end
	if self.player:getDefensiveHorse() then
		tool = tool -1
	end
	if self.player:getTreasure() then
		tool = tool -1
	end
	local tool_s = tool
	for _,player in ipairs(self.enemies) do
		if player:isAlive() then
				if player:getWeapon() then
					tool = tool +1
				end
				if self.player:getArmor() then
					tool = tool +1
				end
				if self.player:getOffensiveHorse() then
					tool = tool +1
				end
				if self.player:getDefensiveHorse() then
					tool = tool +1
				end
				if self.player:getTreasure() then
		            tool = tool +1
	            end
				Can_get_Card_Num = Can_get_Card_Num + player:getHandcardNum() + tool
				tool = tool_s
		end
	end
	local i = 0
	for _,player in ipairs(self.friends) do
		if player:isAlive() then
			if player:getJudgingArea():length() > 0 then
				Can_get_Card_Num = Can_get_Card_Num + 1
			end
			if player:hasShownSkill("Fangzhu|jujian") then
				i = i+1
			end
		end
	end
	local source = self.player
	if (Can_get_Card_Num >=2 or source:getHp() == 1) and (source:getHandcardNum() > source:getHp() or i>0) then
		return true
	end
	return
end

sgs.Zhudao_keep_value = sgs.xiaoji_keep_value

local wangxiang_skill = {}
wangxiang_skill.name = "wangxiang"
table.insert(sgs.ai_skills, wangxiang_skill)
wangxiang_skill.getTurnUseCard = function(self,room,player,data)
	if self.player:hasUsed("ViewAsSkill_wangxiangCard") or self.player:isKongcheng() or (not self:willShowForAttack() and not self:willShowForDefence()) then return end
	local usevalue = 0
	local keepvalue = 0	
	local id
	local card1
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)
	for _,card in ipairs(cards) do
		if card:isBlack() and card:isKindOf("BasicCard") then
			id = tostring(card:getId())
			card1 = card
			usevalue=self:getUseValue(card)
			keepvalue=self:getKeepValue(card)
			break
		end
	end
	if not id then return end
	local parsed_card = {}
    table.insert(parsed_card, sgs.Card_Parse("drowning:wangxiang[to_be_decided:"..card1:getNumberString().."]=" .. id .."&wangxiang"))				--水淹七军
	table.insert(parsed_card, sgs.Card_Parse("threaten_emperor:wangxiang[to_be_decided:"..card1:getNumberString().."]=" .. id .."&wangxiang"))		--挟天子以令诸侯
	table.insert(parsed_card, sgs.Card_Parse("await_exhausted:wangxiang[to_be_decided:"..card1:getNumberString().."]=" .. id .."&wangxiang"))			--以逸待劳
	table.insert(parsed_card, sgs.Card_Parse("befriend_attacking:wangxiang[to_be_decided:"..card1:getNumberString().."]=" .. id .."&wangxiang"))		--远交近攻
	table.insert(parsed_card, sgs.Card_Parse("duel:wangxiang[to_be_decided:"..card1:getNumberString().."]=" .. id .."&wangxiang"))				--决斗
	table.insert(parsed_card, sgs.Card_Parse("dismantlement:wangxiang[to_be_decided:"..card1:getNumberString().."]=" .. id .."&wangxiang"))		--过河拆桥
	table.insert(parsed_card, sgs.Card_Parse("slash:wangxiang[to_be_decided:"..card1:getNumberString().."]=" .. id .."&wangxiang"))				--顺手牵羊
	table.insert(parsed_card, sgs.Card_Parse("ex_nihilo:wangxiang[to_be_decided:"..card1:getNumberString().."]=" .. id .."&wangxiang"))	--无中生有
	
	local value = 0
	local tcard
	for _, c in ipairs(parsed_card) do
		assert(c)
		if self:getUseValue(c) > value and self:getUseValue(c) > keepvalue and self:getUseValue(c) > usevalue then
			value = self:getUseValue(c)
			tcard = c
		end
	end
	if tcard and id then
		return tcard
	end
end

sgs.ai_skill_invoke.baonu = function(self, data)
	if #self.enemies == 0 or not self:willShowForAttack() or self:willSkipPlayPhase(self.player) then return false end
	if self.player:getHp() == 1 and self:getCardsNum("Peach") == 0 and self:getCardsNum("Analeptic") == 0 then return false end
	for _,p in sgs.list(self.room:getOtherPlayers(self.player)) do
		if self.player:inMyAttackRange(p) and self:isEnemy(p) then return true end
	end
	return false
end

jizhan_skill={}
jizhan_skill.name="jizhan"
table.insert(sgs.ai_skills,jizhan_skill)
jizhan_skill.getTurnUseCard=function(self, inclusive)
	if self.player:hasUsed("JizhanCard") then return end
	if not self.player:hasFlag("baonu_used") then return end
	if not self:willShowForAttack() or self.player:isNude() then return end
	for _,p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if self.player:inMyAttackRange(p) and self:isEnemy(p) and not p:hasShownSkill("huansha") then
		 	return sgs.Card_Parse("@JizhanCard=.&jizhan")
		end
	end
end

sgs.ai_skill_use_func.JizhanCard = function(card,use,self)
	local target
	local card
	if #self.enemies > 0 then
		for _,p in sgs.list(self.room:getOtherPlayers(self.player)) do
			if self.player:inMyAttackRange(p) and self:isEnemy(p) and not p:hasShownSkill("huansha") then
				target = p
			end
		end
	end
	local cards = sgs.QList2Table(self.player:getHandcards())
	local cards2= sgs.QList2Table(self.player:getEquips())
	for _,acard in ipairs(cards) do
		if self:getKeepValue(acard)<5 then
			card = acard
		end
    end
	if not card then
	  for _,acard in ipairs(cards2) do
		  if self:getKeepValue(acard)<5 then
			  card = acard
		  end
      end
	end
	if target and card then
		use.card = sgs.Card_Parse("@JizhanCard="..card:getEffectiveId().."&jizhan")
		if use.to then use.to:append(target) end
		return
	end
end

heiyan_skill={}
heiyan_skill.name="heiyan"
table.insert(sgs.ai_skills,heiyan_skill)
heiyan_skill.getTurnUseCard=function(self,inclusive)
    if not self:willShowForAttack() then return end
	if #self.enemies < 1 then return end
	local source = self.player
	if source:hasUsed("HeiyanCard") then return end
	if self.player:getHp() < 2 then return end
	return sgs.Card_Parse("@HeiyanCard=.&heiyan")
end

sgs.ai_skill_use_func.HeiyanCard = function(card,use,self)
	local target
	local source = self.player
	for _,enemy in ipairs(self.enemies) do
	    if self.player:getHp()>2 and not enemy:hasShownSkill("tianhuo") and not enemy:hasShownSkill("huansha") and not (enemy:getArmor() and ememy:getArmor():objectName()=="PeaceSpell") then
		  target = enemy
		end
	end
	for _,enemy in ipairs(self.enemies) do
		if enemy:getHp() == 2 and self.player:getHp()>2 and not enemy:hasShownSkill("tianhuo") and not enemy:hasShownSkill("huansha") and not (enemy:getArmor() and ememy:getArmor():objectName()=="PeaceSpell") then
			target = enemy
		end
	end
	for _,enemy in ipairs(self.enemies) do
		if enemy:getHp() == 1 and not enemy:hasShownSkill("tianhuo") and not enemy:hasShownSkill("huansha") and not (enemy:getArmor() and ememy:getArmor():objectName()=="PeaceSpell") then
			target = enemy
		end
	end
	if target then
		use.card = sgs.Card_Parse("@HeiyanCard=.&heiyan")
		if use.to then use.to:append(target) end
		return
	end
end

sgs.ai_use_value["HeiyanCard"] = 8
sgs.ai_use_priority["HeiyanCard"]  = 10
sgs.ai_card_intention.HeiyanCard = 90

tiaojiao_skill={}
tiaojiao_skill.name="tiaojiao"
table.insert(sgs.ai_skills,tiaojiao_skill)
tiaojiao_skill.getTurnUseCard=function(self,inclusive)
	if self.player:hasUsed("TiaojiaoCard") then return end
	if not self:willShowForDefence() and not self:willShowForAttack() then return end
	for _,friend in ipairs(self.friends) do
		if friend:getJudgingArea():length() > 0 then
			return sgs.Card_Parse("@TiaojiaoCard=.&tiaojiao")
		end
	end
	if #self.enemies < 1 then return end
	if #self.enemies == 1 and self.enemies[1]:isNude() then return end
	return sgs.Card_Parse("@TiaojiaoCard=.")
end

sgs.ai_skill_use_func.TiaojiaoCard = function(card,use,self)
	local target
	local slashtarget
	local source = self.player
	for _,enemy in ipairs(self.enemies) do
		if not enemy:isNude() and self.player:inMyAttackRange(enemy) then
			target = enemy
		end
	end
	for _,enemy in ipairs(self.enemies) do
		if enemy:getHp() == 2 and not enemy:isNude() and self.player:inMyAttackRange(enemy) then
			target = enemy
		end
	end
	for _,enemy in ipairs(self.enemies) do
		if enemy:getHp() == 1 and not enemy:isNude() and self.player:inMyAttackRange(enemy) then
			target = enemy
		end
	end
	for _,friend in ipairs(self.friends_noself) do
		if friend:getJudgingArea():length() > 0 and not noNeedToRemoveJudgeArea(friend) and self.player:inMyAttackRange(friend) then
			target = friend
		end
	end
	if target then
        for _,p in sgs.qlist(self.room:getOtherPlayers(target)) do
			if target:inMyAttackRange(p) then
				slashtarget = p
			end
		end	
	    for _,enemy in ipairs(self.enemies) do
			if target:inMyAttackRange(enemy) and target:objectName()~=enemy:objectName() then
				slashtarget = enemy
			end
		end
	end
	if target and slashtarget then
		use.card = sgs.Card_Parse("@TiaojiaoCard=.&tiaojiao")
		if use.to then use.to:append(target) end
		if use.to then use.to:append(slashtarget) end
		return
	end
end

sgs.ai_use_value.TiaojiaoCard = 8
sgs.ai_use_priority.TiaojiaoCard  = 10
sgs.ai_card_intention.TiaojiaoCard = 30


--[[sgs.ai_skill_playerchosen.tiaojiao = function(self, targets)
	local target
	if #self.enemies > 0 then
		local cards = 0
		for _,enemy in ipairs(self.enemies) do
			if enemy:isAlive() then
				target = enemy
			end
		end
		for _,enemy in ipairs(self.enemies) do
			if enemy:isAlive() and enemy:getHandcardNum() == 1 then
				target = enemy
			end
		end
		if target then return target end
	    return self.player
	end
end]]

sgs.ai_skill_invoke.gangqu = function(self, data)
    local effect = data:toCardEffect()
	if not effect.from then return self:willShowForDefence() and self.player:getHandcardNum()>1 end
	if effect.from then return (self:isEnemy(effect.from) and not effect.card:isKindOf("ExNihilo") and not effect.card:isKindOf("AmazingGrace") and not effect.card:isKindOf("GodSalvation")) or effect.card:isKindOf("SavageAssault") or effect.card:isKindOf("ArcheryAttack") end
end

sgs.ai_skill_invoke.sandun = function(self, data)
   local can
   for _,id in sgs.qlist(self.room:getDrawPile()) do
     local c = sgs.Sanguosha:getCard(id)
	 if c:isKindOf("Armor") then can = true end
   end
   return self:willShowForDefence() and can
end

sgs.ai_skill_invoke.zhonger = function(self, data)
  return self:willShowForDefence() or self:willShowForAttack()
end

sgs.ai_skill_invoke.xieyan = function(self, data)
  return self:willShowForAttack()
end

sgs.ai_skill_discard["sandun"] = function(self, discard_num, min_num, optional, include_equip)
  return self:askForDiscard("discard", discard_num, min_num, false, include_equip)
end

sgs.ai_skill_invoke.toushe = function(self, data)
	return (self:getCardsNum("Slash") > 0 or not self.player:getPile("sword"):isEmpty() or (self.player:hasSkill("shengjian") and sgs.ai_skill_invoke.shengjian== true)) and self:willShowForAttack()
end

sgs.ai_skill_invoke.jianjie = function(self, data)
	return self:willShowForDefence() or self:willShowForAttack()
end

sgs.ai_skill_discard["toushe"] = function(self, discard_num, min_num, optional, include_equip)
  if #self.enemies <= 0 then return {} end
  if self.player:getHandcardNum()<self.player:getHp() then return {} end
  if self:getCardsNum("Slash")==0 and self.player:getPile("sword"):isEmpty() then return {} end  
  return self:askForDiscard("discard", discard_num, min_num, false, include_equip)
end

local jianjie_skill = {}
jianjie_skill.name = "jianjie"
table.insert(sgs.ai_skills, jianjie_skill)
jianjie_skill.getTurnUseCard = function(self)
	if self.player:getPile("sword"):isEmpty() then
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
	for i = 0, self.player:getPile("sword"):length() - 1, 1 do
		local slash = sgs.Sanguosha:getCard(self.player:getPile("sword"):at(i))
		local slash_str = ("slash:jianjie[%s:%s]=%d&jianjie"):format(slash:getSuitString(), slash:getNumberString(), self.player:getPile("sword"):at(i))
		local jianjieslash = sgs.Card_Parse(slash_str)
		assert(jianjieslash)
        if self:slashIsAvailable(self.player, jianjieslash) then
			table.insert(cards, jianjieslash)
		end
	end
	if #cards == 0 then return end
	return cards[1]
end

sgs.ai_view_as.jianjie = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local ask = sgs.Sanguosha:getCurrentCardUsePattern()
	if card_place == sgs.Player_PlaceSpecial and player:getPileName(card_id) == "sword" and ask == "slash" then
		return ("slash:jianjie[%s:%s]=%d%s"):format(suit, number, card_id, "&jianjie")
	end
	if card_place == sgs.Player_PlaceSpecial and player:getPileName(card_id) == "sword" and ask == "jink" then
		return ("jink:jianjie[%s:%s]=%d%s"):format(suit, number, card_id, "&jianjie")
	end
end

sgs.ai_skill_choice["toushe"] = function(self, choices, data)
	local source = self.player
	local n = self.player:getPile("sword"):length()
	if self:getCardsNum("Slash")+n >3 then return "Crossbow" end
    local m = math.random(1,10)	
	if m <= 7 then return "DragonPhoenix" end
end

sgs.ai_skill_invoke.lixiangjianqiao = function(self, data)
	return data:toDamage().to:isWounded()
end

sgs.ai_skill_invoke.zishang = function(self, data)
	return self:isFriend(data:toDamage().to) and self.player:getHp()>=data:toDamage().to:getHp()
end

sgs.ai_skill_playerchosen.zishang = function(self, targets, max_num, min_num)
    local n = 998
	local yyui
	for _, target in sgs.qlist(targets) do
		if self:isFriend(target) and target:getHandcardNum()<n then n = target:getHandcardNum() end
		if self:isFriend(target) and target:hasSkill("wenchang") then yyui = target end
	end
	if yyui and yyui:getHandcardNum()<3 then return yyui end
	for _, target in sgs.qlist(targets) do	     
		if self:isFriend(target) and target == yyui and target:getHandcardNum()==n then return target end
	end
	for _, target in sgs.qlist(targets) do	     
		if self:isFriend(target) and target:getHandcardNum()==n then return target end
	end
	return nil
end

sgs.ai_skill_discard["wenchang"] = function(self, discard_num, min_num, optional, include_equip)
  local data=self.player:getTag("wenchang_data")
  if data:toDamage() and self:isFriend(data:toDamage().to) then
     if data:toDamage().to:isWounded() or data:toDamage().to:getHandcardNum()<self.player:getHandcardNum() then
	    return self:askForDiscard("discard", discard_num, min_num, false, include_equip)
	 end
  end
  return {}
end

sgs.ai_skill_invoke.yuanxin = function(self, data)
	return self:isFriend(data:toDamage().to)
end

local duanzui_skill={}
duanzui_skill.name="duanzui"
table.insert(sgs.ai_skills,duanzui_skill)
duanzui_skill.getTurnUseCard=function(self,inclusive)
    if self.player:hasUsed("DuanzuiCard") then return false end
	if not self:willShowForAttack() and not self:willShowForDefence() then return false end
	local cards = sgs.QList2Table(self.player:getHandcards())
	local equips=sgs.QList2Table(self.player:getEquips())
	self:sortByUseValue(cards,true)
	self:sortByUseValue(equips,true)
	for _,acard in ipairs(cards) do
		if self:getKeepValue(acard)<5 and acard:isRed() then
			return sgs.Card_Parse("@DuanzuiCard=.&duanzui")
		end
	end
	for _,acard in ipairs(equips) do
		if self:getKeepValue(acard)<5 and acard:isRed() then
			return sgs.Card_Parse("@DuanzuiCard=.&duanzui")
		end
	end
end

sgs.ai_skill_use_func.DuanzuiCard = function(card,use,self)
	local target
	local n=998
	for _,enemy in ipairs(self.enemies) do
		if enemy:getHp()<n and self:slashIsEffective(sgs.Sanguosha:cloneCard("fire_slash", sgs.Card_NoSuit, 0), enemy) and (not enemy:hasShownSkill("kongni") or enemy:getEquips():length()==0) and (not enemy:getArmor() or enemy:getArmor():objectName()~="IronArmor" or enemy:getArmor():objectName()~="PeaceSpell" or (self.player:getWeapon() and self.player:getWeapon():objectName()=="QinggangSword")) then
			n = enemy:getHp()
		end
	end
	for _,enemy in sgs.list(self.enemies) do
		if enemy:getHp()==n and not enemy:isRemoved() then
			target = enemy
		end
	end
	local needed
	local cards = sgs.QList2Table(self.player:getHandcards())
	local equips=sgs.QList2Table(self.player:getEquips())
	for _,acard in ipairs(equips) do
		if target and (self:getKeepValue(acard)<5 or self:isWeak(target)) and acard:isRed() then
			needed = acard
		end
	end
	for _,acard in ipairs(cards) do
		if self:getKeepValue(acard)<5 and acard:isRed() then
			needed = acard
		end
	end
	if target and needed then
		use.card = sgs.Card_Parse("@DuanzuiCard="..needed:getEffectiveId().."&duanzui")
		if use.to then use.to:append(target) end
		return
	end
end

sgs.ai_skill_invoke.buwu = function(self, data)
	local damage = data:toDamage()
	local dest = damage.to
	if self:isEnemy(dest) and self:willShowForAttack() then
		return true
	end
	return false
end

sgs.ai_skill_invoke.chigui = function(self, data)
	for _,enemy in ipairs(self.enemies) do
	  if enemy:getWeapon() then return self:willShowForAttack() and self.player:getMark("@tianmo")>0 and self.player:hasSkill("tianmo") end
	end
end

sgs.ai_skill_invoke.tianmo = function(self, data)
	return self:willShowForAttack() or self:willShowForDefence()
end

sgs.ai_skill_playerchosen.chigui = function(self, targets, max_num, min_num)
	for _, target in sgs.qlist(targets) do
		if self:isEnemy(target) and target:getWeapon() then return target end
	end
	return nil
end

--尤斯蒂亚
sgs.ai_skill_invoke["jinghua"] = function(self, data)
  if #self.friends<1 or (not self:willShowForDefence() and not self:willShowForAttack()) then return false end
  return true
end

sgs.ai_skill_playerchosen["jinghua"] = function(self, targets)
	local friends = {}
	for _,player in ipairs(self.friends) do
		if player:isAlive() and self.room:getCurrent():objectName() ~= player:objectName() then
			table.insert(friends, player)
		end
	end
	self:sort(friends)

	local source = self.player
	local max_x = 5 - source:getHandcardNum()
	local target = source
	local judge = target:getJudgingArea()
	if judge:length() > 0 and not noNeedToRemoveJudgeArea(target) then
		return target
	end
	for _, friend in ipairs(friends) do
		if friend:getHp() < friend:getMaxHp() and self:hasSkills(sgs.masochism_skill, friend) then
			return friend
		end
	end

	for _, friend in ipairs(friends) do
		if friend:getHp() < friend:getMaxHp() and friend:getHp() == 1 then
			return friend
		end
	end

	for _, friend in ipairs(friends) do
		local x = 5 - friend:getHandcardNum()
		if friend:hasSkill("manjuan") then x = x + 1 end

		local judge = friend:getJudgingArea()
		if judge:length() > 0 and not noNeedToRemoveJudgeArea(friend) then
			return friend
		end

		if x > max_x and friend:isAlive() then
			max_x = x
			target = friend
		end
	end

	return target
end

--sgs.ai_skillInvoke_intention.jinghua = -60

sgs.ai_skill_choice["jinghua"] = function(self, choices, data)
	local source = self.player
	local judge = source:getJudgingArea()
	if judge:length() > 0 then
		return "jinghua_getcard"
	end
	if source:getHp() < source:getMaxHp() then return "jinghua_recover" end
	return "jinghua_drawcard"
end

sgs.ai_skill_invoke["jiushu"] = function(self, data)
	local dying_data = data:toDying()
	local source = dying_data.who
	for _,player in ipairs(self.friends) do
		if player:isAlive() and source:objectName() == player:objectName() then
			return true
		end
	end
	if source:objectName() == self.player:objectName() then
		return true
	end
	return false
end

--经济学魔王
boxue_skill={}
boxue_skill.name="boxue"
table.insert(sgs.ai_skills,boxue_skill)
boxue_skill.getTurnUseCard=function(self,inclusive)
	if self.player:hasUsed("BoxueCard") then return end
	if #self.friends<1 then return end
	if not self:willShowForAttack() and not self:willShowForDefence() then return end
	return sgs.Card_Parse("@BoxueCard=.&boxue")
end

sgs.ai_skill_use_func.BoxueCard = function(card,use,self)
	local targets = sgs.SPlayerList()
	for _,friend in ipairs(self.friends) do
		targets:append(friend)
	end
	if targets then
		use.card = sgs.Card_Parse("@BoxueCard=.&boxue")
		if use.to then use.to = targets end
		return
	end
end

sgs.ai_use_value["BoxueCard"] = 8
sgs.ai_use_priority["BoxueCard"]  = 10
sgs.ai_card_intention.BoxueCard = -60

sgs.ai_skill_choice.BoxueCard = function(self, choices, data)
	return "gx"
end

sgs.ai_view_as.cangshan = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if (card_place ~= sgs.Player_PlaceSpecial or player:getHandPile():contains(card_id)) and card:isKindOf("EquipCard") then
		return ("slash:cangshan[%s:%s]=%d&cangshan"):format(suit, number, card_id)
	end
end

local cangshan_skill = {}
cangshan_skill.name = "cangshan"
table.insert(sgs.ai_skills, cangshan_skill)
cangshan_skill.getTurnUseCard = function(self, inclusive)

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

	local hecards = self.player:getCards("he")
	for _, id in sgs.qlist(self.player:getHandPile()) do
		hecards:prepend(sgs.Sanguosha:getCard(id))
	end
	local cards = {}
	for _, card in sgs.qlist(hecards) do
		if card:isKindOf("EquipCard")
			and (not isCard("Crossbow", card, self.player) or disCrossbow ) then
			local suit = card:getSuitString()
			local number = card:getNumberString()
			local card_id = card:getEffectiveId()
			local card_str = ("slash:cangshan[%s:%s]=%d&cangshan"):format(suit, number, card_id)
			local slash = sgs.Card_Parse(card_str)
			assert(slash)
			if self:slashIsAvailable(self.player, slash) then
				table.insert(cards, slash)
			end
		end
	end

	if #cards == 0 then return end

	self:sortByUsePriority(cards)
	return cards[1]
end

function sgs.ai_cardneed.cangshan(to, card)
	return to:getHandcardNum() < 3 and card:isKindOf("EquipCard")
end

local yuehuang_skill={}
yuehuang_skill.name="yuehuang"
table.insert(sgs.ai_skills,yuehuang_skill)
yuehuang_skill.getTurnUseCard=function(self,inclusive)
	if #self.friends < 2 then return end
	if self.player:hasUsed("YuehuangCard") then return end
	return sgs.Card_Parse("@YuehuangCard=.&yuehuang")
end

sgs.ai_skill_use_func.YuehuangCard = function(card,use,self)
	use.card = sgs.Card_Parse("@YuehuangCard=.&yuehuang")
	return
end

sgs.ai_use_value.YuehuangCard = 7
sgs.ai_use_priority.YuehuangCard = 9

sgs.ai_skill_cardask["@YuehuangGive"] = function(self, data)
  for _, c in sgs.qlist(self.player:getCards("he")) do
	 if c:isKindOf("EquipCard") then
		return "$" .. c:getEffectiveId()
	 end
  end
end

sgs.ai_skill_cardask["@saiqian"] = function(self, data)
  local dest = data:toPlayer()
  if not self:isFriend(dest) or not dest:isWounded() then return "." end
  for _, c in sgs.qlist(self.player:getCards("he")) do
	 if c:isRed() then
		return "$" .. c:getEffectiveId()
	 end
  end
end

sgs.ai_skill_invoke.jianshi = function(self, data)
  local move=data:toMoveOneTime()
  if move.to then
    return self:isEnemy(move.to) or (self:willShowForDefence() and not self.player:hasShownSkill("jianshi"))
  end
end

sgs.ai_skill_invoke.qiyue = function(self, data)
  return self:isFriend(self.room:getCurrent())
end

--绫波丽
sgs.ai_skill_invoke.chidun = function(self, data)
	return self:isFriend(data:toDamage().to) and self.player:getHp()>=data:toDamage().to:getHp() and (self.player:getHp()>1 or self.player:getHandcardNum()<=data:toDamage().to:getHandcardNum())
end

sgs.ai_skill_invoke.weixiao = function(self, data)
	local f
	local e
	for _,p in sgs.qlist(self.room:getAlivePlayers()) do
	  if self:isEnemy(p) and p:getHandcardNum()>p:getHp() then
	    e=p
	  end
	  if self:isFriend(p) and p:getHandcardNum()<=p:getHp() then
	    f=p
	  end
	end
	return (e and self:willShowForAttack()) or (f and self:willShowForDefence())
end

sgs.ai_skill_playerchosen.weixiao = function(self, targets, max_num, min_num)
    local list = {}	
	for _,e in ipairs(self.enemies) do
		if e:getHandcardNum()>e:getHp() then table.insert(list,e) end
	end
	for _,f in ipairs(self.friends) do
		if f:getHandcardNum()<=f:getHp() then table.insert(list,f) end
	end
	return list[math.random(1,#list)]
end

sgs.ai_skill_discard["weixiao"] = function(self, discard_num, min_num, optional, include_equip)
   return self:askForDiscard("discard", discard_num, min_num, false, include_equip)
end


sgs.ai_skill_invoke.huansha = function(self, data)
    if data:toDamage().to then
	   return data:toDamage().damage>0 or self:willShowForDefence()
	else
       return self:willShowForAttack()	
	end
end

sgs.ai_skill_invoke.dapo = function(self, data)
    if data:toDamage().to then
	  return self:isFriend(data:toDamage().to) and (self:willShowForAttack() or self:isWeak(data:toDamage().to))
	end
end

sgs.ai_skill_invoke.wujie = function(self, data)
	return true
end

sgs.ai_skill_invoke.shuangqiang = function(self, data)
	local use=data:toCardUse()
	for _,p in sgs.qlist(use.to) do
	  if (self:isFriend(p) and p:getJudgingArea():length()>0 and not noNeedToRemoveJudgeArea(p)) or (self:isEnemy(p) and not p:isNude()) then
	    return true
	  end
	end
end

sgs.ai_skill_choice.wujie = function(self, choices, data)
	local damage=data:toDamage()
	for _,p in sgs.qlist(self.room:getAlivePlayers()) do
	  if p:willBeFriendWith(self.player) and p:inMyAttackRange(damage.to) then
	     return "wujie_drawcard"
	  end
	end
	return "wujie_givecard"
end

--友利奈绪
sgs.ai_skill_invoke.huanxing = function(self, data)
  local use = data:toCardUse()
  return self:isEnemy(use.from) and self:willShowForDefence() and self:getCardsNum("Slash")>0
end

sgs.ai_skill_invoke.fushang = function(self, data)
  for _,p in ipairs(self.friends) do
    if p:isWounded() then return true end
  end
end

sgs.ai_skill_playerchosen.fushang = function(self, targets)
	for _,p in ipairs(self.friends) do
      if p:isWounded() and p:getJudgingArea():length()>0 then return p end
    end
	for _,p in ipairs(self.friends) do
      if p:isWounded() then return p end
    end
end

--雷德
sgs.ai_skill_invoke.gaoxiao = function(self, data)
  return self:willShowForAttack() or self:willShowForDefence()
end

sgs.ai_skill_invoke.gaokang = function(self, data)
  local damage = data:toDamage()
  local can
  for _,c in sgs.qlist(self.player:getHandcards()) do
    if c:isBlack() then can = true end
  end
  return damage.damage > 0 and self:isFriend(damage.to) and can
end

--玛茵
sgs.ai_skill_invoke.jixian = function(self, data)
    local list = sgs.SPlayerList()
    for _,p in ipairs(self.enemies) do
		if self.player:inMyAttackRange(p) then list:append(p) end
	end
	if #self.enemies == 0 or not self:willShowForAttack() then return false end
	if self.player:getLostHp() <= 1 then return true end
	if self.player:getLostHp() > 1 then
		if self.room:getAlivePlayers():length() == 2 then return true end
		if self.player:getHp() <= 1 and self:getCardsNum("Peach") == 0 and self:getCardsNum("Analeptic") == 0 then return true end
	end
	for _,p in ipairs(self.enemies) do
		if p:getHp() == 1 and self:isWeak(p) then return true end
	end
	return false
end

sgs.ai_skill_playerchosen.jixian = function(self, targets)
	return self:getPriorTarget()
end

sgs.ai_skill_choice.nangua = function(self, choices, data)
	if self.player:faceUp() then return "nangua_recover" end
	if self.player:getHp() < 1 then return "nangua_recover" end
	return "nangua_turnover"
end

--大傻
nuequ_skill={}
nuequ_skill.name="nuequ"
table.insert(sgs.ai_skills,nuequ_skill)
nuequ_skill.getTurnUseCard=function(self,inclusive)
	if self.player:hasUsed("NuequCard") or (not self:willShowForAttack() and not self:willShowForDefence()) then return end
	if self.player:isNude() then return end
	local targets = {}
	--[[local min = 100
	for _,p in sgs.list(self.room:getAlivePlayers()) do
		if p:getHp() < min then
			targets = {}
			min = p:getHp()
		end
		if p:getHp() <= min then table.insert(targets, p) end
	end
	if min == 0 then
		if #targets == 1 and targets[1]:hasShownSkill("lingti") then return end
	end]]
	return sgs.Card_Parse("@NuequCard=.&nuequ")
end

sgs.ai_skill_use_func.NuequCard = function(card,use,self)
	local target

	local cards = sgs.QList2Table(self.player:getCards("h"))
	self:sortByKeepValue(cards)
	local card
	for _,c in ipairs(cards) do
	  if c:isKindOf("BasicCard") then
	     card = c
		 break
	  end
	end
    
	if not card then return end
	local dummyslash = sgs.Sanguosha:cloneCard("fire_slash", card:getSuit(), card:getNumber())
	local targets = {}
	local min = 100
	for _,p in sgs.list(self.room:getAlivePlayers()) do
		--if p:getHp() <= min then table.insert(targets, p) end
		table.insert(targets, p)
	end

	for _, t in ipairs(targets) do
		if self:isEnemy(t) and t:getHandcardNum() == 0 and self:slashIsEffective(dummyslash, t) then
			target = t
		end
	end

	--[[if not target then
		for _, t in ipairs(targets) do
			if self:isFriend(t) and t:getHp() <= 1 and self:slashIsEffective(dummyslash, t) then
				target = t
			end
		end
	end]]

	if not target then
		for _, t in ipairs(targets) do
			if self:isEnemy(t) and t:getHandcardNum() <= 1 and self:slashIsEffective(dummyslash, t) then
				target = t
			end
		end
	end

	if not target then
		for _, t in ipairs(targets) do
			if self:isEnemy(t) and self:slashIsEffective(dummyslash, t) then
				target = t
			end
		end
	end

	--[[if not target then
		for _, t in ipairs(targets) do
			if self:isFriend(t) and t:isWounded() and self:slashIsEffective(dummyslash, t) then
				target = t
			end
		end
	end]]

	--if not target then target = targets[1] end



	if target and card then
		use.card = sgs.Card_Parse("@NuequCard="..card:getEffectiveId().."&nuequ")
		 if use.to then use.to:append(target) end
		return
	end
end

sgs.ai_use_value["NuequCard"] = 5
sgs.ai_use_priority["NuequCard"]  = 0
sgs.ai_card_intention.NuequCard = function(self, card, from, tos)
	--[[for _, to in ipairs(tos) do
		if self:isFriend(to) then return -80 end
	end]]
	return 100
end

sgs.ai_skill_invoke["BurningLove"] = function(self, data)
	local damage = data:toDamage()
	return self:isEnemy(damage.to)
end

sgs.ai_skill_choice.BurningLove = function(self, choices, data)
   local damage=data:toDamage()
   if (damage.to:hasShownSkill("zhudao")) then return "nuequ_chain" end
   if (damage.to:hasShownSkill("zhengchang")) then return "nuequ_discard" end
   if (damage.to:hasShownSkill("tianhuo")) then return "nuequ_discard" end
   if (damage.to:getEquips():length()>2 and damage.to:getHp()>2) then
      return "nuequ_discard"
   end
   for _,p in sgs.qlist(self.room:getAlivePlayers()) do
     if p:objectName()~= damage.to:objectName() and p:isFriendWith(damage.to) then
	    return "nuequ_chain"
	 end
   end
end

--加贺
sgs.ai_skill_invoke.weishi = function(self, data)
    if (not self:willShowForAttack() and not self:willShowForDefence()) then return end
	local targets = {}
	if self:isWeak() then return true end
	for _,p in sgs.list(self.room:getOtherPlayers(self.player)) do
		if (#p:getPileNames() > 0 or self.player:isFriendWith(p)) and self:isFriend(p) then
			table.insert(targets, p)
		end
	end
	if #targets > 0 then return true end
	if self.player:getHandcardNum() > self.player:getHp() - 1 then return true end
	if #self.enemies > self.player:getPile("Kansaiki"):length() then return true end
	return false
end

sgs.ai_skill_playerchosen.weishi = function(self, targets)
    for _, target in sgs.qlist(targets) do
	  if self:isFriend(target) and target:isWounded() then
	    return target
	  end
	end
	if #self.enemies > self.player:getPile("Kansaiki"):length() then return self.player end
	-- todo, now return random fuck
end

hongzha_skill={}
hongzha_skill.name="hongzha"
table.insert(sgs.ai_skills,hongzha_skill)
hongzha_skill.getTurnUseCard=function(self,inclusive)
	if self.player:hasUsed("HongzhaCard") then return end
	if #self.enemies == 0 then return end
	if self.player:getPile("Kansaiki"):length() == 0 then return end

	--是否发动？
	if self.player:isKongcheng() then return end
	if self.player:getHandcardNum() == 1 then
		if self:getCardsNum("Jink") == 1 or self:getCardsNum("Peach") == 1 then return end
	end

	for _,target in ipairs(self.enemies) do
		if self:slashIsEffective(sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0), target)  then
			return sgs.Card_Parse("@HongzhaCard=.&hongzha")
		end
	end
end

sgs.ai_skill_use_func.HongzhaCard = function(card,use,self)
	local targets = sgs.SPlayerList()
	local n = 0
	for _,p in sgs.qlist(self.room:getAlivePlayers()) do
	   n = n+ p:getPile("Kansaiki"):length()
	end
	for _,enemy in ipairs(self.enemies) do
		if targets:length() < n and not enemy:hasArmorEffect("vine") and self:slashIsEffective(sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0), enemy) then
			targets:append(enemy)
		end
	end
	local cards = sgs.QList2Table(self.player:getCards("he"))
	self:sortByKeepValue(cards)
    local card
	for _,c in ipairs(cards) do
	  if c:isKindOf("BasicCard") then
	     card = c
		 break
	  end
	end
    
	if not card then return end
	if targets:length() > 0 and card then
		use.card = sgs.Card_Parse("@HongzhaCard="..card:getEffectiveId().."&hongzha")
		 use.to = targets
		return
	end
end

sgs.ai_use_value["HongzhaCard"] = 4
sgs.ai_use_priority["HongzhaCard"]  = 0
sgs.ai_card_intention["HongzhaCard"] = 100

--狂三
local function card_for_kekedi(self, who, return_prompt)
	local card, target
	if self:isFriend(who) then
		local judges = who:getJudgingArea()
		if not judges:isEmpty() and not noNeedToRemoveJudgeArea(who) then
			for _, judge in sgs.qlist(judges) do
				card = sgs.Sanguosha:getCard(judge:getEffectiveId())
				for _, enemy in ipairs(self.enemies) do
					if not enemy:containsTrick(judge:objectName()) and self:hasTrickEffective(judge, enemy, self.player) then
						target = enemy
						break
					end
					if target then break end
				end
			end
		end

		local equips = who:getCards("e")
		local weak = false
		if not target and not equips:isEmpty() and who:hasShownSkills(sgs.lose_equip_skill) then
			for _, equip in sgs.qlist(equips) do
				if equip:isKindOf("OffensiveHorse") then card = equip break
				elseif equip:isKindOf("Weapon") then card = equip break
				elseif equip:isKindOf("DefensiveHorse") and not self:isWeak(who) then
					card = equip
					break
				elseif equip:isKindOf("Armor") and (not self:isWeak(who) or self:needToThrowArmor(who)) then
					card = equip
					break
				end
			end

			if card then
				if card:isKindOf("Armor") or card:isKindOf("DefensiveHorse") then
					self:sort(self.friends, "defense")
				else
					self:sort(self.friends, "handcard")
					self.friends = sgs.reverse(self.friends)
				end
				for _, friend in ipairs(self.friends) do
					if not self:getSameEquip(card, friend) and friend:objectName() ~= who:objectName()
						and friend:hasShownSkills(sgs.need_equip_skill .. "|" .. sgs.lose_equip_skill) then
							target = friend
							break
					end
				end
				for _, friend in ipairs(self.friends) do
					if not self:getSameEquip(card, friend) and friend:objectName() ~= who:objectName() then
						target = friend
						break
					end
				end
			end
		end
	else
		local judges = who:getJudgingArea()

		if card == nil or target == nil then
			if not who:hasEquip() or who:hasShownSkills(sgs.lose_equip_skill) then return nil end
			local card_id = self:askForCardChosen(who, "e", "snatch")
			if card_id >= 0 and who:hasEquip(sgs.Sanguosha:getCard(card_id)) then card = sgs.Sanguosha:getCard(card_id) end
			if card then
				if card:isKindOf("Armor") or card:isKindOf("DefensiveHorse") then
					self:sort(self.friends, "defense")
				else
					self:sort(self.friends, "handcard")
					self.friends = sgs.reverse(self.friends)
				end
				for _, friend in ipairs(self.friends) do
					if not self:getSameEquip(card, friend) and friend:objectName() ~= who:objectName() and friend:hasShownSkills(sgs.lose_equip_skill .. "|shensu") then
						target = friend
						break
					end
				end
				for _, friend in ipairs(self.friends) do
					if not self:getSameEquip(card, friend) and friend:objectName() ~= who:objectName() then
						target = friend
						break
					end
				end
			end
		end
	end

	if return_prompt == "card" then return card
	elseif return_prompt == "target" then return target
	else
		return (card and target)
	end
end

sgs.ai_skill_cardchosen.kekedi = function(self, who, flags)
	if flags == "ej" then
		return card_for_kekedi(self, who, "card")
	end
end

sgs.ai_skill_playerchosen.kekedi = function(self, targets)
    if (math.ceil(self.player:getHandcardNum()/2) * 2 ~= self.player:getHandcardNum()) then
	local who = self.room:getTag("KekediTarget"):toPlayer()
	if who then
		if not card_for_kekedi(self, who, "target") then self.room:writeToConsole("NULL") end
		return card_for_kekedi(self, who, "target")
	end
	else
	   return self:getPriorTarget()
	end
end

sgs.ai_skill_use["@@kekedi"] = function(self, prompt)
    self:updatePlayers()
	local QBCard = "@KekediCard=.&kekedi->"

        self:sort(self.enemies, "defense")
		for _, friend in ipairs(self.friends) do
			if not friend:getCards("j"):isEmpty() and card_for_kekedi(self, friend, ".") then
				return QBCard .. friend:objectName()
			end
		end

		for _, friend in ipairs(self.friends_noself) do
			if not friend:getCards("e"):isEmpty() and friend:hasShownSkills(sgs.lose_equip_skill) and card_for_kekedi(self, friend, ".") then
				return QBCard .. friend:objectName()
			end
			if not friend:getArmor() then has_armor = false end
		end


		local targets = {}
		for _, enemy in ipairs(self.enemies) do
			if card_for_kekedi(self, enemy, ".") then
				table.insert(targets, enemy)
			end
		end

		if #targets > 0 then
			self:sort(targets, "defense")
			return QBCard .. targets[#targets]:objectName()
		end
end

sgs.ai_skill_invoke.kekedi = function(self, data)
  return self:willShowForAttack() or self:willShowForDefence()
end

badan_skill={}
badan_skill.name="badan"
table.insert(sgs.ai_skills,badan_skill)
badan_skill.getTurnUseCard=function(self,inclusive)
    if not self:willShowForAttack() then return end
	if #self.enemies < 1 then return end
	local source = self.player
	if source:getMark("@Eight")==0 then return end
	local n=0
   local m=0
   for _,p in sgs.qlist(self.room:getAlivePlayers()) do
     if (p:isFriendWith(self.player) and not p:isNude()) then n=n+1 end
     if (not p:hasShownOneGeneral()) then m=m+1 end
   end
   if n<3 and m>=2 then
     return
   end
   if self.player:getLostHp()<=0 and n>1 then return end
	return sgs.Card_Parse("@BadanCard=.&badan")
end

sgs.ai_skill_use_func.BadanCard = function(card,use,self)
	use.card = sgs.Card_Parse("@BadanCard=.&badan")
	return
end

--御坂美琴
sgs.ai_skill_invoke.dianci = function(self, data)
  return self:willShowForAttack() or self:willShowForDefence()
end

sgs.ai_skill_choice.dianci = function(self, choices, data)
    local can
    for _, enemy in ipairs(self.enemies) do
		if not enemy:isChained() then
			can=true
		end
	 end
	 for _, friend in ipairs(self.friends) do
		if friend:isChained() then
			can=true
		end
	 end
	if can==true then
	  return "chain_target"
	else
	  return "know_target"
	end
end

sgs.ai_skill_playerchosen.dianci = function(self, targets)
     local target
     for _, enemy in ipairs(self.enemies) do
		if not enemy:isChained() then
			target = enemy
		end
	 end
	 for _, friend in ipairs(self.friends) do
		if friend:isChained() then
			target = friend
		end
	 end
	 if target then return target end
end

sgs.ai_view_as.paoji = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if (card_place ~= sgs.Player_PlaceSpecial or player:getHandPile():contains(card_id)) and card:isBlack() then
		return ("thunder_slash:paoji[%s:%s]=%d&paoji"):format(suit, number, card_id)
	end
end

local paoji_skill = {}
paoji_skill.name = "paoji"
table.insert(sgs.ai_skills, paoji_skill)
paoji_skill.getTurnUseCard = function(self, inclusive)
    if not self:willShowForAttack() then return end
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

	local hecards = self.player:getCards("he")
	for _, id in sgs.qlist(self.player:getHandPile()) do
		hecards:prepend(sgs.Sanguosha:getCard(id))
	end
	local cards = {}
	for _, card in sgs.qlist(hecards) do
		if card:isBlack() then
			local suit = card:getSuitString()
			local number = card:getNumberString()
			local card_id = card:getEffectiveId()
			local card_str = ("thunder_slash:paoji[%s:%s]=%d&paoji"):format(suit, number, card_id)
			local slash = sgs.Card_Parse(card_str)
			assert(slash)
			if self:slashIsAvailable(self.player, slash) then
				table.insert(cards, slash)
			end
		end
	end

	if #cards == 0 then return end

	self:sortByUsePriority(cards)
	return cards[1]
end

function sgs.ai_cardneed.paoji(to, card)
	return to:getHandcardNum() < 3 and card:isBlack()
end

sgs.ai_suit_priority.paoji= "club|spade|diamond|heart"

sgs.ai_skill_choice.paoji = function(self, choices, data)
    local effect = data:toSlashEffect()
	local can = false
	for _,c in sgs.qlist(self.player:getHandcards()) do
	  if c:isKindOf("Slash") or c:isBlack() then can = true end
	end
	if (self:isEnemy(effect.to) and effect.to:getHp()==1 and can == true) then
       return "slash_to_target"
	end
	return "draw_one_card"
end

--岛风
sgs.ai_skill_invoke.jifeng = function(self, data)
  local current = self.room:getCurrent()
  if current and current:getNextAlive():objectName()==self.player:objectName() then return false end
  return self:willShowForAttack() or self:willShowForDefence()
end

yulei_skill={}
yulei_skill.name="yulei"
table.insert(sgs.ai_skills,yulei_skill)
yulei_skill.getTurnUseCard=function(self,inclusive)
	local source = self.player
	if source:isNude() then return end
	if source:hasUsed("YuleiCard") then return end
	for _,friend in ipairs(self.friends) do
	  if friend:objectName()==source:getNextAlive():objectName() then return end
	end
	if not self:willShowForAttack() then return end
	return sgs.Card_Parse("@YuleiCard=.&yulei")
end

sgs.ai_skill_use_func.YuleiCard = function(card,use,self)
	local source = self.player
	local max_num = 0
	local nex
	local new = source
	
	for _,q in sgs.qlist(self.room:getPlayers()) do
	    if q:objectName()==new:getNext():objectName() then 
		nex = q
		new = nex
		break
		end
	end
	
	for _,p in sgs.qlist(self.room:getPlayers()) do
	  if (self:isEnemy(nex) or nex:isDead()) then
	  for _,q in sgs.qlist(self.room:getPlayers()) do
	    if q:objectName()==new:getNext():objectName() then
		nex = q
		new = nex
		break
		end
	  end
	  max_num = max_num+1
	  else
	    break
	  end
	end
    
	if max_num>=self.room:getAlivePlayers():length() then
	   max_num = self.room:getAlivePlayers():length()-1
	end
	
	local cards=sgs.QList2Table(self.player:getHandcards())
	local equips=sgs.QList2Table(self.player:getEquips())
	local needed = {}
	for _,acard in ipairs(cards) do
		if #needed < max_num and acard:isKindOf("BasicCard") then
			table.insert(needed, acard:getEffectiveId())
		end
	end
	for _,acard in ipairs(equips) do
		if #needed < max_num and acard:isKindOf("BasicCard") then
			table.insert(needed, acard:getEffectiveId())
		end
	end
	if  #needed>0 then
		use.card = sgs.Card_Parse("@YuleiCard="..table.concat(needed,"+").."&yulei")
		return
	end
end

sgs.ai_use_priority.YuleiCard  = 6

sgs.ai_skill_invoke.huibi = function(self, data)
  local use = data:toCardUse()
  local can
  for _,c in sgs.qlist(self.player:getHandcards()) do
    if c:isKindOf("BasicCard") then can = true end
  end
  if not can then return false end
  if self:isEnemy(use.from) then 
     return not use.card:isKindOf("ExNihilo") and not use.card:isKindOf("AmazingGrace") and not use.card:isKindOf("GodSalvation")
  end
  if self:isFriend(use.from) then
     return use.card:isKindOf("SavageAssault") or use.card:isKindOf("ArcheryAttack")
  end
end

--kinpika
sgs.ai_skill_invoke.tiansuo = function(self, data)
  local use = data:toCardUse()
  return self:willShowForAttack() and self:isEnemy(use.to:at(0))
end

caibao_skill={}
caibao_skill.name="caibao"
table.insert(sgs.ai_skills,caibao_skill)
caibao_skill.getTurnUseCard=function(self,inclusive)
    if not self:willShowForAttack() then return end
	if #self.enemies < 1 then return end
	local source = self.player
	if source:hasUsed("CaibaoCard") then return end
	if self.player:isKongcheng() then return end
	return sgs.Card_Parse("@CaibaoCard=.&caibao")
end

sgs.ai_skill_use_func.CaibaoCard = function(card,use,self)
	use.card = sgs.Card_Parse("@CaibaoCard=.&caibao")
	return
end


sgs.ai_use_value["CaibaoCard"] = 8
sgs.ai_use_priority["CaibaoCard"]  = 10
sgs.ai_card_intention.CaibaoCard = 70

sgs.ai_skill_playerchosen.caibao = function(self, targets)
	for _,p in ipairs(self.enemies) do
		if not p:isChained() and self:slashIsEffective(sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0), p) and self:isWeak(p) then return p end
	end
	for _,p in ipairs(self.enemies) do
		if not p:isChained() and self:slashIsEffective(sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0), p) then return p end
	end
	for _,p in ipairs(self.enemies) do
		if not p:isChained() and self:slashIsEffective(sgs.Sanguosha:cloneCard("fire_slash", sgs.Card_NoSuit, 0), p) then return p end
	end
	return self.enemies[1]
end

sgs.ai_skill_choice.caibao = function(self, choices, data)
	local brave_shine_num = 0
	for _,p in ipairs(self.enemies) do
		if p:isChained() then brave_shine_num = brave_shine_num + 1 end
	end
	local target = data:toPlayer()
	if target:getArmor() and target:getArmor():objectName()=="PeaceSpell" then return "caibaoslash" end
	if brave_shine_num > 0 and not target:hasShownSkill("tianhuo") and not target:hasShownSkill("huansha")then return "caibaofire_slash" end
	if brave_shine_num > 0 and not target:hasShownSkill("huansha") then return "caibaothunder_slash" end
	if not self:slashIsEffective(sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0), target) then return "caibaofire_slash" end
	return "caibaoslash"
end

--夕立
sgs.ai_skill_invoke.kuangquan = function(self, data)
  local damage = data:toDamage()
  return self:willShowForAttack() and self:isEnemy(damage.to) and not damage.to:hasShownSkill("rennai")
end

--黑雪姬
sgs.ai_skill_invoke.sexunyu = function(self, data)
  local use = data:toCardUse()
  if self.player:objectName()==use.from:objectName() then
    return self:willShowForAttack() and (self.player:getMark("drank") == 0 or (self:isEnemy(use.to:at(0)) and getCardsNum("Jink", use.to:at(0))> 0 and getCardsNum("Slash", use.to:at(0))<=self:getCardsNum("Slash") and not use.to:at(0):hasShownSkill("huansha")))
  end
  if use.to:contains(self.player) then
    return self:willShowForDefence() and (not self:isFriend(use.from) or use.to:length()==1)
  end
end

sgs.ai_skill_invoke.juedoujiasu = function(self, data)
   return self:willShowForDefence() or self:willShowForAttack()
end

sgs.ai_skill_invoke.jiasugaobai = function(self, data)   
   for _,p in ipairs(self.friends_noself) do
	 if p:isMale() and (self:getCardsNum("Slash") <2 or self.player:hasSkill("huansha")) then return true end
   end
end

sgs.ai_skill_playerchosen.jiasugaobai = function(self, targets)
   for _,p in ipairs(self.friends_noself) do
	 if p:isMale() then return p end
   end
end

--坂本
sgs.ai_skill_invoke.xianjing = function(self, data)
   if not data:toCardUse().from then
     return self:willShowForDefence() or self:willShowForAttack()
   else
   local use = data:toCardUse()
   local can
   for _,c in sgs.qlist(self.player:getHandcards()) do
      if use.card and use.card:getSuitString()==c:getSuitString() then can = true end
   end
   if not can then return false end
     return self:isEnemy(data:toCardUse().from)
   end
end

--柊司
sgs.ai_skill_choice.tianran = function(self, choices, data)
	local move = data:toMoveOneTime()
	local source
	for _,p in sgs.qlist(self.room:getAlivePlayers()) do
	  if p:objectName()==move.from:objectName() then source = p end
	end
	for _,f in ipairs(self.friends_noself) do
	  if f:objectName()~=source:objectName() and self.player:isFriendWith(f) and f:getHandcardNum()<self.player:getHandcardNum() and not f:hasFlag("tianran_cancel") then
	    return "cancel"
	  end
	end
	return "tianran_obtaincards"
end

sgs.ai_skill_invoke.tianran = function(self, data)
   for _,p in ipairs(self.friends_noself) do
	 if self.player:willBeFriendWith(p) then return self:willShowForDefence() or self:willShowForAttack() end
   end
   return false
end

liaoli_skill={}
liaoli_skill.name="liaoli"
table.insert(sgs.ai_skills,liaoli_skill)
liaoli_skill.getTurnUseCard=function(self,inclusive)
	local source = self.player
	if source:isKongcheng() then return end
	if source:hasUsed("LiaoliCard") then return end
	return sgs.Card_Parse("@LiaoliCard=.&liaoli")
end

sgs.ai_skill_use_func.LiaoliCard = function(card,use,self)
	local target
	local needed = {}
	local source = self.player
	for _,p in ipairs(self.friends) do
	  if p:getLostHp()==1 and p:getMaxHp()-p:getHandcardNum()>1 then
	     target = p
	  end
	end
	
	if not target then
	  local n =998
	  for _,p in ipairs(self.friends) do
	    if p:getHp()<n then
	       n = p:getHp()
	    end
	  end
	  for _,p in ipairs(self.friends) do
	    if p:getHp()==n and p:isWounded() then
		   target = p
		end
	  end
	end
	
	local cards=sgs.QList2Table(self.player:getHandcards())
	for _,acard in ipairs(cards) do
		if acard:isRed() and acard:isKindOf("BasicCard") and #needed==0 then
			table.insert(needed, acard:getEffectiveId())
		end
	end

	if target and #needed==1 then
		use.card = sgs.Card_Parse("@LiaoliCard="..table.concat(needed,"+").."&liaoli")
		if use.to then use.to:append(target) end
		return
	end
end

sgs.ai_use_value.LiaoliCard = 4
sgs.ai_use_priority.LiaoliCard  = 5

--觉
sgs.ai_cardneed.xiangqi = function(to, card, self)
	if not self:willSkipPlayPhase(to) then
		return  (not to:getWeapon() and  getCardsNum("Weapon",to,self.player)<1 and card:isKindOf("Weapon"))
		or (not to:getOffensiveHorse() and  getCardsNum("OffensiveHorse",to,self.player)<1 and card:isKindOf("OffensiveHorse"))
	end
end

sgs.ai_skill_invoke.xiangqi = function(self,data)
  if not self:willShowForAttack() and not self:willShowForDefence() then return false end
  local prompt = data:toString():split(":")
  local from
  local to
  for _,p in sgs.qlist(self.room:getAlivePlayers()) do
    if p:objectName()==prompt[2] then from = p end
	if p:objectName()==prompt[3] then to = p end
  end
  if  self:isEnemy(from) and not self:isFriend(to) then return true end
  if  self:isEnemy(to) and not self:isFriend(from) and from:getHandcardNum()>1 then return true end
  if  not self:isFriend(from) and to:objectName() == self.player:objectName() then return true end
  if  not self:isFriend(from) and self:isFriend(to) and from:getHandcardNum()>1 then return true end
  if  not self:isFriend(from) and self:isFriend(to) and to:getHp()>1 then return true end
  if  self:isEnemy(to) and to:getHp() <= 1 and self:isFriend(from) and from:getHandcardNum()>4 and from:getHandcardNum()>from:getHp()+1 then return true end
end

sgs.ai_skill_invoke.duxin = function(self,data)
  return self:willShowForAttack() or self:willShowForDefence()
end

--灵梦
sgs.ai_skill_invoke.mengfeng = function(self,data)
  return self:willShowForAttack() or self:willShowForDefence()
end

sgs.ai_skill_playerchosen.mengfeng = function(self, targets)
   for _,p in ipairs(self.friends) do
	 if p:getGeneral2Name() == "Kitagami" then return p end
   end
   return self.friends[1]
end

reimugive_skill={}
reimugive_skill.name="reimugive"
table.insert(sgs.ai_skills,reimugive_skill)
reimugive_skill.getTurnUseCard=function(self,inclusive)
	if #self.friends <= 1 then return end
	local source = self.player
	if source:isNude() then return end
	if source:hasUsed("ReimugiveCard") then return end
	return sgs.Card_Parse("@ReimugiveCard=.&reimugive")
end

sgs.ai_skill_use_func.ReimugiveCard = function(card,use,self)
	local target
	local source = self.player
	local max_x = 0
	for _,friend in ipairs(self.friends) do
		local x = 5 - friend:getHandcardNum()

		if x > max_x and friend:objectName() ~= source:objectName() and friend:hasShownSkill("saiqian") then
			max_x = x
			target = friend
		end
	end
	local cards=sgs.QList2Table(self.player:getHandcards())
	local equips=sgs.QList2Table(self.player:getEquips())
	local needed = {}
	for _,acard in ipairs(cards) do
		if #needed < 2 then
			table.insert(needed, acard:getEffectiveId())
		end
	end
	for _,acard in ipairs(equips) do
		if #needed < 2 then
			table.insert(needed, acard:getEffectiveId())
		end
	end
	if target and #needed>0 then
		use.card = sgs.Card_Parse("@ReimugiveCard="..table.concat(needed,"+").."&reimugive")
		if use.to then use.to:append(target) end
		return
	end
end

--魔理沙
sgs.ai_skill_invoke.mingqie = function(self, data)
	local use=data:toCardUse()
	for _,p in sgs.qlist(use.to) do
	  if (self:isFriend(p) and p:getJudgingArea():length()>0) or (self:isEnemy(p) and not p:isNude()) then
	    return true
	  end
	end
end

--克鲁特
sgs.ai_skill_invoke.wuming = function(self, data)
   local kingdoms = {}
   for _,p in sgs.qlist(self.room:getAlivePlayers()) do
   local can = true
   for _,name in ipairs(kingdoms) do
     if name == p:getKingdom() then can = false end
   end
   if ((p:hasShownGeneral1() or p:hasShownGeneral2()) and p:getRole()~="careerist" and can) then
                table.insert(kingdoms , p:getKingdom())
   end
   
   end
   
   local n = 0
   for _,name in pairs(self.enemies)do
			if not name:isKongcheng() then n=n+1 end
   end
   if #self.enemies< 2 or #kingdoms<2 or n<2 then return false end
   return self:willShowForAttack()
end

sgs.ai_skill_playerchosen.wuming = function(self)
    local kingdoms = {}
   for _,p in sgs.qlist(self.room:getAlivePlayers()) do
   local can = true
   for _,name in ipairs(kingdoms) do
     if name == p:getKingdom() then can = false end
   end
   if ((p:hasShownGeneral1() or p:hasShownGeneral2()) and p:getRole()~="careerist" and can) then
                table.insert(kingdoms , p:getKingdom())
   end
   end
    local result = {}
	for _,name in ipairs(self.enemies)do
		if  #result< #kingdoms and not name:isKongcheng() then table.insert(result, findPlayerByObjectName(name:objectName())) end
	end
	return result
end

local zhanshu_skill = {}
zhanshu_skill.name = "zhanshu"
table.insert(sgs.ai_skills, zhanshu_skill)
zhanshu_skill.getTurnUseCard = function(self,room,player,data)
	if self.player:hasUsed("ViewAsSkill_zhanshuCard") or self.player:isKongcheng() or (not self:willShowForAttack() and not self:willShowForDefence()) then return end
	local usevalue = 998
	local keepvalue = 998	
	local id
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)
	for _,card in ipairs(cards) do
		if self:getUseValue(card) < usevalue and self:getKeepValue(card)< keepvalue then
			id = tostring(card:getId())
			usevalue=self:getUseValue(card)
			keepvalue=self:getKeepValue(card)
		end
	end
	if not id then return end
	local parsed_card = {}
	for i=1,998,1 do
	   local c = sgs.Sanguosha:getCard(i-1)
	   if (c and c:isKindOf("TrickCard") and self.player:getMark(c:objectName().."zhanshu")==0) then
	      table.insert(parsed_card, sgs.Card_Parse(c:objectName()..":zhanshu[to_be_decided:0]=" .. id .."&zhanshu"))	
	   end
	end
	
	local value = 0
	local tcard
	for _, c in ipairs(parsed_card) do
		assert(c)
		if self:getUseValue(c) > value and self:getUseValue(c) > keepvalue and self:getUseValue(c) > usevalue then
			value = self:getUseValue(c)
			tcard = c
		end
	end
	if tcard and id then
		return tcard
	end
end

sgs.ai_skill_invoke.zhishu = function(self, data)
   local n=0
   local m=0
   for _,p in sgs.qlist(self.room:getAlivePlayers()) do
     if (p:isFriendWith(self.player)) then n=n+1 end
     if (not p:hasShownOneGeneral()) then m=m+1 end
   end
   if n>=3 or m<2 then
     return self:willShowForAttack()
   end
end

sgs.ai_skill_playerchosen.zhishu = function(self)   
    local result = {}
	for _,name in ipairs(self.friends)do
		if  self.player:isFriendWith(name) then table.insert(result, findPlayerByObjectName(name:objectName())) end
	end
	return result
end

--北上
sgs.ai_skill_playerchosen.leimu = function(self)
    local result = {}
	for _,name in ipairs(self.enemies)do
		if  #result< 3 then table.insert(result, findPlayerByObjectName(name:objectName())) end
	end
	return result
end

sgs.ai_skill_invoke.yezhan= function(self, data)
   if self:isEnemy(data:toDamage().to) then
     return self:willShowForAttack()
   end
end

sgs.ai_skill_playerchosen.yezhan = function(self, targets, max_num, min_num)
	for _, target in sgs.qlist(targets) do
		if self.player:isFriendWith(target) and target:hasShownAllGenerals() then return target end
	end
	return nil
end

--一方
sgs.ai_skill_invoke.vector= function(self, data)
   local can
   for _,c in sgs.qlist(self.player:getHandcards()) do
    if c:isKindOf("BasicCard") then can = true end
   end
   if not can then return false end
   local effect = data:toCardEffect()
   if effect.card:isKindOf("ThreatenEmperor") then return false end
	if self.player:isChained() then
		if effect.card:isKindOf("FightTogether") then return false end
		if effect.card:isKindOf("IronChain") then return false end
	end
	if effect.card:isKindOf("ImperialOrder") then return false end
	if effect.card:isKindOf("Peach") then return false end
	if effect.card:isKindOf("Analeptic") then return false end
	if effect.card:isKindOf("AmazingGrace") then return false end
	if effect.card:isKindOf("GodSalvation") then return false end
	if effect.card:isKindOf("ExNihilo") then return false end
	if effect.card:isKindOf("AwaitExhausted") then return false end
   return self:willShowForDefence() or self:willShowForAttack()
end

sgs.ai_skill_cardask["@vector-discard"] = function(self, data)
	if self.player:isKongcheng() then return "." end
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByUseValue(cards,true)
	local toUse
	for _,c in ipairs(cards) do
	  if c:isKindOf("BasicCard") then
	     toUse = "$" .. c:getEffectiveId()
	  end
	end
	local effect = data:toCardEffect()
	local card = effect.card
	if effect.card:isKindOf("ThreatenEmperor") then return "." end
	if self.player:isChained() then
		if effect.card:isKindOf("FightTogether") then return "." end
		if effect.card:isKindOf("IronChain") then return "." end
	end
	if effect.card:isKindOf("ImperialOrder") then return "." end
	if effect.card:isKindOf("Peach") then return "." end
	if effect.card:isKindOf("Analeptic") then return "."end
	if effect.card:isKindOf("AmazingGrace") then return "." end
	if effect.card:isKindOf("GodSalvation") then return "." end
	if effect.card:isKindOf("ExNihilo") then return "." end
	if effect.card:isKindOf("AwaitExhausted") then return "." end
	local power = self.player:getHp() * 2 + self.player:getHandcardNum()
	local cardN = self.player:getCards("he"):length()
	local hp = self.player:getHp()
	if card:isKindOf("AOE") then
		if power > 9 or cardN > hp or power < 4 or hp == 1 then
			return toUse
		end
		return "."
	elseif card:isKindOf("Snatch") or card:isKindOf("Dismantlement") or card:isKindOf("Collateral") or card:isKindOf("Duel") then return toUse
	elseif card:isKindOf("ex_nihilo") then
		self.room:setPlayerFlag(self.player,"vecter_friend")
		if power > 10 then return toUse end
		return "."
	elseif card:isKindOf("Slash") then
		if self:isFriend(effect.from) then return "." end
		if #self.enemies == 0 then return "." end
		self.room:setPlayerFlag(self.player,"vecter_slash")
		return toUse
	end
	return toUse
end

local function doVector(who, self)
	if not who then return false end
	if who:objectName() == self.player:objectName() then return false end
	if not self:isFriend(who) and who:hasSkill("leiji") and (self:hasSuit("spade", true, who) or who:getHandcardNum() >= 3) and (getKnownCard(who, "Jink", true) >= 1 or self:hasEightDiagramEffect(who)) then
		return false
	end

	local cards = self.player:getCards("h")
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)
	for _, card in ipairs(cards) do
		if not self.player:isCardLimited(card, method) then
			if self:isFriend(who) then
				return false
			else
				return true
			end
		end
	end

	local cards = self.player:getCards("e")
	cards=sgs.QList2Table(cards)
	self:sortByKeepValue(cards)
	for _, card in ipairs(cards) do
		if not self.player:isCardLimited(card, method) then
			return true
		end
	end
	return false
end

sgs.ai_skill_playerchosen.vector = function(self, targets)
    return self:getPriorTarget()
	--[[if self.player:hasFlag("vecter_friend") then
		return self:findPlayerToDraw(false, 2)
	elseif self.player:hasFlag("vecter_slash") then
		for _,ememy in ipairs(self.enemies) do
			if doVector(enemy, self) then return enemy end
		end
		for _,ememy in ipairs(self.enemies) do
			return enemy
		end
	else
		return self:findPlayerToDiscard()
	end]]
end

local bianhua_skill={}
bianhua_skill.name="bianhua"
table.insert(sgs.ai_skills,bianhua_skill)
bianhua_skill.getTurnUseCard=function(self,inclusive)
    if self.player:usedTimes("BianhuaCard")>1 or (not self:willShowForAttack() and not self:willShowForDefence()) then return false end
	local cards = sgs.QList2Table(self.player:getHandcards())
	local equips = sgs.QList2Table(self.player:getEquips())
	self:sortByUseValue(cards,true)
	self:sortByUseValue(equips,true)
	for _,acard in ipairs(cards) do
		if self:getKeepValue(acard)<5 then
			return sgs.Card_Parse("@BianhuaCard=.&bianhua")
		end
	end
	for _,acard in ipairs(equips) do
		if self:getKeepValue(acard)<5 then
			return sgs.Card_Parse("@BianhuaCard=.&bianhua")
		end
	end
end

sgs.ai_skill_use_func.BianhuaCard = function(card,use,self)
	local needed
	local cards = sgs.QList2Table(self.player:getHandcards())
	local equips=sgs.QList2Table(self.player:getEquips())
	for _,acard in ipairs(equips) do
		if self:getKeepValue(acard)<5 then
			needed = acard
		end
	end
	for _,acard in ipairs(cards) do
		if self:getKeepValue(acard)<5 then
			needed = acard
		end
	end
	if needed then
		use.card = sgs.Card_Parse("@BianhuaCard="..needed:getEffectiveId().."&bianhua")
		return
	end
end

--助手
sgs.ai_skill_invoke.tiancai= function(self, data)
   local can
   for _,p in ipairs(self.friends) do
      if self.player:isFriendWith(p) and p:getHp()>self.player:getHp() then can = true end
   end
   return self:willShowForDefence() or self:willShowForAttack() and (self.player:getPhase()~=sgs.Player_Draw or self.player:getHp()<= 2 or can) 
end

sgs.ai_skill_invoke.zhushou= function(self, data)
   local current
   for _,p in sgs.qlist(self.room:getAlivePlayers()) do
      if p:getPhase()==sgs.Player_Play then current = p end
   end
   if not current then return end
   for _,p in ipairs(self.friends) do
      if current:isFriendWith(p) and p:getHp()>current:getHp() then return false end
   end
   return self:willShowForDefence() or self:willShowForAttack()  
end

--早苗
sgs.ai_skill_invoke.jiyi= function(self, data)
  if self.player:getPhase()==sgs.Player_Draw then return self:willShowForAttack() or self:willShowForDefence() end
  return true
end

local zmqiji_skill = {}
zmqiji_skill.name = "zmqiji"
table.insert(sgs.ai_skills, zmqiji_skill)
zmqiji_skill.getTurnUseCard = function(self,room,player,data)
	if self.player:hasUsed("ViewAsSkill_zmqijiCard") or self.player:getHandcardNum()~=1 or (not self:willShowForAttack() and not self:willShowForDefence()) then return end
	local usevalue = 998
	local keepvalue = 998	
	local id
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	card = cards[1]
	id = tostring(card:getId())
	usevalue=self:getUseValue(card)
	keepvalue=self:getKeepValue(card)
	if not id then return end
	local parsed_card = {}
	for i=1,998,1 do
	   local c = sgs.Sanguosha:getCard(i-1)
	   if c and (c:isKindOf("BasicCard") or c:isNDTrick()) then
	      table.insert(parsed_card, sgs.Card_Parse(c:objectName()..":zmqiji["..card:getSuitString()..":"..card:getNumberString().."]=" .. id .."&zmqiji"))	
	   end
	end
	
	local value = 0
	local tcard
	for _, c in ipairs(parsed_card) do
		assert(c)
		if self:getUseValue(c) > value and self:getUseValue(c) > keepvalue and self:getUseValue(c) > usevalue then
			value = self:getUseValue(c)
			tcard = c
		end
	end
	if tcard and id then
		return tcard
	end
end

--saber
local function add_different_kingdoms(self, target, targets)
   for _,name in ipairs(targets) do
     local p = findPlayerByObjectName(name)
	 if target:isFriendWith(p) then return false end
   end
   return true
end

sgs.ai_skill_invoke.shengjian= function(self, data)
   for _,enemy in ipairs(self.enemies) do
      if enemy:getEquips():length()>0 then return self:willShowForAttack() end
   end
end

sgs.ai_skill_use["@@shengjian"] = function(self, prompt)
	if not self:willShowForAttack() then
		return "."
	end
	local targets = {}
	local n = self.player:getAttackRange()
	for _,p in ipairs(self.enemies) do
	  if #targets <n and p:getEquips():length()>0 then table.insert(targets, p:objectName()) end
	end
	for _,p in ipairs(self.enemies) do
	  if #targets <n and p:getEquips():length()==0 then table.insert(targets, p:objectName()) end
	end
	if type(targets) == "table" and #targets > 0 then
		return ("@ShengjianCard=.&->" .. table.concat(targets, "+"))
	end
	return "."
end

--Azura
azuyizhi_skill={}
azuyizhi_skill.name="azuyizhi"
table.insert(sgs.ai_skills,azuyizhi_skill)
azuyizhi_skill.getTurnUseCard=function(self,inclusive)
    if not self:willShowForAttack() and not self:willShowForDefence() then return end
	local source = self.player
	if source:hasUsed("AzuyizhiCard") then return end
	if source:getHp()<=1 and self:getCardsNum("Peach") == 0 and self:getCardsNum("Analeptic") == 0 then return end
	return sgs.Card_Parse("@AzuyizhiCard=.&azuyizhi")
end

sgs.ai_skill_use_func.AzuyizhiCard = function(card,use,self)
	local target
	local source = self.player	
	for _,enemy in ipairs(self.enemies) do
	    if enemy:faceUp() then
		  target = enemy
		end
	end
	for _,enemy in ipairs(self.enemies) do
	    if enemy:faceUp() and not (enemy:getWeapon() or enemy:isKongcheng()) then
		  target = enemy
		end
	end
	for _,friend in ipairs(self.friends) do
	    if not friend:faceUp() then
		  target = friend
		end
	end
	if target then
		use.card = sgs.Card_Parse("@AzuyizhiCard=.&azuyizhi")
		if use.to then use.to:append(target) end
		return
	end
end

sgs.ai_skill_use["@@yizhiresponse"] = function(self, prompt)
	if not self.player:faceUp() or self.player:isRemoved() then
		return "."
	end
	local cards=sgs.QList2Table(self.player:getHandcards())
	local equips=sgs.QList2Table(self.player:getEquips())
	local needed = {}
	for _,acard in ipairs(cards) do
		if #needed == 0 and acard:isKindOf("Slash") then
			table.insert(needed, acard:getEffectiveId())
		end
	end
	for _,acard in ipairs(equips) do
		if #needed == 1 and acard:isKindOf("Weapon") then
			table.insert(needed, acard:getEffectiveId())
		end
	end
	for _,acard in ipairs(cards) do
		if #needed == 1 and acard:isKindOf("Weapon") then
			table.insert(needed, acard:getEffectiveId())
		end
	end
	if #needed==2 then
		return ("@YizhiresponseCard="..table.concat(needed, "+").."&")
	end
	return "."
end

--露易丝
local lingjie_skill={}
lingjie_skill.name="lingjie"
table.insert(sgs.ai_skills,lingjie_skill)
lingjie_skill.getTurnUseCard=function(self,inclusive)
	if self.player:hasUsed("LingjieCard") or not self:willShowForAttack() then return end
	return sgs.Card_Parse("@LingjieCard=.&lingjie")
end

sgs.ai_skill_use_func.LingjieCard = function(card,use,self)
    local target
	for _,enemy in ipairs(self.enemies) do
		if not enemy:isChained() and  enemy:hasShownOneGeneral() then
			target = enemy
		end
	end
    if target then
	  use.card = sgs.Card_Parse("@LingjieCard=.&lingjie")
	  if use.to then use.to:append(target) end
	  return
	end
end

sgs.ai_use_value.LingjieCard = 20
sgs.ai_use_priority.LingjieCard = 9

local xuwu_skill={}
xuwu_skill.name="xuwu"
table.insert(sgs.ai_skills,xuwu_skill)
xuwu_skill.getTurnUseCard=function(self,inclusive)
	if not self:willShowForAttack() then return end
	return sgs.Card_Parse("@XuwuCard=.&xuwu")
end

sgs.ai_skill_use_func.XuwuCard = function(card,use,self)
    local card
    local hecards = self.player:getCards("he")
	for _, c in sgs.qlist(hecards) do
	  if c:isKindOf("EquipCard") then card = c end
	end
	if self.player:getPhase()~=sgs.Player_NotActive and self.player:usedTimes("XuwuCard") >=3 then return end
    if card then
	  use.card = sgs.Card_Parse("@XuwuCard="..card:getEffectiveId().."&xuwu")
	  return
	end
end

sgs.ai_skill_use["@@xuwu"] = function(self, prompt)
	if not self:willShowForDefence() and not self:willShowForAttack() then
		return "."
	end
	local card
    local hecards = self.player:getCards("he")
	for _, c in sgs.qlist(hecards) do
	  if c:isKindOf("EquipCard") then card = c end
	end
	if card then
		return ("@XuwuCard="..card:getEffectiveId().."&->")
	end
	return "."
end

--K1
sgs.ai_skill_invoke.qiubang= function(self, data)
  return self:willShowForAttack()
end

sgs.ai_skill_invoke.randong= function(self, data)
  local use = data:toCardUse()
  local dest = use.to:at(0)
  if self:isEnemy(use.from) then
     return true  
  end
end

--koromo
sgs.ai_skill_invoke.kongyun= function(self, data)
  return not self:isFriend(self.room:getCurrent()) and (self:willShowForAttack() or self:willShowForDefence())
end

sgs.ai_skill_invoke.laoyue= function(self, data)
  return self:willShowForDefence()
end

sgs.ai_skill_choice.laoyue = function(self, choices, data)
   if table.contains(choices:split("+"), "use")
      then return "use"
   else
      return "replace"
   end
end

sgs.ai_skill_askforag.laoyue = function(self, card_ids)
  return card_ids[1]
end

--Rentarou
sgs.ai_skill_invoke.huodan= function(self, data)
  local damage = data:toDamage()
  return self:willShowForAttack() and self:isEnemy(damage.to)
end

sgs.ai_view_as.xieti = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if (card_place ~= sgs.Player_PlaceSpecial or player:getHandPile():contains(card_id)) and card:isKindOf("EquipCard") then
		return ("slash:xieti[%s:%s]=%d&xieti"):format(suit, number, card_id)
	end
end

local xieti_skill = {}
xieti_skill.name = "xieti"
table.insert(sgs.ai_skills, xieti_skill)
xieti_skill.getTurnUseCard = function(self, inclusive)

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

	local hecards = self.player:getCards("he")
	for _, id in sgs.qlist(self.player:getHandPile()) do
		hecards:prepend(sgs.Sanguosha:getCard(id))
	end
	local cards = {}
	for _, card in sgs.qlist(hecards) do
		if card:isKindOf("EquipCard")
			and (not isCard("Crossbow", card, self.player) or disCrossbow ) then
			local suit = card:getSuitString()
			local number = card:getNumberString()
			local card_id = card:getEffectiveId()
			local card_str = ("slash:xieti[%s:%s]=%d&xieti"):format(suit, number, card_id)
			local slash = sgs.Card_Parse(card_str)
			assert(slash)
			if self:slashIsAvailable(self.player, slash) then
				table.insert(cards, slash)
			end
		end
	end

	if #cards == 0 then return end

	self:sortByUsePriority(cards)
	return cards[1]
end

function sgs.ai_cardneed.xieti(to, card)
	return to:getHandcardNum() < 3 and card:isKindOf("EquipCard")
end

sgs.ai_skill_invoke.xieti= function(self, data)
  local dest = data:toPlayer()
  local room = self.room
  if dest then return not self:isFriend(dest) end
  if not dest then
    if self:isEnemy(self.player:getNextAlive()) or (self:isFriend(self.player:getNextAlive():getNextAlive()) and not self:isFriend(self.player:getNextAlive())) then
	   room:setPlayerFlag(self.player, "xieti_right")
	   return true
	end
	if self:isFriend(self.player:getLastAlive()) then
	   room:setPlayerFlag(self.player, "xieti_left")
	   return true
	end
  end
end

sgs.ai_skill_choice.xieti = function(self, choices, data)
   if self.player:hasFlag("xieti_left") then
     return "move_left"
   else
     return "move_right"
   end
   local room = self.room
   room:setPlayerFlag(self.player, "-xieti_right")
   room:setPlayerFlag(self.player, "-xieti_left")
end

--Asuna
sgs.ai_skill_invoke.shanyao = function(self, data)
  local effect = data:toSlashEffect()
  return self:willShowForAttack() and self:isEnemy(effect.to)
end

sgs.ai_skill_invoke.jiansu = function(self, data)
  local dest = self.room:getCurrent()
  local can
  for _,enemy in ipairs(self.enemies) do
    if self:isWeak(enemy) then can = true end
  end
  return #self.enemies>0 and self:willShowForAttack() and (self:getCardsNum("Slash")>0 or dest:getHandcardNum()>1) and (self:isFriend(dest) or can)
end

sgs.ai_skill_playerchosen.jiansu = function(self, targets)
  local n = 998
  local target
  for _,p in sgs.qlist(self.room:getAlivePlayers()) do
    if self:isEnemy(p) and p:objectName()~=self.player:objectName() and p:objectName()~=self.room:getCurrent():objectName() and p:getHp()<n then
	  target = p
	  n = p:getHp()
	end
  end
  if target then return target end
end

--妲利安
sgs.ai_skill_invoke.shuji = function(self, data)
  return self:willShowForAttack() or self:willShowForDefence()
end

sgs.ai_skill_discard["shuji"] = function(self, discard_num, min_num, optional, include_equip)
	local card_id = self.room:getTag("shuji-card"):toInt()
	if not card_id or card_id == -1 then return end

	if self.player:getHandcardNum() == 0 then return end

	--土豪
	if self.player:getPile("huanshu"):length() <= 3 and self.player:getHandcardNum() >= 2 then return self:askForDiscard("discard", discard_num, min_num, false, include_equip) end

	-- 达利安需要考虑的一些情况： 1.尽可能收束需要的锦囊花色  2.依据类型而定，最有价值的主要是顺手牵羊 无中生有 无懈可击 3.根据情况而怂，比如自己快死了
	if self:isWeak() and self.player:getHandcardNum() <= 1 and (self:getCardsNum("Peach") > 0 or self:getCardsNum("Jink") > 0 or self:getCardsNum("Analeptic") > 0) then return end


	local rcard = sgs.Sanguosha:getCard(card_id)

	if self:getUseValue(rcard) >= 6 then return self:askForDiscard("discard", discard_num, min_num, false, include_equip) end

	local same = 0
	for _, id in sgs.list(self.player:getPile("huanshu")) do
		local card = sgs.Sanguosha:getCard(id)
		if card:getSuit() == rcard:getSuit() then same = same + 1 end
	end

	if same >= 1 then return self:askForDiscard("discard", discard_num, min_num, false, include_equip) end

	return
end

sgs.ai_skill_invoke.shoushi = function(self, data)
	local use = data:toCardUse()
	return (not use.card:isKindOf("ExNihilo") and not use.card:isKindOf("AmazingGrace") and not use.card:isKindOf("GodSalvation")) or self:isFriend(use.from)
end

sgs.ai_skill_invoke.jicheng = function(self, data)
  local n = math.random(1,2)
  if n==1 then return true end
  return false
end

sgs.ai_skill_invoke.kaiqi = function(self, data)
  return self:willShowForAttack() or self:willShowForDefence()
end

sgs.kaiqi_ag_type = ""
sgs.ai_skill_discard["kaiqi"] = function(self, discard_num, min_num, optional, include_equip)
	--先测评一下有没有人要给

	--开启的一些逻辑
	--尽量保证剩余的书里有3个的花色
	--可以把一些其他的花色的牌分给队友
	--如果是红桃且有队友的话自己拿无中生有的效果很好
	--如果黑桃且队友基本没有的情况下可以考虑拿顺手牵羊
	local heart, diamond, spade, club = 0,0,0,0
	local ex, sn, need = nil, nil, nil, nil, nil, nil

	for _, id in sgs.list(self.player:getPile("huanshu")) do
		local card = sgs.Sanguosha:getCard(id)
		if card:getSuit() == sgs.Card_Heart then heart = heart + 1 end
		if card:getSuit() == sgs.Card_Diamond then diamond = diamond + 1 end
		if card:getSuit() == sgs.Card_Spade then spade = spade + 1 end
		if card:getSuit() == sgs.Card_Club then club = club + 1 end
		if card:isKindOf("ExNihilo") then ex = card end
		if card:isKindOf("Snatch") then sn = card end
		if self:getUseValue(card) >= 6 then need = card end
	end

	if not self.player:hasFlag("kaiqi_self_used") then
		if heart > 3 then
			sgs.kaiqi_ag_type = "heart"
			return self:askForDiscard("discard", discard_num, min_num, false, include_equip)
		end
		if diamond > 3 then
			sgs.kaiqi_ag_type = "diamond"
			return self:askForDiscard("discard", discard_num, min_num, false, include_equip)
		end
		if spade > 3 then
			sgs.kaiqi_ag_type = "spade"
			return self:askForDiscard("discard", discard_num, min_num, false, include_equip)
		end
		if club > 3 then
			sgs.kaiqi_ag_type = "club"
			return self:askForDiscard("discard", discard_num, min_num, false, include_equip)
		end
		if heart > 2 and ((ex and ex:getSuit() == sgs.Card_Heart) or (need and need:getSuit() == sgs.Card_Heart)) then
			sgs.kaiqi_ag_type = "heart"
			return self:askForDiscard("discard", discard_num, min_num, false, include_equip)
		end
		if diamond > 2 and need and need:getSuit() == sgs.Card_Diamond then
			sgs.kaiqi_ag_type = "diamond"
			return self:askForDiscard("discard", discard_num, min_num, false, include_equip)
		end
		if spade > 2 and need and need:getSuit() == sgs.Card_Spade then
			sgs.kaiqi_ag_type = "spade"
			return self:askForDiscard("discard", discard_num, min_num, false, include_equip)
		end
		if club > 2 and need and need:getSuit() == sgs.Card_Club then
			sgs.kaiqi_ag_type = "club"
			return self:askForDiscard("discard", discard_num, min_num, false, include_equip)
		end
	else
		--是否有人可送
		local has = false
		for _,p in ipairs(self.friends_noself) do
			if not p:hasFlag("kaiqi_used") then has = true end
		end

		if has then
			if self:isWeak() or self.player:getPile("huanshu"):length() <= 5 or self.player:getHandcardNum() < 2 then return "." end
			if heart > 3 then
				sgs.kaiqi_ag_type = "heart"
				return self:askForDiscard("discard", discard_num, min_num, false, include_equip)
			end
			if diamond > 3 then
				sgs.kaiqi_ag_type = "diamond"
				return self:askForDiscard("discard", discard_num, min_num, false, include_equip)
			end
			if spade > 3 then
				sgs.kaiqi_ag_type = "spade"
				return self:askForDiscard("discard", discard_num, min_num, false, include_equip)
			end
			if club > 3 then
				sgs.kaiqi_ag_type = "club"
				return self:askForDiscard("discard", discard_num, min_num, false, include_equip)
			end
			if diamond == 1 then
				sgs.kaiqi_ag_type = "diamond"
				return self:askForDiscard("discard", discard_num, min_num, false, include_equip)
			end
			if spade == 1 then
				sgs.kaiqi_ag_type = "spade"
				return self:askForDiscard("discard", discard_num, min_num, false, include_equip)
			end
			if club == 1 then
				sgs.kaiqi_ag_type = "club"
				return self:askForDiscard("discard", discard_num, min_num, false, include_equip)
			end
		end
	end
	return {}
end

--get AG
sgs.ai_skill_askforag.kaiqi = function(self, card_ids)
	local sel = {}
	for _,card_id in sgs.list(card_ids) do
		local card = sgs.Sanguosha:getCard(card_id)
		if card:getSuitString() == sgs.kaiqi_ag_type then
			if card:isKindOf("ExNihilo") then return card_id end
			table.insert(sel, card)
		end
	end

	self:sortByUseValue(sel)
	if #sel > 0 then return sel[1]:getId() end
	return card_ids[1]
end



sgs.ai_skill_playerchosen.kaiqi = function(self, targets)
	if not self.player:hasFlag("kaiqi_self_used") then
		self.player:setFlags("kaiqi_self_used")
		return self.player
	end
	for _, target in sgs.list(targets) do
		if self:isFriend(target) and not self:isWeak(target) then
			target:setFlags("kaiqi_used")
			return target
		end
	end

	for _, target in sgs.list(targets) do
		if self:isFriend(target) then
			target:setFlags("kaiqi_used")
			return target
		end
	end
end

--Kotarou
gaixie_skill={}
gaixie_skill.name="gaixie"
table.insert(sgs.ai_skills,gaixie_skill)
gaixie_skill.getTurnUseCard=function(self,inclusive)
	local source = self.player
	if source:hasUsed("GaixieCard") then return end
	if (not self:willShowForAttack() and not self:willShowForDefence()) or not source:isWounded() then return end
	if source:getMaxHp()<=1 then return end
	return sgs.Card_Parse("@GaixieCard=.&gaixie")
end

sgs.ai_skill_use_func.GaixieCard = function(card,use,self)
	use.card = sgs.Card_Parse("@GaixieCard=.&gaixie")
	return
end

--刀子
sgs.ai_skill_invoke.tulong = function(self, data)
  local damage = data:toDamage()
  local use = data:toCardUse()
  if self:willShowForAttack() then
     if damage and damage.to then return self:isEnemy(damage.to) end
	 if use then return self:isEnemy(use.to:at(0)) end
  end
  return false
end

sgs.ai_skill_invoke.congyun = function(self, data)
  local use = data:toCardUse()
  return self:willShowForAttack() or self:willShowForDefence()
end

--koishi
maihuo_skill={}
maihuo_skill.name="maihuo"
table.insert(sgs.ai_skills,maihuo_skill)
maihuo_skill.getTurnUseCard=function(self,inclusive)
	if #self.friends <= 1 then return end
	local source = self.player
	if source:isKongcheng() then return end
	if source:hasUsed("MaihuoCard") then return end
	return sgs.Card_Parse("@MaihuoCard=.&maihuo")
end

sgs.ai_skill_use_func.MaihuoCard = function(card,use,self)
	local target
	local needed = {}
	local source = self.player
	if #self.friends_noself>0 then
	  target = self.friends_noself[math.random(1, #self.friends_noself)]
	end
	local cards=sgs.QList2Table(self.player:getHandcards())
	for _,acard in ipairs(cards) do
		if acard:isRed() and #needed==0 then
			table.insert(needed, acard:getEffectiveId())
		end
	end
	for _,acard in ipairs(cards) do
		if #needed==0 then
			table.insert(needed, acard:getEffectiveId())
		end
	end
	if target and #needed==1 then
		use.card = sgs.Card_Parse("@MaihuoCard="..table.concat(needed,"+").."&maihuo")
		if use.to then use.to:append(target) end
		return
	end
end

sgs.ai_skill_invoke.wunian = function(self, data)
  local use = data:toCardUse()
  if self:willShowForDefence() and self:isEnemy(use.from) and (use.card and not use.card:isKindOf("AmazingGrace")) and (use.card and not use.card:isKindOf("GodSalvation")) then return true end
  return false
end

sgs.ai_use_value.MaihuoCard = 7
sgs.ai_use_priority.MaihuoCard  = 5
sgs.ai_card_intention.MaihuoCard  = -60

--柊镜
sgs.ai_skill_invoke.zhengchang = function(self, data)
   local use = data:toCardUse()
   if not use then return self:willShowForDefence() end
   if use then return self:isEnemy(use.from) end
end

sgs.ai_skill_invoke.tucao = function(self, data)
  if not self:willShowForAttack() and not self:willShowForDefence() then
    return false
  end
  local use = data:toCardUse()
  local card = use.card
  
  if not use then return false end
  
  if self:isFriend(use.from)then
    for _,c in sgs.qlist(self.player:getHandcards()) do
	  if c:isRed() and c:getTypeId()~= use.card:getTypeId() then
	     return true 
	  end
	end
  end
  
  if self:isEnemy(use.from) then
    local can
    for _,f in ipairs(self.friends) do
		if use.to:contains(f) then
			can = true
		end
	end
	for _,c in sgs.qlist(self.player:getHandcards()) do
	  if can and c:isBlack() and c:getTypeId()~= use.card:getTypeId() then
	     return true 
	  end
	end
  end
  
end

sgs.ai_skill_cardask["@tucao"] = function(self, data)
  local use = data:toCardUse()
  local card = use.card
  for _, c in sgs.qlist(self.player:getCards("h")) do
	 if self:isFriend(use.from) and c:isRed() and c:getTypeId()~= use.card:getTypeId() then
		return "$" .. c:getEffectiveId()
	 end
	 if self:isEnemy(use.from) and c:isBlack() and c:getTypeId()~= use.card:getTypeId() then
	    return "$" .. c:getEffectiveId()
	 end
  end
end

--okarin
shixian_skill={}
shixian_skill.name="shixian"
table.insert(sgs.ai_skills,shixian_skill)
shixian_skill.getTurnUseCard=function(self,inclusive)
	local source = self.player
	if source:isNude() then return end
	if source:hasUsed("ShixianCard") then return end
	if source:getPile("shikongcundang"):length()>0 then return end
	return sgs.Card_Parse("@ShixianCard=.&shixian")
end

sgs.ai_skill_use_func.ShixianCard = function(card,use,self)
	local source = self.player
	local max_num = source:getMaxHp()
	local cards=sgs.QList2Table(self.player:getHandcards())
	local equips=sgs.QList2Table(self.player:getEquips())
	local needed = {}
	for _,acard in ipairs(cards) do
		if #needed < max_num then
			table.insert(needed, acard:getEffectiveId())
		end
	end
	for _,acard in ipairs(equips) do
		if #needed < max_num then
			table.insert(needed, acard:getEffectiveId())
		end
	end
	if #needed>0 then
		use.card = sgs.Card_Parse("@ShixianCard="..table.concat(needed,"+").."&shixian")
		return
	end
end

sgs.ai_use_value.ShixianCard = 4
sgs.ai_use_priority.ShixianCard  = 5

sgs.ai_skill_invoke.jiaxiang = function(self, data)
  local use = data:toCardUse()
  if use then return self:isEnemy(use.from) end
  if not use then return true end
end

tiaoyue_skill={}
tiaoyue_skill.name="tiaoyue"
table.insert(sgs.ai_skills,tiaoyue_skill)
tiaoyue_skill.getTurnUseCard=function(self,inclusive)
	local source = self.player
	if source:hasUsed("TiaoyueCard") then return end
	if source:hasUsed("ShixianCard") then return end
	if source:getPile("shikongcundang"):length()==0 then return end
	return sgs.Card_Parse("@TiaoyueCard=.&tiaoyue")
end

sgs.ai_skill_use_func.TiaoyueCard = function(card,use,self)
	local source = self.player
	local max_num = math.ceil(source:getPile("shikongcundang"):length()/2)
	local cards=sgs.QList2Table(self.player:getHandcards())
	local equips=sgs.QList2Table(self.player:getEquips())
	local needed = {}
	for _,acard in ipairs(cards) do
		if #needed < max_num and not acard:isKindOf("BasicCard") then
			table.insert(needed, acard:getEffectiveId())
		end
	end
	for _,acard in ipairs(equips) do
		if #needed < max_num and not acard:isKindOf("BasicCard") then
			table.insert(needed, acard:getEffectiveId())
		end
	end
	if #needed == max_num then
		use.card = sgs.Card_Parse("@TiaoyueCard="..table.concat(needed,"+").."&tiaoyue")
		return
	end
end

sgs.ai_use_value.TiaoyueCard = 5
sgs.ai_use_priority.TiaoyueCard  = 7

--Fubuki
sgs.ai_skill_invoke.qianlei = function(self, data)
	local dying_data = data:toDying()
	local damage = dying_data.damage
	local der = dying_data.who
	return self:isEnemy(der) or self:isEnemy(damage.from)
end

sgs.ai_skill_choice["qianlei"] = function(self, choices, data)
	local dying_data = data:toDying()
	local damage = dying_data.damage
	local der = dying_data.who
	if self:isEnemy(der) then return "se_qianlei_second" end
	return "se_qianlei_first"
end

sgs.ai_skill_invoke.shuacun = function(self, data)
  return self:willShowForAttack() or self:willShowForDefence()
end

--Kazuma

qiequ_skill={}
qiequ_skill.name="qiequ"
table.insert(sgs.ai_skills,qiequ_skill)
qiequ_skill.getTurnUseCard=function(self,inclusive)
    if not self:willShowForAttack() and not self:willShowForDefence() then return end
	if #self.enemies < 1 then return end
	local source = self.player
	if source:hasUsed("QiequCard") then return end
	return sgs.Card_Parse("@QiequCard=.&qiequ")
end

sgs.ai_skill_use_func.QiequCard = function(card,use,self)
	local target
	local source = self.player
	for _,enemy in ipairs(self.enemies) do
	    target = enemy
	end

	if target then
		use.card = sgs.Card_Parse("@QiequCard=.&qiequ")
		if use.to then use.to:append(target) end
		return
	end
end

sgs.ai_use_value["QiequCard"] = 8
sgs.ai_use_priority["QiequCard"]  = 10
sgs.ai_card_intention.QiequCard = 90

sgs.ai_skill_choice.qiangyun = function(self, choices, data)
	local judge = data:toJudge()
	
	if judge and judge.reason == "qiequ" then
	local n = math.random(1,#targets)
	if n == 0 then 
	  return "heart"
	else
	  return "diamond"
	end
	end
end

local gongfang_skill={}
gongfang_skill.name="gongfang"
table.insert(sgs.ai_skills,gongfang_skill)
gongfang_skill.getTurnUseCard=function(self,inclusive)
    if self.player:usedTimes("GongfangCard")>2 or (not self:willShowForAttack() and not self:willShowForDefence()) then return false end
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByUseValue(cards,true)
	for _,acard in ipairs(cards) do
		if self:getKeepValue(acard)<5 then
			return sgs.Card_Parse("@GongfangCard=.&gongfang")
		end
	end
end

sgs.ai_skill_use_func.GongfangCard = function(card,use,self)
	local needed
	local cards = sgs.QList2Table(self.player:getHandcards())
	for _,acard in ipairs(cards) do
		if self:getKeepValue(acard)<5 then
			needed = acard
		end
	end
	if needed then
		use.card = sgs.Card_Parse("@GongfangCard="..needed:getEffectiveId().."&gongfang")
		return
	end
end