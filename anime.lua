extension = sgs.Package("animetest", sgs.Package_GeneralPack)

abc = sgs.General(extension , "abc", "magic", 4)
megumin = sgs.General(extension , "megumin", "real", 3, false)

animeskill1 = sgs.CreateTriggerSkill{   --技能本身
	name = "animeskill1", --技能名字，为字符串
	events = {sgs.Damaged}, --事件表，多个事件在括号里用逗号隔开
	can_trigger = function(self, event, room, player, data)--这个函数用来判断技能是否可触发，人物牌暗置时通过预亮进行判断。返回技能名表示可以触发，返回空字符串表示不可触发。
	     -- self 为技能本身， event为事件，room为房间， player为玩家， data为事件包含的数据
		if player and player:isAlive() and player:hasSkill(self:objectName()) then  -- 判断语句，如果玩家存活并拥有该技能。 这里event只有一个，所以不需要对event进行判断。 
			return self:objectName() --返回技能名表示可以触发， self表示该技能本身，self:objectName()为技能名即是"animeskill1"
		end
		return "" --若不满足条件，返回空字符串表示不可触发。
	end ,
	on_cost = function(self, event, room, player, data,ask_who) --发动消耗，一般满足条件会询问玩家是否发动，玩家发动即可触发技能，触发技能时原本暗置的人物牌会立即明置。		
	    --ask_who为另一个玩家变量， 以后再说明此变量的用处
		if player:askForSkillInvoke(self, data) then --若玩家选择触发技能
		    room:broadcastSkillInvoke("animeskill1", player) --播放音频，随机编号
		    return true --触发成功
		end
		return false  -- 不选择则不触发
	end ,
	on_effect = function(self, event, room, player, data,ask_who)--技能效果，技能的具体效果代码，选择发动后会执行
		player:drawCards(10)--你需要进一步写的效果A		
	end ,
}

animeskill2 = sgs.CreateViewAsSkill{   --技能本身
  name = "animeskill2", --技能名字，为字符串
  n = 1, --表示最多选择1张牌当作对应牌使用
  view_filter = function(self, selected, to_select)--布尔值函数，检索可以选择的牌。 若可以选择，则返回true
    --self 为技能本身， selected 为已经选择的卡牌列表， to_select为将要选择的下一张牌
	return (#selected == 0) and (not to_select:isEquipped()) --返回只有在你没有选牌时，且要选择牌不为装备区的牌时才能选择，也就是只能选择一张手牌的逻辑。
  end,
  
  view_as = function(self, cards)--卡牌函数，生成你最终要使用的卡牌
	--self 为技能本身， cards为你已经选中的卡牌列表
	
	if #cards<1 then return nil end 
	--如果列表长度为0，即没有牌被选中，那么返回空对象，即不生成卡牌
	
	local suit,number --定义两个变量，分别为花色和点数
	
	for _,card in ipairs(cards) do
		if suit and (suit~=card:getSuit()) then suit=sgs.Card_NoSuit else suit=card:getSuit() end
		if number and (number~=card:getNumber()) then number=-1 else number=card:getNumber() end
	end
	
	--如果所有被选中的牌的花色一样，那么被视为的卡牌的花色也为该花色。否则，该卡牌无花色。
	--点数同理。
	
	local view_as_card= sgs.Sanguosha:cloneCard("eirei_shoukan", suit, number)
	--生成一张英灵召唤卡牌， 英灵召唤的名称字符串为"eirei_shoukan"，可以在 image/card里面查看。
	
	for _,card in ipairs(cards) do
		view_as_card:addSubcard(card:getId())
	end
	--将被用作视为英灵召唤的牌都加入到英灵召唤的subcards里
	
	view_as_card:setSkillName(self:objectName())
	--标记该英灵召唤是由本技能生成的
	
	view_as_card:setShowSkill(self:objectName())
	--标记生成该英灵召唤的技能，用于亮将
	
	return view_as_card
end,
  
}

abc:addSkill(animeskill1) --添加技能
abc:addSkill(animeskill2) --添加技能
abc:addCompanion("megumin") --增加珠联璧合

sgs.LoadTranslationTable{
  ["animetest"] = "动漫测试包", --卡牌包名称
  ["abc"] = "二次元最强",   --人物名称
  ["&abc"] = "二次元最强",  --人物对战显示名称，默认和人物名称一样，所以除非和原本人物名不一样否则不用写
  ["#abc"] = "最强传说", --人物称号
  ["~abc"] = "不可能！", --人物阵亡配音翻译
  ["designer:abc"] = "动漫杀萌新", --人物设计者
  ["cv:abc"] = "未知", --人物cv
  
  ["animeskill1"] = "技能一", --技能名称
  [":animeskill1"] = "当你受到伤害后，你可以摸10张牌", --技能内容
  ["$animeskill11"] = "未知", --技能编号1音频的翻译，一般格式是 "$xxx1"，这里技能有数字不知道是否会影响
  
  ["animeskill2"] = "技能二", --同上
  [":animeskill2"] = "出牌阶段，你可以将一张牌当作“英灵召唤”使用。", 
  ["$animeskill21"] = "未知", 
}


MegCard = sgs.CreateSkillCard{ --技能卡创建
	name = "MegCard",  --技能卡名字
	target_fixed = false, --是否固定目标，默认是false，一般情况下我们也用false所以该情况可以不写
	will_throw = true,   --使用此卡时是否弃置此卡，默认是true，一般情况下我们也用true所以该情况可以不写
	filter = function(self, targets, to_select, player) --布尔值函数，判断技能卡是否能选择目标
	    --self是该技能卡，targets是已经选定的角色列表，to_select是将选择的玩家，player是玩家自己	
		return #targets < 1 and to_select:objectName() ~= player:objectName() --列表为空且选择对象不为自己时才能选择
	end,
	on_use = function(self, room, source, targets)	--空值函数，写技能卡具体的效果
	     --self是该技能卡，room是房间，source是技能卡来源即玩家自己，targets是已经选定的角色列表
         local target = targets[1] --获取列表第一个玩家，命名为target。这里因为只有一个玩家，否则我们需要对targets做遍历语句。
		 local damage = sgs.DamageStruct() --创建一个伤害结构
		 damage.from = source --source为该伤害来源
		 damage.to = target --source为该伤害目标
		 damage.nature = sgs.DamageStruct_Normal --伤害属性为普通，这个是默认值所以可以不写
		 damage.damage = 1 --伤害点数为1，1是默认值所以其实可以不写
		 room:damage(damage) --执行此次伤害
	end
}

meg = sgs.CreateViewAsSkill{   --技能本身
  name = "meg",
  n = 2, --表示最多选择2张牌当作对应牌使用
  
  view_filter = function(self, selected, to_select)
	return (#selected < 2)
  end,
  
  enabled_at_play = function(self, target)--判断是否能在出牌阶段发动技能
     --self是该技能，target是玩家自己
	 return not target:hasUsed("#MegCard") --在target此回合没用过该技能卡时才能发动
  end,
  
  view_as = function(self, cards)
	
	if #cards<2 then return nil end 
	--如果列表长度小于2，那么返回空对象，即不生成卡牌
	

	local view_as_card= MegCard:clone()
	--生成一张MegCard技能卡	
	
	for _,card in ipairs(cards) do
		view_as_card:addSubcard(card:getId()) --将选中的2张牌加入技能卡
	end
	
	view_as_card:setSkillName(self:objectName())
	
	view_as_card:setShowSkill(self:objectName())
	
	return view_as_card
end, 
}

HelpCard = sgs.CreateSkillCard{ 
	name = "HelpCard", 
	will_throw = false,   --重要步骤，此时我们是把牌交出去，所以不是弃置牌。
	filter = function(self, targets, to_select, player)
		return #targets < 1 and to_select:objectName() ~= player:objectName() --同之前
	end,
	on_use = function(self, room, source, targets)
         local target = targets[1] --获取列表第一个玩家，命名为target。这里因为只有一个玩家，否则我们需要对targets做遍历语句。
		 room:obtainCard(target, self, false) --target获取该卡牌，即self，这里false表示获得时不展示卡牌
	end
}

Helpvs = sgs.CreateViewAsSkill{
  name = "helpvs",
  n = 2,  
  view_filter = function(self, selected, to_select)
	return (#selected < 2)
  end,
  
  enabled_at_play = function(self, target)
	 return false --不能在出牌阶段使用
  end,
  
  enabled_at_response = function(self, player, pattern) --该视为技能够响应的时机
     --self为技能，player为玩家自己，pattern是字符串指令
	 return pattern == "@@help" --只有在字符串指令为"@@help"时才能使用该技能，这个字符串我们会在接下来的触发技定义
  end,
  
  view_as = function(self, cards)
	
	if #cards==0 then return nil end 
	--如果列表长度为空，那么返回空对象，即不生成卡牌
	

	local view_as_card= HelpCard:clone()
	--生成一张HelpCard技能卡	
	
	for _,card in ipairs(cards) do
		view_as_card:addSubcard(card:getId()) --将选中的1~2张牌加入技能卡
	end
	
	view_as_card:setSkillName(self:objectName())
	
	view_as_card:setShowSkill(self:objectName())
	
	return view_as_card
end, 
}

megumin:addSkill(meg)
megumin:addSkill(Helpvs)

return {extension}