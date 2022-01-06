
function SmartAI:useCardGuangyuCard(card, use)
    local room=self.room
	for _,v in ipairs(self.friends) do
		if not v:faceUp() then
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
		if v:containsTrick("keyCard") and v:isWounded() then
			use.card = card
			if use.to then use.to:append(v) end
			return
		end
	end
	for _,v in ipairs(self.friends) do
		if v:isChained() then
			use.card = card
			if use.to then use.to:append(v) end
			return
		end
	end
	for _,v in ipairs(self.friends) do
		if v:hasFlag("Global_Dying") then
			use.card = card
			if use.to then use.to:append(v) end
			return
		end
	end
end
sgs.ai_use_priority.GuangyuCard = 1.2
sgs.ai_use_value.GuangyuCard = 6
sgs.ai_keep_value.GuangyuCard = 6
sgs.ai_card_intention.GuangyuCard = -60

function SmartAI:useCardEireishoukan(card, use)
    if not card:isAvailable(self.player) then return end
	if not self:hasTrickEffective(card, self.player, self.player) then return end
	use.card = card
end
sgs.ai_use_priority.Eireishoukan = 3
sgs.ai_use_value.Eireishoukan = 6.5
sgs.ai_keep_value.Eireishoukan = 3
sgs.ai_card_intention.Eireishoukan= -60

function SmartAI:useCardIsekai(card, use)
    if not card:isAvailable(self.player) then return end
	if not self:hasTrickEffective(card, self.player, self.player) then return end
	use.card = card
end
sgs.ai_use_priority.Isekai = 0
sgs.ai_use_value.Isekai = 6.5
sgs.ai_keep_value.Isekai= 3
sgs.ai_card_intention.Isekai= -60


sgs.ai_skill_choice.isekai = function(self, choices, data)
   local n = data:toInt()
   local m = self.player:getMaxHp()
   if n<=m then
      return "draw_maxhpcards_recover"
   end
   if m<n then
      return "draw_throwcards"
   end
end

--for shifeng
sgs.ai_skill_use["BasicCard+^Jink,TrickCard+^Nullification,EquipCard|.|.|hand"] = function(self, prompt, method)
	local cards =  sgs.QList2Table(self.player:getHandcards())
	self:sortByUseValue(cards)
	for _, card in ipairs(cards) do
		if card:getTypeId() == sgs.Card_TypeTrick and not card:isKindOf("Nullification") then
			local dummy_use = { isDummy = true, to = sgs.SPlayerList() }
			self:useTrickCard(card, dummy_use)
			if dummy_use.card then
				if dummy_use.to:isEmpty() then
					if card:isKindOf("IronChain") then
						return "."
					end
					return dummy_use.card:toString()
				else
					local target_objectname = {}
					for _, p in sgs.qlist(dummy_use.to) do
						table.insert(target_objectname, p:objectName())
					end
					return dummy_use.card:toString() .. "->" .. table.concat(target_objectname, "+")
				end
			end
		elseif card:getTypeId() == sgs.Card_TypeBasic and not card:isKindOf("Jink") then
			local dummy_use = { isDummy = true, to = sgs.SPlayerList() }
			self:useBasicCard(card, dummy_use)
			if dummy_use.card then
				if dummy_use.to:isEmpty() then
					return dummy_use.card:toString()
				else
					local target_objectname = {}
					for _, p in sgs.qlist(dummy_use.to) do
						table.insert(target_objectname, p:objectName())
					end
					return dummy_use.card:toString() .. "->" .. table.concat(target_objectname, "+")
				end
			end
		elseif card:getTypeId() == sgs.Card_TypeEquip then
			local dummy_use = { isDummy = true }
			self:useEquipCard(card, dummy_use)
			if dummy_use.card then
				return dummy_use.card:toString()
			end
		end
	end
	return "."
end

--shifeng
sgs.ai_skill_invoke.shifeng = function(self, data)
	if not self:willShowForDefence() and not self:willShowForAttack() then
		return false
	end
	return true
end

sgs.ai_skill_use["@@shifeng"] = function(self, prompt)
	local targets = {}
	local n = 0
	for _,p in ipairs(self.friends) do
	  if self.player:inMyAttackRange(p) then
	     n = n+1
	  end
	end
	
	if n > 1 then
	 for _,p in ipairs(self.friends) do
	   if #targets <n and self.player:inMyAttackRange(p) then table.insert(targets, p:objectName()) end
	 end
	end
	
	if (n == 1) then
	   local m = 0
	   local target
	   for _,p in ipairs(self.friends) do
	     if p:getHandcardNum()>m and p:isWounded() then 
		   target = p
		   m = p:getHandcardNum()
		 end
	   end
	   if #targets <1 and target then table.insert(targets, target:objectName()) end
	end
	
	if type(targets) == "table" and #targets > 0 then
		return ("@ShifengCard=.&shifeng->" .. table.concat(targets, "+"))
	end
	return "."
end

sgs.ai_skill_choice.shifeng = function(self, choices, data)
   if self.player:containsTrick("indulgence") then
     return "shifeng_otherdraw"
   else
     local n = math.random(1,4)
	 if n==4 then
	   return "shifeng_otherdraw"
	 end
   end
   return "shifeng_selfdraw"
end

zhiyan_skill={}
zhiyan_skill.name="zhiyan"
table.insert(sgs.ai_skills,zhiyan_skill)
zhiyan_skill.getTurnUseCard=function(self,inclusive)
	local source = self.player
	if self.player:isKongcheng() then return end
	if self.player:hasFlag("zhiyan_used") then return end
	local can_man = false
	for _,enemy in ipairs(self.enemies) do
		if not self.player:hasFlag(enemy:objectName().."zhiyan") and enemy:getHandcardNum() > 0 then
			can_man = true
		end
	end
	for _,friend in ipairs(self.friends_noself) do
		for _,c in sgs.qlist(friend:getJudgingArea()) do
		   if (not c:isKindOf("Key") or friend:isWounded()) and not self.player:hasFlag(friend:objectName().."zhiyan") and friend:getHandcardNum()>0 then
			 can_man = true
		   end
		end
	end
	if not can_man then return end
	local cards=sgs.QList2Table(self.player:getHandcards())
	local OK = false
	for _,card in ipairs(cards) do
		if card:getNumber() > 6 then
			OK =true
		end
	end
	if OK then
		return sgs.Card_Parse("@ZhiyanCard=.&zhiyan")
	end
end

sgs.ai_skill_use_func.ZhiyanCard = function(card,use,self)
	local target
	local source = self.player
	local m = 998
	
	for _,friend in ipairs(self.friends_noself) do
	   for _,c in sgs.qlist(friend:getJudgingArea()) do
		   if c:isKindOf("Key") and not friend:isKongcheng() and friend:isWounded() and not self.player:hasFlag(friend:objectName().."zhiyan") then
			 target = friend
		   end
		end
	end
	for _,enemy in ipairs(self.enemies) do
	    if enemy:getHandcardNum()<m and enemy:getHandcardNum()>0 and not self.player:hasFlag(enemy:objectName().."zhiyan") then
		  target = enemy
		  m = enemy:getHandcardNum()
		end
	end
	for _,friend in ipairs(self.friends_noself) do
	   for _,c in sgs.qlist(friend:getJudgingArea()) do
		   if not c:isKindOf("Key") and not friend:isKongcheng() and not self.player:hasFlag(friend:objectName().."zhiyan") then
			 target = friend
		   end
		end
	end
	if target then
		use.card = sgs.Card_Parse("@ZhiyanCard=.&zhiyan")
		if use.to then use.to:append(target) end
		return
	end
end

sgs.ai_skill_invoke.rennai = function(self, data)
    local damage = data:toDamage()
	if damage and damage.damage>1 then
	  return true
	end
	return self:willShowForAttack() or self:willShowForDefence()
end

sgs.ai_skill_choice.rennai = function(self, choices, data)
	choices = choices:split("+")
	local tp = 1
	for _,choice in ipairs(choices) do
		if choice == "rennai_hp" then
			tp = 0
			break
		end
		if choice == "rennai_gain" then
			tp = 2
			break
		end
	end

	-- analysis
	local hp_table = {}
	local hand_table = {}
	for _,p in sgs.qlist(self.room:getAlivePlayers()) do
		if hp_table[p:getHp()] ~= nil then
			self.room:writeToConsole("old")
			if self:isFriend(p) then
				if p:getMark("@Frozen_Eu") > 0 then
				else
					hp_table[p:getHp()] = hp_table[p:getHp()] - 2
				end
			else
				if p:getMark("@Frozen_Eu") > 0 then
					hp_table[p:getHp()] = hp_table[p:getHp()] + 1
				else
					hp_table[p:getHp()] = hp_table[p:getHp()] + 3
				end
			end
		else
			self.room:writeToConsole("new")
			if self:isFriend(p) then
				if p:getMark("@Frozen_Eu") > 0 then
					hp_table[p:getHp()] = 0
				else
					hp_table[p:getHp()] = - 2
				end
			else
				if p:getMark("@Frozen_Eu") > 0 then
					hp_table[p:getHp()] = 1
				else
					hp_table[p:getHp()] = 3
				end
			end
		end
		if hand_table[p:getHandcardNum()] ~= nil then
			if self:isFriend(p) then
				if p:getMark("@Frozen_Eu") > 0 then
				else
					hand_table[p:getHandcardNum()] = hand_table[p:getHandcardNum()] - 2
				end
			else
				if p:getMark("@Frozen_Eu") > 0 then
					hand_table[p:getHandcardNum()] = hand_table[p:getHandcardNum()] + 1
				else
					hand_table[p:getHandcardNum()] = hand_table[p:getHandcardNum()] + 3
				end
			end
		else
			if self:isFriend(p) then
				if p:getMark("@Frozen_Eu") > 0 then
					hand_table[p:getHandcardNum()] = 0
				else
					hand_table[p:getHandcardNum()] = - 2
				end
			else
				if p:getMark("@Frozen_Eu") > 0 then
					hand_table[p:getHandcardNum()] = 1
				else
					hand_table[p:getHandcardNum()] = 3
				end
			end
		end
	end

	local maxValue = -100000
	local hp_or_hand
	local isHp = false
	for k,v in ipairs(hp_table) do
		self.room:writeToConsole(k)
		self.room:writeToConsole(v)
		if v > maxValue then
			maxValue = v
			hp_or_hand = k
			isHp = true
		end
	end

	for k,v in ipairs(hand_table) do
		if v > maxValue then
			maxValue = v
			hp_or_hand = k
			isHp = false
		end
	end

	self.room:writeToConsole(maxValue)
	self.room:writeToConsole(hp_or_hand)

	if tp == 0 then
		-- rennai_hp  rennai_lose
		if isHp then
			return "rennai_hp"
		else
			return "rennai_lose"
		end
	elseif tp == 1 then
		return hp_or_hand
	else
		return "rennai_gain"
	end
end

sgs.ai_skill_invoke.zhanfang = function(self, data)
	-- 绽放吧！
	local use = data:toCardUse()
	if use and use.card and self:isEnemy(use.to:first()) then return true end
	return false
end

sgs.ai_skill_choice.zhanfang = function(self, choices, data)
	if self.player:getMark("@Frozen_Eu") > 1 and math.random(1,2)==2 then
		return "cancel"
	else
		return "zhanfang_discard"
	end
end

sgs.ai_skill_invoke.zuozhan = function(self, data)
   return self:willShowForAttack() or self:willShowForDefence()
end

sgs.ai_skill_choice["zuozhan1"] = function(self, choices, data)
	local room = self.room
	local p = room:getCurrent()
	if self:isEnemy(p) then
		return "1_Zuozhan"
	else
		if p:getHandcardNum() <= p:getHp() then return "4_Zuozhan" else return "2_Zuozhan" end
	end
	return "1_Zuozhan"
end

sgs.ai_skill_choice["zuozhan2"] = function(self, choices, data)
	local room = self.room
	local p = room:getCurrent()
	if self:isEnemy(p) then
		if p:getHandcardNum() <= 1 and p:getHp() <= 2 then
			return "3_Zuozhan"
		else
			return "2_Zuozhan"
		end
	else
		if p:getHandcardNum() <= p:getHp() then return "2_Zuozhan" else return "3_Zuozhan" end
	end
	return "2_Zuozhan"
end

sgs.ai_skill_choice["zuozhan3"] = function(self, choices, data)
	local room = self.room
	local p = room:getCurrent()
	if self:isEnemy(p) then
		if p:getHandcardNum() <= 1 and p:getHp() <= 2 then
			return "2_Zuozhan"
		else
			return "4_Zuozhan"
		end
	else
		if p:getHandcardNum() <= p:getHp() then return "3_Zuozhan" else return "4_Zuozhan" end
	end
	return "3_Zuozhan"
end

sgs.ai_skill_choice["zuozhan4"] = function(self, choices, data)
	local room = self.room
	local p = room:getCurrent()
	if self:isEnemy(p) then
		if p:getHandcardNum() <= 1 and p:getHp() <= 2 then
			return "4_Zuozhan"
		else
			return "3_Zuozhan"
		end
	else
		return "1_Zuozhan"
	end
	return "4_Zuozhan"
end

sgs.ai_skill_invoke.nishen = function(self, data)
	local dying = data:toDying()
	if not self:isEnemy(dying.who) then return true end
	for _,p in ipairs(self.friends) do
		if self:isWeak(p) then return false end
	end
	return true
end

sgs.ai_skill_choice.nishen = function(self, choices, data)
	choices = choices:split("+")
	local on_join = false
	for _,choice in ipairs(choices) do
		if choice == "nishen_accept" then
			on_join = true
		end
	end
	if on_join then
		local yuri = self.room:findPlayerBySkillName("nishen")
		if not yuri then return "cancel" end
		if self.player:getRole() == "careerist" then return "cancel" end
		if not self:isEnemy(yuri) then return "nishen_accept" end
		return "cancel"
	else
		if self.player:getHandcardNum() < self.player:getHp() * 2 then return "nishen_draw" end
		return "nishen_recover"
	end
end

sgs.ai_skill_invoke.xingbao = function(self, data)
    local damage = data:toDamage()
	local card = damage.card
    local hecards = self.player:getCards("he")
	for _, c in sgs.qlist(hecards) do
	  if c:isRed() and card:isRed()  then
	    return true
	  end
	end
	for _, c in sgs.qlist(hecards) do
	  if c:isBlack() and card:isBlack() then
	    return true 
	  end
	end
   return false
end

sgs.ai_skill_use["@@xingbao"] = function(self, prompt)
	local card
    local hecards = self.player:getCards("he")
	for _, c in sgs.qlist(hecards) do
	  if c:isRed() and self.player:hasFlag("xingbao_red") and not c:isKindOf("Slash") then
	    card = c 
	  end
	end
	for _, c in sgs.qlist(hecards) do
	  if c:isBlack() and self.player:hasFlag("xingbao_black") and not c:isKindOf("Slash") then
	    card = c 
	  end
	end
	if not card then
	
	  for _, c in sgs.qlist(hecards) do
	  if c:isRed() and self.player:hasFlag("xingbao_red") then
	    card = c 
	  end
	end
	for _, c in sgs.qlist(hecards) do
	  if c:isBlack() and self.player:hasFlag("xingbao_black") then
	    card = c 
	  end
	end
	
    end 
	if card then
		return ("@XingbaoCard="..card:getEffectiveId().."&->")
	end
	return "."
end

shiso_skill={}
shiso_skill.name="shiso"
table.insert(sgs.ai_skills,shiso_skill)
shiso_skill.getTurnUseCard=function(self,inclusive)
	local source = self.player
	if source:hasUsed("ShisoCard") then return end
	return sgs.Card_Parse("@ShisoCard=.&shiso")
end
sgs.ai_skill_use_func.ShisoCard = function(card,use,self)
	local target
	local card
	local player = self.player
	for _,friend in ipairs(self.friends) do
		if friend:hasSkill("shizu") then
			target = friend
		end
	end
	local cards=sgs.QList2Table(player:getHandcards())
    self:sortByUseValue(cards, true)
    for _,c in ipairs(cards) do
	  if (c:getSuitString()=="heart" or c:getSuitString()=="spade") and c:getNumber()>10 then
	     card = c
	  end
	end
	if not card then
	    local cards = sgs.QList2Table(player:getEquips())
		self:sortByUseValue(cards, true)
		 for _,c in ipairs(cards) do
	  if (c:getSuitString()=="heart" or c:getSuitString()=="spade") and c:getNumber()>10 then
	     card = c
	  end
    end
	end
	
	if not card then
	
	for _,c in ipairs(cards) do
	  if (c:getSuitString()=="heart" or c:getSuitString()=="spade") then
	     card = c
	  end
	end
	if not card then
	    local cards = sgs.QList2Table(player:getEquips())
		self:sortByUseValue(cards, true)
		 for _,c in ipairs(cards) do
	  if c:getSuitString()=="heart" or c:getSuitString()=="spade" then
	     card = c
	  end
    end
	end
	
	end
	
	if target and card then
		use.card = sgs.Card_Parse("@ShisoCard="..card:getEffectiveId().."&shiso")
		if use.to then use.to:append(target) end
		return
	end
end

sgs.ai_skill_invoke.zahyo = function(self, data)
  return true
end

sgs.ai_skill_invoke.quzhuaddtarget = function(self, data)
	local use = data:toCardUse()
	if use and use.card and self:isEnemy(use.to:first()) then return true end
	return false
end

sgs.ai_skill_invoke.quzhudamage= function(self, data)
	local player= data:toPlayer()
	if player and self:isEnemy(player) then return true end
	return false
end

sgs.ai_skill_invoke.jinji = function(self, data)
  local damage = data:toDamage()
  if damage.from and damage.from:getKingdom()==self.player:getKingdom() then return false end
  return true
end

sgs.ai_skill_playerchosen.jinji = function(self, targets, max_num, min_num)
  for _,p in sgs.qlist(self.room:getAlivePlayers()) do
     if self:isEnemy(p) and p:getMark("jinji_used")==0 and not self.player:inMyAttackRange(p) and p:getMark("@quzhu")==0 then
        return p
     end
	 if self:isEnemy(p) and p:getMark("jinji_used")==0 and p:getMark("@quzhu")==0 then
        return p
     end
	 if self:isEnemy(p) and p:getMark("jinji_used")==0 and  p:getMark("@quzhu")==1 then
        return p
     end
	 if self:isEnemy(p) and p:getMark("jinji_used")==0 and  p:getMark("@quzhu")==2 then
        return p
     end
  end
end

sgs.ai_skill_choice.docommand_shizu = function(self, choices, data)
   local n = data:toInt()
   if n==1 or n==2 then
     return "yes"
   else
     return "no"
   end	 
end

--朝潮
fanqian_skill={}
fanqian_skill.name="fanqian"
table.insert(sgs.ai_skills,fanqian_skill)
fanqian_skill.getTurnUseCard=function(self,inclusive)
	if self:getCardsNum("Peach") +  self:getCardsNum("Jink") + self:getCardsNum("Analeptic") +  self:getCardsNum("Nullification") >= self.player:getHandcardNum() then return end
	return sgs.Card_Parse("@FanqianCard=.&fanqian")
end

sgs.ai_skill_use_func.FanqianCard = function(card,use,self)
	local card

	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByUsePriority(cards)


	--check equips first
	local equips = {}
	for _, card in sgs.list(self.player:getHandcards()) do
		if card:isKindOf("Armor") or card:isKindOf("Weapon") then
			if not self:getSameEquip(card) then
			elseif card:isKindOf("GudingBlade") and self:getCardsNum("Slash") > 0 then
				local HeavyDamage
				local slash = self:getCard("Slash")
				for _, enemy in ipairs(self.enemies) do
					if self.player:canSlash(enemy, slash, true) and not self:slashProhibit(slash, enemy) and
						self:slashIsEffective(slash, enemy) and not self.player:hasSkill("jueqing") and enemy:isKongcheng() then
							HeavyDamage = true
							break
					end
				end
				if not HeavyDamage then table.insert(equips, card) end
			else
				table.insert(equips, card)
			end
		elseif card:getTypeId() == sgs.Card_TypeEquip then
			table.insert(equips, card)
		end
	end

	if #equips > 0 then

		local select_equip, target
		for _, friend in ipairs(self.friends) do
			for _, equip in ipairs(equips) do
				if not self:getSameEquip(equip, friend) and self:hasSkills(sgs.need_equip_skill .. "|" .. sgs.lose_equip_skill, friend) then
					target = friend
					select_equip = equip
					break
				end
			end
			if target then break end
			for _, equip in ipairs(equips) do
				if not self:getSameEquip(equip, friend) then
					target = friend
					select_equip = equip
					break
				end
			end
			if target then break end
		end


		if target then
			use.card = sgs.Card_Parse("@FanqianCard="..select_equip:getEffectiveId().."&fanqian")
			self.room:setTag("fanqian_target",sgs.QVariant(target:getSeat()))
			return
		end
	end

    










	for _, c in ipairs(cards) do
		if not c:isKindOf("Collateral") then
			if c:isKindOf("Slash") or c:isKindOf("SingleTargetTrick") or c:isKindOf("Lightning") or c:isKindOf("AOE") then
				card = c
				break
			end
		end
	end

	if card then

		local target

		for _,p in sgs.list(self.room:getAlivePlayers()) do
			if p:getMark("@Buyu") > 0 then target = p end
		end
		if not target then target = self.enemies[1] end

		if target then
			use.card = sgs.Card_Parse("@FanqianCard="..card:getEffectiveId().."&fanqian")
			self.room:setTag("fanqian_target",sgs.QVariant(target:getSeat()))
			return
		end
	else
		--peach
		for _, c in ipairs(cards) do
			if c:isKindOf("Peach") or c:isKindOf("GodSalvation") then
				card = c
				break
			end
		end

		if card then
			local target
			local minHp = 100
			for _,friend in ipairs(self.friends) do
				local hp = friend:getHp()
				if friend:getHp()==friend:getMaxHp() then
					hp = 1000
				end
				if self:hasSkills(sgs.masochism_skill, friend) then
					hp = hp - 1
				end
				if friend:isLord() then
					hp = hp - 1
				end
				if hp < minHp then
					minHp = hp
					target = friend
				end
			end
			for _,friend in ipairs(self.friends) do
				if friend:objectName() == "SE_Kirito" and friend:getHp() == 1 then
					target = friend
				end
			end
			if target then
				use.card = sgs.Card_Parse("@FanqianCard="..card:getEffectiveId().."&fanqian")
				self.room:setTag("fanqian_target",sgs.QVariant(target:getSeat()))
				return
			end


		else
			for _, c in ipairs(cards) do
				if c:isKindOf("ExNihilo") or c:isKindOf("AmazingGrace") then
					card = c
					break
				end
			end

			if card then
				target = self:findPlayerToDraw(true, 2)
				if target then
					use.card = sgs.Card_Parse("@FanqianCard="..card:getEffectiveId().."&fanqian")
					self.room:setTag("fanqian_target",sgs.QVariant(("%d"):format(target:getSeat())))
					return
				end
			end
		end
	end
end
sgs.ai_skill_choice["fanqian"] = function(self, choices, data)
	return self.room:getTag("fanqian_target"):toString()
end


sgs.ai_use_value["FanqianCard"] = 8
sgs.ai_use_priority["FanqianCard"]  = 10
sgs.ai_card_intention["FanqianCard"] = 0

sgs.ai_skill_invoke.buyu = function(self, data)
	if #self.enemies == 0 then return false end
	local num = 0
	local other = 0
	for _, c in sgs.list(self.player:getHandcards()) do
		if (c:isKindOf("Slash") or c:isKindOf("SingleTargetTrick") or c:isKindOf("Lightning") or c:isKindOf("AOE")) and not c:isKindOf("Collateral") then
			num = num + 1
		elseif not c:isKindOf("Analeptic") and not c:isKindOf("Jink") then
			other = other + 1
		end
	end

	if num >= other then return true end
	return false
end

sgs.ai_skill_playerchosen.buyu = function(self, targets)
	return self:getPriorTarget()
end

--蓝羽浅葱
sgs.ai_skill_invoke.guanli = function(self, data)
    if not self:willShowForAttack() and not self:willShowForDefence() then return false end
	local PlayerNow = data:toPlayer()
	if self:isEnemy(PlayerNow) then
		sgs.guanli_reason = "enemy_discard"
		if PlayerNow:getHandcardNum() - PlayerNow:getMaxCards() > 1 then
			if self.player:getHandcardNum() > 3 then return true end
		elseif PlayerNow:getHandcardNum() - PlayerNow:getMaxCards() > 2 then
			if self.player:getHandcardNum() > 2 then return true end
		elseif PlayerNow:getHandcardNum() - PlayerNow:getMaxCards() > 4 then
			if self.player:getHandcardNum() > 0 then return true end
			if self.player:getEquips():length() > 0 then return true end
		end
	elseif self:isFriend(PlayerNow) then
		if self.player:getHandcardNum() > 0 then
			if self:hasSkills("qixin|shunshan|kanhu|shengjian|jianyu|huanyuan|zhanjing|gonglue|boxue|",PlayerNow) then
				sgs.guanli_reason = "friend_play"
				return true
			end
			if self:hasSkills("guanli|weigong|zhufu|luowang",PlayerNow) then
				sgs.guanli_reason = "friend_draw"
				return true
			end
		end
		if self.player:getHandcardNum() > 3 then
			sgs.guanli_reason = "friend_draw"
			return true
		end
	end
	return false
end

sgs.ai_skill_choice.guanli = function(self, choices, data)
	if sgs.guanli_reason == "friend_draw" then return "Gl_draw"
	elseif sgs.guanli_reason == "friend_play" then return "Gl_play"
	elseif sgs.guanli_reason == "enemy_discard" then return "Gl_discard"
	end
end

poyi_skill={}
poyi_skill.name="poyi"
table.insert(sgs.ai_skills,poyi_skill)
poyi_skill.getTurnUseCard=function(self,inclusive)
	if self.player:hasUsed("PoyiCard")  then return end
	if #self.enemies < 1 or self.room:getAlivePlayers():length()<=2 then return end
	return sgs.Card_Parse("@PoyiCard=.&poyi")
end

sgs.ai_skill_use_func.PoyiCard = function(card,use,self)
	local target
	local slashtarget
	local source = self.player
	for _,enemy in ipairs(self.enemies) do
		if source:getHp()<=enemy:getHp() then
			target = enemy
		end
	end
	for _,enemy in ipairs(self.enemies) do
		if source:getHp()<=enemy:getHp() and enemy:getHp() == 2 then
			target = enemy
		end
	end
	for _,enemy in ipairs(self.enemies) do
		if enemy:getHp() == 1 and source:getHp()<=enemy:getHp() then
			target = enemy
		end
	end
	for _,friend in ipairs(self.friends_noself) do
		if source:getHp()<= friend:getHp() and  not self.player:isFriendWith(friend) then
			target = friend
		end
	end
	if target then
	    for _,enemy in ipairs(self.enemies) do
			if target:inMyAttackRange(enemy) and target:objectName()~=enemy:objectName() then
				slashtarget = enemy
			end
		end
	end
	if target and slashtarget then
		use.card = sgs.Card_Parse("@PoyiCard=.&poyi")
		if use.to then use.to:append(target) end
		if use.to then use.to:append(slashtarget) end
		return
	end
end

sgs.ai_use_value.PoyiCard = 5
sgs.ai_use_priority.PoyiCard = 2
sgs.ai_card_intention.PoyiCard = 0

chicheng_skill={}
chicheng_skill.name="chicheng"
table.insert(sgs.ai_skills,chicheng_skill)
chicheng_skill.getTurnUseCard=function(self,inclusive)
	local source = self.player
	if not (source:getHandcardNum() >= 2 or source:getHandcardNum() > source:getHp()) then return end
	if not self:willShowForAttack() and not self:willShowForDefence() then return end
	if source:hasUsed("ChichengCard") then return end
	return sgs.Card_Parse("@ChichengCard=.&chicheng")
end

sgs.ai_skill_use_func.ChichengCard = function(card,use,self)
	local cards=sgs.QList2Table(self.player:getHandcards())
	local cards2=sgs.QList2Table(self.player:getEquips())
	local needed = {}
	local num = 2
	if not self.player:isWounded() and self.player:getSiblings():length()<=1 then num = 1 end
	if self.player:getHandcardNum() - self.player:getHp() > 2 then num = self.player:getHandcardNum() - self.player:getHp() end
	for _,acard in ipairs(cards) do
		if #needed < num then
			table.insert(needed, acard:getEffectiveId())
		end
	end
	for _,acard in ipairs(cards2) do
		if #needed < num then
			table.insert(needed, acard:getEffectiveId())
		end
	end
	if needed then
		use.card = sgs.Card_Parse("@ChichengCard="..table.concat(needed,"+").."&chicheng")
		return
	end
end

sgs.ai_use_value.ChichengCard = 2
sgs.ai_use_priority.ChichengCard  = 1.2

sgs.ai_skill_invoke.zhikong = function(self, data)
	local pname = data:toPlayer():objectName()
	local p
	for _,r in sgs.qlist(self.room:getAlivePlayers()) do
		if r:objectName() == pname then p = r end
	end
	if not p then return false end
	if self:isFriend(p) and self.player:getPile("akagi_lv"):length() > 1 and not p:hasShownSkills("pasheng|wushi") then return true end
	if self:isFriend(p) and p:isFriendWith(self.player) then return true end
	if p:objectName() == self.player:objectName() then return true end
	return false
end

sgs.ai_skill_invoke.lianchui = function(self, data)
  return self:willShowForAttack() or self:willShowForDefence()
end

sgs.ai_skill_playerchosen.lianchui= function(self, targets)
	for _, target in sgs.qlist(targets) do
		if self:isEnemy(target) then return target end
	end
	return nil
end

sgs.ai_skill_invoke.xianshu = function(self, data)
  return self:willShowForDefence()
end

sgs.ai_skill_playerchosen.xianshu= function(self, targets)
	for _, target in sgs.qlist(targets) do
		if self:isFriend(target) then return target end
	end
	return nil
end

sgs.ai_skill_invoke.huanbing = function(self, data)
  if not self:willShowForAttack() and not self:willShowForDefence() then return false end
  local damage = data:toDamage()
  if damage and damage.to then return self:isEnemy(damage.to) end
  return #self.enemies>0
end

sgs.ai_skill_playerchosen.huanbing= function(self, targets)
	return self:getPriorTarget()
end

sgs.ai_skill_invoke.trial = function(self, data)
   if not self:willShowForAttack() and not self:willShowForDefence() then return false end
   local use = data:toCardUse()
   if self:isEnemy(use.from) and self:isFriend(use.to:at(0)) then return true end
   if self:isEnemy(use.from) and use.from:isFriendWith(use.to:at(0)) then return true end
end

local yaozhan_skill = {}
yaozhan_skill.name = "yaozhan"
table.insert(sgs.ai_skills, yaozhan_skill)
yaozhan_skill.getTurnUseCard = function(self)
	if not self:willShowForAttack() then
		return
	end
	if self.player:hasUsed("YaozhanCard") then return end
	return sgs.Card_Parse("@YaozhanCard=.&yaozhan")
end

sgs.ai_skill_use_func.YaozhanCard = function(YZCard, use, self)
	local targets = {}
	for _, enemy in ipairs(self.enemies) do
		table.insert(targets, enemy)
	end

	if #targets == 0 then return end

	sgs.ai_use_priority.YaozhanCard = 8
	if not self.player:getArmor() and not self.player:isKongcheng() then
		for _, card in sgs.qlist(self.player:getCards("h")) do
			if card:isKindOf("Armor") and self:evaluateArmor(card) > 3 then
				sgs.ai_use_priority.YaozhanCard = 5.9
				break
			end
		end
	end

	if use.to then
		self:sort(targets, "defenseSlash")
		use.to:append(targets[1])
	end
	use.card = YZCard
end


local function getSlashNum(player)
	local num = 0
	for _,card in sgs.qlist(player:getHandcards()) do
		if card:isKindOf("Slash") then
			num = num + 1
		end
	end
	return num
end

local poshi_skill={}
poshi_skill.name="poshi"
table.insert(sgs.ai_skills,poshi_skill)
poshi_skill.getTurnUseCard=function(self,inclusive)
	if self.player:hasUsed("PoshiCard") then return end
	if #self.enemies < 1 then return end
	if getSlashNum(self.player) < 2 then return end
	if self.player:getHp() < 2 then return end
	if getSlashNum(self.player) < 3 and self.player:getHp() < 3 then return end
	return sgs.Card_Parse("@PoshiCard=.&poshi")
end

sgs.ai_skill_use_func.PoshiCard = function(card,use,self)
	use.card = sgs.Card_Parse("@PoshiCard=.&poshi")
	return
end

sgs.ai_use_value.PoshiCard = 7
sgs.ai_use_priority.PoshiCard = 9

sgs.ai_skill_invoke.liansuo = true

sgs.ai_skill_playerchosen.liansuo = function(self, targets)
     local target
     for _, enemy in ipairs(self.enemies) do
		if not enemy:isChained() then
			target = enemy
		end
	 end
	 if target then return target end
end

sgs.ai_skill_invoke.yinguo = function(self, data)
	for _,p in sgs.qlist(self.room:getAlivePlayers()) do
		if p:hasShownOneGeneral() and self:isFriend(p) then
			return true
		end
	end
	return false
end

sgs.ai_skill_playerchosen.yinguo = function(self, targets)
	local min_card_num = 100
	local target
	for _,p in sgs.qlist(targets) do
		if p:hasShownOneGeneral() and self:isFriend(p) then
			if p:getHandcardNum() < min_card_num then
				target = p
				min_card_num = p:getHandcardNum()
			end
		end
	end
	if target then return target end
end

local jiuzhu_skill={}
jiuzhu_skill.name="jiuzhu"
table.insert(sgs.ai_skills,jiuzhu_skill)
jiuzhu_skill.getTurnUseCard=function(self,inclusive)
	if self.player:hasUsed("JiuzhuCard") then return end
	if #self.friends_noself < 1 then return end
	return sgs.Card_Parse("@JiuzhuCard=.&jiuzhu")
end

sgs.ai_skill_use_func.JiuzhuCard = function(card,use,self)
    local target1
	local target2
	for _,p in ipairs(self.friends_noself) do
	   if p:getHp()<= self.player:getHp() and p:isWounded() then
	     target1 = p
	   end
	end
	for _,p in ipairs(self.friends_noself) do
	   if target1 and p:getHp()<= self.player:getHp() and p:isWounded() and p:objectName()~=target1:objectName() then
	     target2 = p
	   end
	end
	if (self.player:getHp()==1 and (not self.player:hasSkill("shexin") or self.player:getMark("@shexin")==0) and self.player:getHandcardNum()>3) then
	   return
	end
	if target1 then 
	   use.card = sgs.Card_Parse("@JiuzhuCard=.&jiuzhu")
	   if use.to then use.to:append(target1) end
	end
	if use.to and target2 then 
	   use.to:append(target2)
	end
	return
end

sgs.ai_skill_invoke.shexin= function(self, data)
   return true
end

sgs.ai_skill_use["@@shexin"] = function(self, prompt)
	local targets = {}
	local dest
	local card
	for _,p in ipairs(self.friends) do
	  dest = p
	end
	local cards = sgs.QList2Table(self.player:getHandcards())
	local equips=sgs.QList2Table(self.player:getEquips())
	self:sortByUseValue(cards,true)
	self:sortByUseValue(equips,true)
	for _,acard in ipairs(cards) do
		if  acard:isRed() then
			card =acard
		end
	end
	for _,acard in ipairs(equips) do
		if acard:isRed()  then
			card =acard
		end
	end
	if dest and card then
	  return ("@ShexinCard="..card:getEffectiveId().."&->" .. dest:objectName())
	else
	  return "."
	end
end

sgs.ai_skill_invoke.xintiao= function(self, data)
   return true
end

sgs.ai_skill_playerchosen.xintiao = function(self, targets, max_num, min_num)
	for _, target in sgs.qlist(targets) do
		if self:isFriend(target) then return target end
	end
	return nil
end

sgs.ai_skill_invoke.suipian= function(self, data)
   return true
end

local lunhui_skill = {}
lunhui_skill.name = "lunhui"
table.insert(sgs.ai_skills, lunhui_skill)
lunhui_skill.getTurnUseCard = function(self,room,player,data)
	if self.player:hasUsed("ViewAsSkill_lunhuiCard") or self.player:getPile("Fragments"):length()==0 or self.player:getKingdom()~="magic" then return end
	local id
	local idn
	local ids = self.player:getPile("Fragments")
	for _,i in sgs.qlist(ids) do
	    id = tostring(i)
		idn = i
		break
    end
	if not id then return end
	local pattern = self.player:property("lunhui_card"):toString()
    if pattern == "" then
	  return
	end
	local card = sgs.Sanguosha:getCard(idn)
	local str = sgs.Card_Parse(pattern..":lunhui["..card:getSuitString()..":"..card:getNumberString().."]="..id.."&lunhui")
    local can
	if self:getUseValue(str) > 0.2 then
       can = true
     end
	if can and str and id then
		return str
	end
end

sgs.ai_skill_invoke.yandan= function(self, data)
   return true
end

sgs.ai_skill_invoke.lunpo = function(self, data)
	local use = data:toCardUse()
	if use.from and self:isEnemy(use.from) then
		if use.card:isKindOf("SingleTargetTrick") and use.to:length() > 0 and self:isFriend(use.to:at(0)) then
			if use.card:isKindOf("Snatch") or use.card:isKindOf("Duel") then return true end
			--if use.card:isKindOf("Dismantlement") and use.to:at(0):getEquips():length() > 0 then return true end
			if use.card:isKindOf("DelayedTrick") and not use.card:isKindOf("KeyTrick") then return true end
		end
		if use.card:isKindOf("Slash") and self:isWeak(use.to:at(0)) and self:isFriend(use.to:at(0)) then return true end
		if use.card:isKindOf("Jink") or use.card:isKindOf("Peach") then return true end
		if use.card:isKindOf("AOE") or use.card:isKindOf("GlobalEffect") then
			for _,p in ipairs(self.friends) do
				if self:isWeak(p) then
					return true
				end
			end
		end
	elseif not use.from then	
       if #self.enemies == 0 then return false end
	local min = 100
	for _,p in sgs.list(self.room:getAlivePlayers()) do
		if p:getHp() < min then min = p:getHp() end
	end
	local num = self.player:getPile("yandan"):length()
	if min <= 2 then
		local toFight = self:getPriorTarget()
		if toFight:getHp() <= 1 then return true end
		if toFight:hasSkills(sgs.masochism_skill) then return true end
	end
	return false
    end
	return false
end

sgs.ai_view_as.zizheng = function(card, player, card_place)
    local list = player:getPile("yandan")
	if list:length()<2 then return end
	local card1
	local card2
	for _,i in sgs.qlist(list) do
	   for _,j in sgs.qlist(list) do
	      local c1 = sgs.Sanguosha:getCard(i)
	      local c2 = sgs.Sanguosha:getCard(j)
		  if i ~= j and (c1:getNumber()==c2:getNumber() or c1:getSuit()==c2:getSuit()) then
		      card1 = c1
			  card2 = c2
			  break
		  end
	   end
	end
	if not card1 or not card2 then
	   card1 = sgs.Sanguosha:getCard(list:at(0))
	   card2 = sgs.Sanguosha:getCard(list:at(1))
	end
	
	local id1 = card1:getEffectiveId()
	local id2 = card2:getEffectiveId()
	local str = ("heg_nullification:%s[%s:%s]=%d+%d&zizheng"):format("zizheng", "to_be_decided", "-", id1, id2)
	return str
end

sgs.ai_skill_choice.zizheng= function(self, choices, data)
	if self.player:getPile("yandan"):length()<=2 then
	  return "zizheng_transform"
	end
end

huanshi_skill={}
huanshi_skill.name="huanshi"
table.insert(sgs.ai_skills,huanshi_skill)
huanshi_skill.getTurnUseCard=function(self,inclusive)
	local source = self.player
	if self.player:isKongcheng() then return end
	if self.player:hasUsed("HuanshiCard") then return end
	if #self.enemies == 0 then return end
	return sgs.Card_Parse("@HuanshiCard=.&huanshi")
end

sgs.ai_skill_use_func.HuanshiCard = function(card,use,self)
	local target
	local source = self.player
	local m = 998
	
	for _,enemy in ipairs(self.enemies) do
	    if enemy:getHandcardNum()<m and enemy:getHandcardNum()>0  then
		  target = enemy
		  m = enemy:getHandcardNum()
		end
	end

	if target then
		use.card = sgs.Card_Parse("@HuanshiCard=.&huanshi")
		if use.to then use.to:append(target) end
		return
	end
end

sgs.ai_skill_invoke.kuangzao= function(self, data)
   local damage = data:toDamage()
   return self:isEnemy(damage.to)
end

sgs.ai_skill_invoke.haoqi= function(self, data)
   local use = data:toCardUse()
   return self:isEnemy(use.from) or use.card:isRed()
end

local function add_different_kingdoms2(self, target, targets)
   for _,p in ipairs(targets) do
	 if target:isFriendWith(p) then return false end
   end
   return true
end

shouji_skill={}
shouji_skill.name="shouji"
table.insert(sgs.ai_skills,shouji_skill)
shouji_skill.getTurnUseCard=function(self,inclusive)
	if self.player:hasUsed("ShoujiCard") then return end
	return sgs.Card_Parse("@ShoujiCard=.&shouji")
end

sgs.ai_skill_use_func.ShoujiCard = function(card,use,self)
	local needed = {}
	local hp = math.max(self.player:getHp(), 1)
	local cards = sgs.QList2Table(self.player:getHandcards())
	local equips = sgs.QList2Table(self.player:getEquips())
	self:sortByUseValue(cards)
	self:sortByUseValue(equips)
	for _,acard in ipairs(cards) do
		table.insert(needed, acard:getEffectiveId())
			if #needed >= hp then
				break
			end
	end
	for _,acard in ipairs(equips) do
			if #needed < hp then
				table.insert(needed, acard:getEffectiveId())
			end
	end
	
	local targets = {}
	for _,p in ipairs(self.enemies) do
	  if #targets <hp and not p:isKongcheng() and add_different_kingdoms2(self, p, targets) then table.insert(targets, p) end
	end

   if #needed == hp and #targets >0  then
	  use.card = sgs.Card_Parse("@ShoujiCard="..table.concat(needed,"+").."&shouji")
	  if use.to then
        for _,target in ipairs(targets) do	  
	      use.to:append(target)
		end
	  end
	  return
   end
end

sgs.ai_use_value.ShoujiCard= 8
sgs.ai_use_priority.ShoujiCard  = 5

sgs.ai_skill_invoke.zhouli= function(self, data)
   local damage = data:toDamage()
   return (self:isEnemy(damage.from) and not damage.to:hasShownAllGenerals()) or (self:isFriend(damage.to) and self:isFriend(damage.from))
end

sgs.ai_skill_choice.zhouli = function(self, choices, data)
   local damage = data:toDamage()
   if self:isFriend(damage.to) and table.contains(choices:split("+"), "zhouli_prevent") then
      return "zhouli_prevent"
   end
end

sgs.ai_skill_invoke.zhenyan= function(self, data)
   for _,p in ipairs(self.friends_noself) do
	  if #self.room:getTag(p:objectName().."zhenyan"):toStringList()>0 then
	    return true
	  end
   end
end

sgs.ai_skill_playerchosen.zhenyan = function(self, targets, max_num, min_num)
  for _,p in ipairs(self.friends_noself) do
	  if #self.room:getTag(p:objectName().."zhenyan"):toStringList()>0 then
	    return p
	  end
   end
end

sgs.ai_skill_invoke.yuyue= function(self, data)
   if self.player:hasShownSkill("yuyue") then return true end
   return self:willShowForAttack() or self:willShowForDefence()
end

sgs.ai_skill_invoke.xianhai= function(self, data)
   local damage = data:toDamage()
   if not damage.from:isFriendWith(self.player) then
     return true
   end
end

sgs.ai_skill_playerchosen.xianhai= function(self, targets)
	return self.player
end

sgs.ai_skill_invoke.wuren= function(self, data)
   return self:willShowForAttack() or self:willShowForDefence()
end

sgs.ai_skill_invoke.weituo= function(self, data)
   local damage = data:toDamage()
   return self:isEnemy(damage.from) or self.player:willBeFriendWith(damage.from)
end

sgs.ai_skill_invoke.wushi= function(self, data)
   return self:willShowForAttack() or self:willShowForDefence()
end

sgs.ai_skill_playerchosen.wushi= function(self, targets)
    local result = {}
	for _,name in sgs.qlist(targets) do
		if  self:isEnemy(name) then table.insert(result, findPlayerByObjectName(name:objectName())) end
	end
	return result
end

sgs.ai_skill_invoke.zhuisha = function(self, data)
	local use=data:toCardUse()
	for _,p in sgs.qlist(use.to) do
	  if (self:isEnemy(p) and not p:isNude()) then
	    return true
	  end
	end
end

sgs.ai_skill_discard["zhuisha"] = function(self, discard_num, min_num, optional, include_equip)
  return self:askForDiscard("discard", discard_num, min_num, false, include_equip)
end

sgs.ai_skill_invoke.songzang = function(self, data)
	local damage=data:toDamage()
	if self:isEnemy(damage.to) then
	    return true
	end
end

sgs.ai_skill_invoke.shashou = function(self, data)
	local damage=data:toDamage()
	if self:isEnemy(damage.to) then
	    return true
	end
end

sgs.ai_skill_invoke.xuexi = function(self, data)
	return true
end

sgs.ai_skill_invoke.qinggan = function(self, data)
	return true
end

sgs.ai_skill_cardask["@shengmu"] = function(self, data)
  local damage = data:toDamage()
  local can
  if damage and self:isFriend(damage.to) then
     if damage.to:isWounded() then
	    can = true
	 end
  end
  for _, c in sgs.qlist(self.player:getCards("he")) do
	 if c:isRed() and can then
		return "$" .. c:getEffectiveId()
	 end
  end
  for _, c in sgs.qlist(self.player:getCards("he")) do
	 if can then
		return "$" .. c:getEffectiveId()
	 end
  end
  return ""
end

sgs.ai_skill_invoke.fenjie = function(self, data)
	local effect=data:toCardEffect()
	if self:isEnemy(effect.to) and effect.to:getEquips():length()>0 then
	    return true
	end
end

sgs.ai_skill_invoke.jiaji = function(self, data)
	return self:willShowForAttack() or self:willShowForDefence()
end

sgs.ai_view_as.gaoling = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local ask = sgs.Sanguosha:getCurrentCardUsePattern()
	if (card_place == sgs.Player_PlaceEquip or card_place == sgs.Player_PlaceHand) and ask ~= "jink" and player:getHandcardNum()==math.ceil(player:getHandcardNum()/2) * 2 then
		return ("nullification:gaoling[%s:%s]=%d%s"):format(suit, number, card_id, "&gaoling")
	end
	if  (card_place == sgs.Player_PlaceEquip or card_place == sgs.Player_PlaceHand) and ask == "jink" and player:getHandcardNum()==math.ceil(player:getHandcardNum()/2) * 2 then
		return ("jink:gaoling[%s:%s]=%d%s"):format(suit, number, card_id, "&gaoling")
	end
end

sgs.ai_skill_choice.gaoling = function(self, choices, data)
   return "draw1card"
end

sgs.ai_skill_invoke.bingdu = function(self, data)
	return true
end

sgs.ai_skill_invoke.juewang = function(self, data)
	return self:willShowForAttack()
end