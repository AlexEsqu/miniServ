#!/usr/bin/env python3

print("Content-Type: text/html\n")

html = """<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>🎃 What Monster Are You? 🎃</title>
	<style>
		body {
			background-color: #0a0a0a;
			color: #ff6600;
			font-family: 'Courier New', monospace;
			text-align: center;
			padding: 50px;
			animation: fadeIn 1s;
		}
		@keyframes fadeIn {
			from { opacity: 0; }
			to { opacity: 1; }
		}
		.container {
			max-width: 800px;
			margin: 0 auto;
			background-color: #1a1a1a;
			padding: 40px;
			border: 3px solid #ff0000;
			border-radius: 15px;
			box-shadow: 0 0 30px #ff0000;
		}
		h1 {
			font-size: 3em;
			text-shadow: 3px 3px 6px #000;
			margin-bottom: 20px;
			animation: pulse 2s infinite;
		}
		@keyframes pulse {
			0%, 100% { opacity: 1; }
			50% { opacity: 0.7; }
		}
		.moon {
			font-size: 6em;
			animation: glow 3s ease-in-out infinite;
		}
		@keyframes glow {
			0%, 100% { text-shadow: 0 0 10px #ffff00; }
			50% { text-shadow: 0 0 30px #ffff00, 0 0 40px #ff6600; }
		}
		.question {
			background-color: #2d2d2d;
			padding: 30px;
			margin: 30px 0;
			border: 2px solid #ff6600;
			border-radius: 10px;
			text-align: left;
		}
		.question-title {
			font-size: 1.5em;
			color: #ffff00;
			margin-bottom: 20px;
			font-weight: bold;
		}
		.answer-option {
			margin: 15px 0;
		}
		.answer-option input[type="radio"] {
			display: none;
		}
		.answer-option label {
			display: block;
			padding: 15px 20px;
			background-color: #1a1a1a;
			border: 2px solid #ff6600;
			border-radius: 8px;
			cursor: pointer;
			transition: all 0.3s;
			font-size: 1.1em;
		}
		.answer-option label:hover {
			background-color: #2d2d2d;
			border-color: #ff9900;
			transform: translateX(10px);
		}
		.answer-option input[type="radio"]:checked + label {
			background-color: #ff6600;
			color: #000;
			border-color: #ffff00;
			font-weight: bold;
		}
		.submit-button {
			margin-top: 30px;
			padding: 20px 50px;
			font-size: 1.5em;
			background-color: #ff0000;
			color: #fff;
			border: none;
			border-radius: 10px;
			cursor: pointer;
			font-weight: bold;
			transition: all 0.3s;
			text-transform: uppercase;
		}
		.submit-button:hover {
			background-color: #ff3333;
			transform: scale(1.1);
			box-shadow: 0 0 30px #ff0000;
		}
		.warning {
			color: #ff0000;
			font-size: 1.2em;
			margin-top: 20px;
			font-style: italic;
		}
	</style>
</head>
<body>
	<div class="container">
		<div class="moon">🌕</div>
		<h1>🎃 MONSTER PERSONALITY TEST 🎃</h1>
		<p style="font-size: 1.3em; margin-bottom: 30px;">
			Answer these questions to discover your TRUE monster identity...
		</p>

		<form method="GET" action="monster_result.py">

			<!-- Question 1 -->
			<div class="question">
				<div class="question-title">1. What time of day do you feel most alive?</div>
				<div class="answer-option">
					<input type="radio" id="q1a" name="q1" value="vampire" required>
					<label for="q1a">🌙 Midnight - I'm a creature of the night</label>
				</div>
				<div class="answer-option">
					<input type="radio" id="q1b" name="q1" value="werewolf">
					<label for="q1b">🌕 Full moon nights - When the beast awakens</label>
				</div>
				<div class="answer-option">
					<input type="radio" id="q1c" name="q1" value="zombie">
					<label for="q1c">🌫️ Dawn - Emerging from my grave</label>
				</div>
				<div class="answer-option">
					<input type="radio" id="q1d" name="q1" value="frankenstein">
					<label for="q1d">⚡ During thunderstorms - Electric energy!</label>
				</div>
			</div>

			<!-- Question 2 -->
			<div class="question">
				<div class="question-title">2. What's your ideal meal?</div>
				<div class="answer-option">
					<input type="radio" id="q2a" name="q2" value="vampire" required>
					<label for="q2a">🩸 Something... liquid and red</label>
				</div>
				<div class="answer-option">
					<input type="radio" id="q2b" name="q2" value="werewolf">
					<label for="q2b">🥩 Rare meat - The rarer the better</label>
				</div>
				<div class="answer-option">
					<input type="radio" id="q2c" name="q2" value="zombie">
					<label for="q2c">🧠 Brain food - Literally</label>
				</div>
				<div class="answer-option">
					<input type="radio" id="q2d" name="q2" value="ghost">
					<label for="q2d">👻 I don't eat - I'm ethereal</label>
				</div>
			</div>

			<!-- Question 3 -->
			<div class="question">
				<div class="question-title">3. How do you prefer to travel?</div>
				<div class="answer-option">
					<input type="radio" id="q3a" name="q3" value="vampire" required>
					<label for="q3a">🦇 Transform into a bat and fly</label>
				</div>
				<div class="answer-option">
					<input type="radio" id="q3b" name="q3" value="werewolf">
					<label for="q3b">🐺 Running on all fours through the forest</label>
				</div>
				<div class="answer-option">
					<input type="radio" id="q3c" name="q3" value="zombie">
					<label for="q3c">🚶 Slow shuffling walk</label>
				</div>
				<div class="answer-option">
					<input type="radio" id="q3d" name="q3" value="ghost">
					<label for="q3d">💨 Float through walls</label>
				</div>
			</div>

			<!-- Question 4 -->
			<div class="question">
				<div class="question-title">4. What's your biggest weakness?</div>
				<div class="answer-option">
					<input type="radio" id="q4a" name="q4" value="vampire" required>
					<label for="q4a">☀️ Sunlight and garlic</label>
				</div>
				<div class="answer-option">
					<input type="radio" id="q4b" name="q4" value="werewolf">
					<label for="q4b">🔫 Silver bullets</label>
				</div>
				<div class="answer-option">
					<input type="radio" id="q4c" name="q4" value="zombie">
					<label for="q4c">🎯 Headshots</label>
				</div>
				<div class="answer-option">
					<input type="radio" id="q4d" name="q4" value="frankenstein">
					<label for="q4d">🔥 Fire - I'm afraid of it</label>
				</div>
			</div>

			<!-- Question 5 -->
			<div class="question">
				<div class="question-title">5. What's your fashion style?</div>
				<div class="answer-option">
					<input type="radio" id="q5a" name="q5" value="vampire" required>
					<label for="q5a">🎩 Victorian elegance with a cape</label>
				</div>
				<div class="answer-option">
					<input type="radio" id="q5b" name="q5" value="werewolf">
					<label for="q5b">👕 Torn clothes - Transforming ruins everything</label>
				</div>
				<div class="answer-option">
					<input type="radio" id="q5c" name="q5" value="zombie">
					<label for="q5c">🩹 Tattered and dirty</label>
				</div>
				<div class="answer-option">
					<input type="radio" id="q5d" name="q5" value="frankenstein">
					<label for="q5d">⚙️ Stitched together from various parts</label>
				</div>
			</div>

			<!-- Question 6 -->
			<div class="question">
				<div class="question-title">6. How do you handle conflict?</div>
				<div class="answer-option">
					<input type="radio" id="q6a" name="q6" value="vampire" required>
					<label for="q6a">🧛 Charm and hypnotize my enemies</label>
				</div>
				<div class="answer-option">
					<input type="radio" id="q6b" name="q6" value="werewolf">
					<label for="q6b">😤 Raw aggression and fury</label>
				</div>
				<div class="answer-option">
					<input type="radio" id="q6c" name="q6" value="zombie">
					<label for="q6c">🧟 Overwhelming numbers</label>
				</div>
				<div class="answer-option">
					<input type="radio" id="q6d" name="q6" value="ghost">
					<label for="q6d">😱 Psychological terror</label>
				</div>
			</div>

			<!-- Question 7 -->
			<div class="question">
				<div class="question-title">7. Where would you live?</div>
				<div class="answer-option">
					<input type="radio" id="q7a" name="q7" value="vampire" required>
					<label for="q7a">🏰 Gothic castle in Transylvania</label>
				</div>
				<div class="answer-option">
					<input type="radio" id="q7b" name="q7" value="werewolf">
					<label for="q7b">🌲 Deep in the forest</label>
				</div>
				<div class="answer-option">
					<input type="radio" id="q7c" name="q7" value="zombie">
					<label for="q7c">⚰️ Cemetery or abandoned building</label>
				</div>
				<div class="answer-option">
					<input type="radio" id="q7d" name="q7" value="frankenstein">
					<label for="q7d">🔬 Mad scientist's laboratory</label>
				</div>
			</div>

			<!-- Question 8 -->
			<div class="question">
				<div class="question-title">8. What's your superpower?</div>
				<div class="answer-option">
					<input type="radio" id="q8a" name="q8" value="vampire" required>
					<label for="q8a">🦇 Shape-shifting and immortality</label>
				</div>
				<div class="answer-option">
					<input type="radio" id="q8b" name="q8" value="werewolf">
					<label for="q8b">💪 Superhuman strength during full moon</label>
				</div>
				<div class="answer-option">
					<input type="radio" id="q8c" name="q8" value="zombie">
					<label for="q8c">♾️ Inability to die (again)</label>
				</div>
				<div class="answer-option">
					<input type="radio" id="q8d" name="q8" value="frankenstein">
					<label for="q8d">⚡ Electrical immunity</label>
				</div>
			</div>

			<button type="submit" class="submit-button">
				🎃 Reveal My Monster 🎃
			</button>
		</form>

		<p class="warning">⚠️ Choose wisely... your answers will seal your fate ⚠️</p>
	</div>
</body>
</html>
"""

print(html)
