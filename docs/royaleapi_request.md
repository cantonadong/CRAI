# 给 RoyaleAPI 的数据授权申请（草稿）

**背景**：royaleapi.com 的 robots.txt 声明 `Content-Signal: ai-train=no`，即不允许用其内容训练 AI 模型。要使用回放数据训练，需要先取得书面授权。

**联系渠道**（请自行确认最新方式）：RoyaleAPI 官网页脚的 Contact 页面、官方 Discord，或 discuss.royaleapi.com 论坛。

---

**Subject:** Request for permission to use battle replay data for a non-commercial AI research project

Hi RoyaleAPI team,

I'm working on a personal, non-commercial research project: an AI agent that plays Clash Royale on a private test server (not on Supercell's official servers). I'd like to learn deck-specific play patterns from human replays. In particular I'm interested in the card placement timelines (tick, card, position) shown in your battle replay viewer.

I noticed that your robots.txt sets `ai-train=no`, so I wanted to ask before using any of your data:

1. Would you grant permission to use replay data (card play timings and positions) from a limited set of top-ladder players to train a model? This would be roughly N players × the last ~25 battles each, refreshed occasionally.
2. If so, is there a preferred way to access it (an export or API endpoint, rate limits, attribution requirements)? I'd rather use an official channel than scrape the website.
3. If permission isn't possible, I fully understand and will not use the data.

I'm happy to share details about the project, credit RoyaleAPI, and follow any conditions you set.

Thanks for building such a great resource.

Best regards,
[你的名字 / 联系方式]

---

**中文要点**：
- 说明这是非商业的个人研究，运行在私服上。
- 主动提到对方 robots.txt 里的 `ai-train=no`，先问再用。
- 明确想要的数据和数量，请对方给出官方获取方式和使用条件。
- 被拒绝就不用。
