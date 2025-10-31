#!/usr/bin/env python3
import cgi
import cgitb
from collections import Counter
import html

# Enable CGI error reporting
cgitb.enable()

# Parse form data
form = cgi.FieldStorage()

# Get all answers
answers = []
for i in range(1, 9):
	answer = form.getvalue(f'q{i}', '')
	if answer:
		answers.append(answer)

# Count occurrences of each monster type
if answers:
	counts = Counter(answers)
	monster_type = counts.most_common(1)[0][0]
else:
	monster_type = 'unknown'

# Monster profiles
monsters = {
	'vampire': {
		'name': 'VAMPIRE',
		'emoji': '🧛',
		'title': 'The Immortal Blood Drinker',
		'description': 'You are elegant, sophisticated, and eternally charming. You prefer the night and have a taste for the finer things in death. Your hypnotic gaze can mesmerize anyone, and you\'ve perfected the art of living forever.',
		'strengths': 'Immortality, Shape-shifting, Hypnosis, Superhuman abilities',
		'weaknesses': 'Sunlight, Garlic, Holy water, Wooden stakes',
		'quote': '"I vant to suck your blood... but make it fashion."',
		'color': '#8B0000'
	},
	'werewolf': {
		'name': 'WEREWOLF',
		'emoji': '🐺',
		'title': 'The Moon-Cursed Beast',
		'description': 'You are wild, passionate, and ruled by primal instincts. When the full moon rises, your true nature emerges. You value loyalty to your pack and have an uncontrollable fury that makes you formidable.',
		'strengths': 'Superhuman strength, Enhanced senses, Pack mentality, Healing factor',
		'weaknesses': 'Silver, Full moon control loss, Vulnerability in human form',
		'quote': '"I didn\'t choose the wolf life, the wolf life chose me."',
		'color': '#8B4513'
	},
	'zombie': {
		'name': 'ZOMBIE',
		'emoji': '🧟',
		'title': 'The Undead Walker',
		'description': 'You are relentless, unstoppable, and always hungry. Life (or death) is simple for you - just keep moving forward. You may be slow, but you\'re persistent, and you never give up. Plus, you\'re great at making friends.',
		'strengths': 'Immunity to pain, Infectious bite, Never tire, Strength in numbers',
		'weaknesses': 'Slow movement, Low intelligence, Headshots, Easily distracted by brains',
		'quote': '"Braaaains... is that too much to ask?"',
		'color': '#2F4F4F'
	},
	'frankenstein': {
		'name': 'FRANKENSTEIN\'S MONSTER',
		'emoji': '🧟‍♂️',
		'title': 'The Misunderstood Creation',
		'description': 'You are a scientific marvel, stitched together from various parts and brought to life by lightning. Despite your fearsome appearance, you have a sensitive soul and just want to be accepted. You\'re stronger than you look!',
		'strengths': 'Superhuman strength, High intelligence, Electrical immunity, Durability',
		'weaknesses': 'Fire, Rejection and loneliness, Misunderstood by society',
		'quote': '"I didn\'t ask to be created, but here I am, bolts and all."',
		'color': '#4B5320'
	},
	'ghost': {
		'name': 'GHOST',
		'emoji': '👻',
		'title': 'The Ethereal Spirit',
		'description': 'You are mysterious, intangible, and definitely not tied down by physical limitations. You can walk through walls, appear and disappear at will, and your favorite hobby is making spooky noises at 3 AM.',
		'strengths': 'Invisibility, Intangibility, Flight, Immortality',
		'weaknesses': 'Can\'t interact with physical world, Bound to location, Salt circles',
		'quote': '"I\'m not saying Boo to scare you, I\'m just saying hi."',
		'color': '#E0E0E0'
	},
	'unknown': {
		'name': 'MYSTERIOUS ENTITY',
		'emoji': '❓',
		'title': 'The Undefined Horror',
		'description': 'You are so mysterious that even we can\'t classify you. Perhaps you\'re a new breed of monster, or maybe you\'re still discovering your true form. Either way, you\'re definitely spooky!',
		'strengths': 'Unpredictable, Mysterious, Adaptable',
		'weaknesses': 'Unknown',
		'quote': '"Even I don\'t know what I am..."',
		'color': '#4B0082'
	}
}

monster = monsters[monster_type]

# Print HTTP header
print("Content-Type: text/html\n")

# Print HTML
html_output = f"""<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>👻 You Are A {monster['name']}! 👻</title>
	<style>
		body {{
			background-color: #0a0a0a;
			color: #ff6600;
			font-family: 'Courier New', monospace;
			text-align: center;
			padding: 50px;
			animation: fadeIn 2s;
		}}
		@keyframes fadeIn {{
			from {{ opacity: 0; transform: scale(0.8); }}
			to {{ opacity: 1; transform: scale(1); }}
		}}
		.container {{
			max-width: 800px;
			margin: 0 auto;
			background-color: #1a1a1a;
			padding: 50px;
			border: 4px solid {monster['color']};
			border-radius: 20px;
			box-shadow: 0 0 50px {monster['color']};
		}}
		.monster-emoji {{
			font-size: 10em;
			animation: float 3s ease-in-out infinite;
		}}
		@keyframes float {{
			0%, 100% {{ transform: translateY(0) rotate(0deg); }}
			50% {{ transform: translateY(-30px) rotate(10deg); }}
		}}
		h1 {{
			font-size: 3.5em;
			color: {monster['color']};
			text-shadow: 3px 3px 6px #000;
			margin: 20px 0;
			animation: pulse 2s infinite;
		}}
		@keyframes pulse {{
			0%, 100% {{ transform: scale(1); }}
			50% {{ transform: scale(1.05); }}
		}}
		.title {{
			font-size: 1.8em;
			color: #ffff00;
			margin-bottom: 30px;
			font-style: italic;
		}}
		.description {{
			background-color: #2d2d2d;
			padding: 30px;
			margin: 30px 0;
			border: 2px solid {monster['color']};
			border-radius: 10px;
			text-align: left;
			font-size: 1.2em;
			line-height: 1.8;
		}}
		.stats-box {{
			background-color: #1a1a1a;
			padding: 25px;
			margin: 20px 0;
			border: 2px solid #ff6600;
			border-radius: 10px;
			text-align: left;
		}}
		.stats-title {{
			font-size: 1.5em;
			color: #ff6600;
			font-weight: bold;
			margin-bottom: 15px;
		}}
		.stats-content {{
			font-size: 1.1em;
			color: #cccccc;
			line-height: 1.6;
		}}
		.quote {{
			font-size: 1.5em;
			color: {monster['color']};
			font-style: italic;
			margin: 30px 0;
			padding: 20px;
			background-color: #0a0a0a;
			border-left: 5px solid {monster['color']};
		}}
		.back-button {{
			display: inline-block;
			margin-top: 40px;
			padding: 20px 50px;
			font-size: 1.3em;
			background-color: {monster['color']};
			color: #fff;
			text-decoration: none;
			border-radius: 10px;
			font-weight: bold;
			transition: all 0.3s;
			text-transform: uppercase;
		}}
		.back-button:hover {{
			transform: scale(1.1);
			box-shadow: 0 0 30px {monster['color']};
		}}
		.warning {{
			color: #ff0000;
			font-size: 1.5em;
			margin-top: 30px;
			animation: blink 1.5s infinite;
		}}
		@keyframes blink {{
			0%, 50%, 100% {{ opacity: 1; }}
			25%, 75% {{ opacity: 0; }}
		}}
	</style>
</head>
<body>
	<div class="container">
		<div class="monster-emoji">{monster['emoji']}</div>
		<h1>YOU ARE A {monster['name']}!</h1>
		<div class="title">{monster['title']}</div>

		<div class="description">
			{monster['description']}
		</div>

		<div class="stats-box">
			<div class="stats-title">💪 YOUR STRENGTHS:</div>
			<div class="stats-content">{monster['strengths']}</div>
		</div>

		<div class="stats-box">
			<div class="stats-title">⚠️ YOUR WEAKNESSES:</div>
			<div class="stats-content">{monster['weaknesses']}</div>
		</div>

		<div class="quote">
			{monster['quote']}
		</div>

		<p class="warning">🎃 YOUR TRANSFORMATION IS COMPLETE 🎃</p>

		<a href="monster_quiz.py" class="back-button">🔄 Take Quiz Again</a>
	</div>
</body>
</html>
"""

print(html_output)
