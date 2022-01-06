
return {
  ["fading"] = "消逝之章",
  ["fadingcard"] = "消逝之章",
  ["specialcard"] = "特殊卡牌包",
  
  ["food_card"] = "食物牌",
  ["mapo_tofu"] = "激辣的麻婆豆腐",
	[":mapo_tofu"] = "基本牌<br /><b>时机</b>：出牌阶段<br /><b>目标</b>：距离为1的一名角色<br /><b>效果</b>：目标角色受到X点火属性伤害（该伤害不会被传导），该回合结束时目标角色回复X+1点体力（X为目标角色的体力值-1且至少为0）。每回合限使用一次，每回合每名角色限选择一次。",
	["#MapoTofuUse"] = "%from 令 %to 食用了激辣的麻婆豆腐！",
	["#MapoTofuRecover"] = "%from 感受到了麻婆豆腐的魅力！",
	
	["tacos"] = "饼",
	[":tacos"] = "基本牌<br />出牌时机：出牌阶段<br />使用目标：自己<br />作用效果：你随机获得弃牌堆中的一张牌，然后将弃牌堆上3/4的牌（向下取整）置于牌堆顶。<br />",

  --现世
  ["mengxian"] = "梦现",
  ["$mengxian1"] = "（长门神）通俗地说，我是相当于宇宙人的存在。",
	["$mengxian2"] = "（朝比奈学姐）我不是这个时代的人，是从更远的未来来的。",
	["$mengxian3"] = "（古泉）如您明察，我是超能力者。",
	[":mengxian"] = "摸牌阶段开始时，你可以声明一种牌的类别，然后进行判定。你重复该过程直到判定牌为你所声明的类别，然后获得此牌。",
	["mengxian$"] = "image=image/animate/mengxian.png",
	["yuanwang$"] = "image=image/animate/yuanwang.png",
	["yuanwang"] = "愿望",
	[":yuanwang"] = "社团技，「SOS团」\n加入条件：出牌阶段开始时，你可以询问一名明置人物牌且符合X条件的其他角色是否加入你的「SOS团」（X：除你之外，社团中每种势力的角色最多一名）。若其加入，将其平置之。\n社团效果：一名「SOS团」成员的回合内，其所属的势力成为唯一大势力。其可以将一张黑色手牌当作“SOS社团活动”使用。",
	["$yuanwang1"] = "我对一般的人类没有兴趣，如果你们中谁是宇宙人，未来人，异世界人或者超能力者的话，就到我这里来！",
	["sos"] = "「SOS团」",
	["@yuanwang"] = "选择一名角色，询问其是否加入「SOS团」",
	["yuanwang_accept"] =  "接受团长的邀请并加入「SOS团」",
	["haruhi"] = "凉宫春日",
	["&haruhi"] = "凉宫春日",
	["@haruhi"] = "凉宫春日的忧郁",
	["#haruhi"] = "团长大人",
	["~haruhi"] = "你不也觉得那个世界很无聊么？你就不希望有什么更有趣的事情发生么？！",
	["cv:haruhi"] = "平野绫",
	["designer:haruhi"] = "帕秋莉·萝莉姬",
	["$yuanwang_add_phase_draw"] = "由于 <font color=\"#f2a0a1\"><b>团长大人</b></font> 的力量， %from 在奇怪的时间点获得了一个额外的摸牌阶段！",
	["$yuanwang_add_phase_play"] = "由于 <font color=\"#f2a0a1\"><b>团长大人</b></font> 的力量， %from 在奇怪的时间点获得了一个额外的出牌阶段！",
	["$yuanwang_mikuru_beam"] = "由于 <font color=\"#f2a0a1\"><b>团长大人</b></font> 的力量， %from 对 %to 发射了光线！",
	["$yuanwang_card_back"] = "由于 <font color=\"#f2a0a1\"><b>团长大人</b></font> 的力量， %from 收回了使用的 %card ！",
	["$yuanwang_august"] = "由于 <font color=\"#f2a0a1\"><b>团长大人</b></font> 的力量， %from 进行了永无止境的八月！",
	["$yuanwang_obtain"] = "<font color=\"#f2a0a1\"><b>团长大人</b></font> 明抢了 %arg 社员 %from 的 %card ！",
	["$yuanwang_wear"] = "<font color=\"#f2a0a1\"><b>团长大人</b></font> 强制社员 %from 穿上了 %card ！",
	["$yuanwang_closed_space"] = "<font color=\"#f2a0a1\"><b>团长大人</b></font> 很不爽，开启了「闭锁空间」！",
	["$yuanwang_closed_space_effect"] = "<font color=\"#f2a0a1\"><b>团长大人</b></font> 的「闭锁空间」下，任何【杀】或【锦囊牌】不可被响应。",
	["closedSpace"] = "「闭锁空间」",
	["yuanwang$"] = "image=image/animate/yuanwang.png",
    ["%haruhi"] = "“我对一般的人类没有兴趣，如果你们中谁是宇宙人，未来人，异世界人或者超能力者的话，就到我这里来！”",
  --魔法
  
  
  --科学
  ["Takamakuri"] = "鹰捲",
	["$Takamakuri1"] = "鹰捲，不是毒!",
	["$Takamakuri2"] = "人体中蕴藏着一种由微细脉冲电流引发的震动，鹰捲能将这一震动振幅。",
	["$Takamakuri3"] = "并击打到对方身上，通过让对方身体旋转产生螺旋效应!",
	[":Takamakuri"] = "当你对一名角色造成伤害后，你可以展示牌顶一张牌，如为基本牌，你获得之，并弃置该角色的一张装备区的牌。",
	["Tobiugachi"] = "鸢穿",
	["$Tobiugachi"] = "钥匙到手了！",
	[":Tobiugachi"] = "每当你需要使用或打出一张闪时，如你此时手牌数大于当前体力数，你可将手牌弃至X-1张，视为你使用或打出一张闪（X为你当前体力值），然后获得任意角色区域或私有牌堆内的一张牌。",
	["ToBiGetRegion"] = "获得其区域内的一张牌",
	["TobiGetPile"] = "获得其任意牌堆内的一张牌",
	["Fukurouza"] = "悲忆",
	["$Fukurouza"] = "如果我当时开枪了，犯人已经死了，这根本就不是武侦！",
	[":Fukurouza"] = "一名角色结束阶段结束时，如你于其回合内发动过鳶穿或鷹捲，你可以摸一张牌。",
	["FukurouzaTaka"] = "悲忆（鷹捲）",
	["FukurouzaTobi"] = "悲忆（鳶穿）",
	["Akari"] = "间宫明里",
	["&Akari"] = "间宫明里",
	["@Akari"] = "绯弹的亚里亚AA",
	["#Akari"] = "爆衣杀手",
	["~Akari"] = "所以是因为我太弱了吗！",
	["designer:Akari"] = "钉子",
	["cv:Akari"] = "佐倉綾音",
	["illustrator:Akari"] = "竹久めい@旧名ジョン",
	["%Akari"] = "“鹰捲，不是毒!”",

  --游戏  
  
	
  --野心家

}

